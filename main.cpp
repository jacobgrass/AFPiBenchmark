#include <arrayfire.h>
#include <math.h>
#include <stdio.h>
#include <cstdlib>
#include <numeric> // Include this header for std::accumulate
#include <iostream>
#include <cmath>
#include <random>
#include <omp.h>
#include <chrono>
#include "BenchmarkTest.h"
#include <string>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <thread>
// Include additional libraries
#ifdef _WIN32
#include <windows.h>
#include <tchar.h>
#else
#include <unistd.h>
#include <sys/utsname.h>
#endif

using namespace af;
using namespace std::chrono;

#ifndef AFPI_BUILD_INFO
#define AFPI_BUILD_INFO "unknown (not built with the project CMakeLists.txt)"
#endif

// Timing wrapper function (steady_clock is monotonic)
template<typename Func>
double measure(Func&& f) {
    auto start = steady_clock::now();
    f();
    auto end = steady_clock::now();
    return duration<double>(end - start).count();
}

// Quantile with linear interpolation between order statistics (q = 0.5 is the median)
double quantile(std::vector<double> v, double q) {
    std::sort(v.begin(), v.end());
    double pos = q * (v.size() - 1);
    size_t lo = (size_t)pos;
    size_t hi = std::min(lo + 1, v.size() - 1);
    return v[lo] + (pos - lo) * (v[hi] - v[lo]);
}

// Median, quartiles, min and max of the timed runs, then the median pi estimate
void writeStats(std::ofstream& file, const std::vector<double>& times, const std::vector<double>& values) {
    file << "," << quantile(times, 0.5) << "," << quantile(times, 0.25) << "," << quantile(times, 0.75)
         << "," << quantile(times, 0.0) << "," << quantile(times, 1.0) << "," << quantile(values, 0.5);
}

void runTest(float samples, int num_runs, std::string csvFile, std::string rawFile) {
    std::vector<double> times_device(num_runs);
    std::vector<double> times_host(num_runs);
    std::vector<double> times_omp(num_runs);
    std::vector<double> values_device(num_runs);
    std::vector<double> values_host(num_runs);
    std::vector<double> values_omp(num_runs);

    BenchmarkTest test(samples, num_runs);

    // Time each method in its own block: one untimed warm-up call, then the timed
    // runs back to back. The warm-up keeps device start-up, kernel compilation and
    // first memory allocation out of the timings; running the block back to back
    // keeps the GPU from idling (and possibly dropping to a power-saving state) while the CPU
    // methods run.
    auto timeMethod = [&](void (BenchmarkTest::*method)(double&),
                          std::vector<double>& times, std::vector<double>& values) {
        double result;
        (test.*method)(result);
        for (int i = 0; i < num_runs; ++i) {
            times[i] = measure([&] { (test.*method)(result); });
            values[i] = result;
        }
    };
    timeMethod(&BenchmarkTest::pi_device, times_device, values_device);
    timeMethod(&BenchmarkTest::pi_host, times_host, values_host);
    timeMethod(&BenchmarkTest::pi_omp, times_omp, values_omp);

    // Print results
    printf("Estimating PI using Monte Carlo method with:");
    // print number of samples in exponential notation and number of runs
    printf("\nSamples: %.1e, Runs: %d\n", (double)samples, num_runs);
    // write how may GB of memory was required, assuming each sample is 8 bytes
    printf("Memory required: %.2f GB\n", (double)samples * 8 / 1e9);

    // Print median times and median values of pi
    printf("\ndevice: %.6f s median (min %.6f, max %.6f) to estimate pi = %.6f\n",
        quantile(times_device, 0.5), quantile(times_device, 0.0), quantile(times_device, 1.0), quantile(values_device, 0.5));
    printf("  host: %.6f s median (min %.6f, max %.6f) to estimate pi = %.6f\n",
        quantile(times_host, 0.5), quantile(times_host, 0.0), quantile(times_host, 1.0), quantile(values_host, 0.5));
    printf("   omp: %.6f s median (min %.6f, max %.6f) to estimate pi = %.6f\n",
        quantile(times_omp, 0.5), quantile(times_omp, 0.0), quantile(times_omp, 1.0), quantile(values_omp, 0.5));

    // print that device is the araryfire device, host is single core performance, and omp is multi-core performance
    printf("NOTE:  Device is the ArrayFire device, Host is single core performance, and OMP is multi-core performance\n\n");

    // write the summary row to the csv file
    std::ofstream file;
    file.open(csvFile, std::ios::app);
    file.precision(9);
    file << samples << "," << (double)samples * 8 / 1e9 << "," << num_runs;
    writeStats(file, times_device, values_device);
    writeStats(file, times_host, values_host);
    writeStats(file, times_omp, values_omp);
    file << "\n";
    file.close();

    // write every timed run to the raw csv file
    std::ofstream raw;
    raw.open(rawFile, std::ios::app);
    raw.precision(9);
    for (int i = 0; i < num_runs; ++i) {
        raw << samples << ",device," << i + 1 << "," << times_device[i] << "," << values_device[i] << "\n";
        raw << samples << ",host," << i + 1 << "," << times_host[i] << "," << values_host[i] << "\n";
        raw << samples << ",omp," << i + 1 << "," << times_omp[i] << "," << values_omp[i] << "\n";
    }
    raw.close();
}

// First "model name" line of /proc/cpuinfo (Linux); empty elsewhere
std::string cpuModel() {
    std::ifstream cpuinfo("/proc/cpuinfo");
    std::string line;
    while (std::getline(cpuinfo, line)) {
        if (line.rfind("model name", 0) == 0) return line.substr(line.find(':') + 2);
    }
    return "";
}

int main(int argc, char** argv) {
    try {
        // Usage: AFPiBenchMark_<backend> [device] [timed runs per size] [largest power of ten]
        int device = argc > 1 ? atoi(argv[1]) : 0;
        int num_runs = argc > 2 ? atoi(argv[2]) : 20;
        if (num_runs < 1) num_runs = 1;  // the statistics need at least one timed run
        int max_power = argc > 3 ? atoi(argv[3]) : 9;
        setDevice(device);
        info();
        char deviceName[256]; 
        char platform[256];
        char toolkit[256]; 
        char compute[256]; 

        // Retrieve only the device name
        af::deviceInfo(deviceName, platform, toolkit, compute);
        // Create a text file that stores the system configuration
        std::string deviceNameStr(deviceName);
        std::string platformStr(platform);
        std::filesystem::create_directories("./results");
        std::string systemConfigFile = "./results/systemConfig_"+deviceNameStr+"_"+platformStr+".txt";
        std::ofstream sysConfig(systemConfigFile);
        if (sysConfig.is_open()) {
            sysConfig << "System Configuration\n";
            // write the output of af::info() to the file
            sysConfig << af::infoString(true) << std::endl;
            sysConfig << "CPU: " << cpuModel() << "\n";
            sysConfig << "Logical CPUs: " << std::thread::hardware_concurrency()
                      << ", OpenMP max threads: " << omp_get_max_threads() << "\n";
#ifndef _WIN32
            struct utsname os;
            if (uname(&os) == 0) sysConfig << "OS: " << os.sysname << " " << os.release << " " << os.machine << "\n";
#endif
#ifdef __VERSION__
            sysConfig << "Compiler: " << __VERSION__ << "\n";
#endif
            sysConfig << "Build: " << AFPI_BUILD_INFO << "\n";
            sysConfig << "Method: per size and method, 1 untimed warm-up call, then " << num_runs
                      << " timed runs back to back; median, quartiles, min and max reported\n";

            sysConfig.close();
        }
        else {
            std::cerr << "Unable to open file " << systemConfigFile << std::endl;
        }

        std::string csvFile = "./results/benchmarkResults_"+deviceNameStr+"_"+platformStr+".csv";
        std::string rawFile = "./results/benchmarkRuns_"+deviceNameStr+"_"+platformStr+".csv";

        // Header for CSV file: times in seconds, then the median pi estimate, per method
        std::ofstream file(csvFile);
        if (file.is_open()) {
            file << "Samples,Memory_Required_GB,Num_Runs";
            for (std::string m : {"Device", "Host", "OMP"}) {
                file << "," << m << "_Median_s," << m << "_P25_s," << m << "_P75_s,"
                     << m << "_Min_s," << m << "_Max_s," << m << "_Pi_Median";
            }
            file << "\n";
            file.close();
        }
        else {
            std::cerr << "Unable to open file " << csvFile << std::endl;
        }

        // Header for the raw CSV file: one row per timed run
        std::ofstream raw(rawFile);
        if (raw.is_open()) {
            raw << "Samples,Method,Run,Seconds,Pi_Estimate\n";
            raw.close();
        }
        else {
            std::cerr << "Unable to open file " << rawFile << std::endl;
        }

        for (int i = 0; i <= max_power; ++i) {
            float samples = pow(10, i);
            runTest(samples, num_runs, csvFile, rawFile);
        }
    }
    catch (std::exception& e) {
        fprintf(stderr, "%s\n", e.what());
        throw;
    }

    return 0;
}

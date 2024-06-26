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

// Timing wrapper function
template<typename Func>
double measure(Func&& f) {
    auto start = high_resolution_clock::now();
    f();
    auto end = high_resolution_clock::now();
    return duration<double>(end - start).count();
}

void getSystemInfo(std::ofstream& sysConfig) {
#ifdef _WIN32
    SYSTEM_INFO siSysInfo;
    GetSystemInfo(&siSysInfo);

    sysConfig << "Processor Architecture: ";
    switch (siSysInfo.wProcessorArchitecture) {
    case PROCESSOR_ARCHITECTURE_AMD64:
        sysConfig << "x64 (AMD or Intel)" << std::endl;
        break;
    case PROCESSOR_ARCHITECTURE_ARM:
        sysConfig << "ARM" << std::endl;
        break;
    case PROCESSOR_ARCHITECTURE_IA64:
        sysConfig << "Intel Itanium-based" << std::endl;
        break;
    case PROCESSOR_ARCHITECTURE_INTEL:
        sysConfig << "x86" << std::endl;
        break;
    default:
        sysConfig << "Unknown architecture" << std::endl;
        break;
    }

    sysConfig << "Number of Processors: " << siSysInfo.dwNumberOfProcessors << std::endl;

    TCHAR szBuffer[MAX_PATH];
    DWORD dwSize = sizeof(szBuffer) / sizeof(szBuffer[0]);
    if (GetComputerName(szBuffer, &dwSize))
        sysConfig << "Computer Name: " << szBuffer << std::endl;

    // Other system info can be added similarly using Windows API
#else
    struct utsname buffer;
    if (uname(&buffer) != 0) {
        perror("uname");
        return;
    }

    sysConfig << "System Name: " << buffer.sysname << std::endl;
    sysConfig << "Node Name: " << buffer.nodename << std::endl;
    sysConfig << "Release: " << buffer.release << std::endl;
    sysConfig << "Version: " << buffer.version << std::endl;
    sysConfig << "Machine: " << buffer.machine << std::endl;

    long nprocs = sysconf(_SC_NPROCESSORS_ONLN);
    sysConfig << "Number of CPU cores: " << nprocs << std::endl;

    char hostname[1024];
    gethostname(hostname, 1024);
    sysConfig << "Hostname: " << hostname << std::endl;
#endif
}

void runTest(float samples, int num_runs, std::string csvFile) {
    std::vector<double> times_device(num_runs);
    std::vector<double> times_host(num_runs);
    std::vector<double> times_omp(num_runs);
    std::vector<double> values_device(num_runs);
    std::vector<double> values_host(num_runs);
    std::vector<double> values_omp(num_runs);

    BenchmarkTest test(samples, num_runs);
    for (int i = 0; i < num_runs; ++i) {
        double result;

        times_device[i] = measure([&] { test.pi_device(result); });
        values_device[i] = result;

        times_host[i] = measure([&] { test.pi_host(result); });
        values_host[i] = result;

        times_omp[i] = measure([&] { test.pi_omp(result); });
        values_omp[i] = result;
    }

    // Print results
    printf("Estimating PI using Monte Carlo method with:");
    // print number of samples in exponential notation and number of runs
    printf("\nSamples: %.1e, Runs: %d\n", (double)samples, num_runs);
    // write how may GB of memory was required, assuming each sample is 8 bytes
    printf("Memory required: %.2f GB\n", (double)samples * 8 / 1e9);

    // Compute average times
    double avg_elapsed_device = accumulate(times_device.begin(), times_device.end(), 0.0) / num_runs;
    double avg_elapsed_host = accumulate(times_host.begin(), times_host.end(), 0.0) / num_runs;
    double avg_elapsed_omp = accumulate(times_omp.begin(), times_omp.end(), 0.0) / num_runs;

    // Compute average values of pi
    double avg_val_device = accumulate(values_device.begin(), values_device.end(), 0.0) / num_runs;
    double avg_val_host = accumulate(values_host.begin(), values_host.end(), 0.0) / num_runs;
    double avg_val_omp = accumulate(values_omp.begin(), values_omp.end(), 0.0) / num_runs;

    // Print results
    printf("\ndevice: %.5f seconds to estimate pi = %.6f\n", avg_elapsed_device, avg_val_device);
    printf("  host: %.5f seconds to estimate pi = %.6f\n", avg_elapsed_host, avg_val_host);
    printf("   omp: %.5f seconds to estimate pi = %.6f\n", avg_elapsed_omp, avg_val_omp);

    // print that device is the araryfire device, host is single core performance, and omp is multi-core performance
    printf("NOTE:  Device is the ArrayFire device, Host is single core performance, and OMP is multi-core performance\n\n");

    // write to csv file
    std::ofstream file;
    file.open(csvFile, std::ios::app);
    file << samples << "," << (double)samples * 8 / 1e9 << "," << num_runs << "," << avg_elapsed_device << "," << avg_val_device << "," << avg_elapsed_host << "," << avg_val_host << "," << avg_elapsed_omp << "," << avg_val_omp << "\n";
    file.close();
}

int main(int argc, char** argv) {
    try {
        int device = argc > 1 ? atoi(argv[1]) : 0;
        setDevice(device);
        info();
        char deviceName[256]; 
        char platform[256];
        char* toolkit = nullptr;
        char* compute = nullptr;

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
            // write the system configuration to the console
            // write various system details
            getSystemInfo(sysConfig);

            sysConfig.close();
        }
        else {
            std::cerr << "Unable to open file " << systemConfigFile << std::endl;
        }

        std::string csvFile = "./results/benchmarkResults_"+deviceNameStr+"_"+platformStr+".csv";

        // Header for CSV file
        std::ofstream file(csvFile);
        if (file.is_open()) {
            file << "Samples,Memory_Required_GB,Num_Runs,Avg_Elapsed_Device,Avg_Val_Device,Avg_Elapsed_Host,Avg_Val_Host,Avg_Elapsed_OMP,Avg_Val_OMP\n";
            file.close();
        }
        else {
            std::cerr << "Unable to open file " << csvFile << std::endl;
        }

        int num_runs = 3;
        for (int i = 0; i < 10; ++i) {
            float samples = pow(10, i);
            runTest(samples, num_runs, csvFile);
        }
    }
    catch (std::exception& e) {
        fprintf(stderr, "%s\n", e.what());
        throw;
    }

    return 0;
}

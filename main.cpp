/*******************************************************
 * Copyright (c) 2014, ArrayFire
 * All rights reserved.
 *
 * This file is distributed under 3-clause BSD license.
 * The complete license agreement can be obtained at:
 * http://arrayfire.com/licenses/BSD-3-Clause
 ********************************************************/

 /*
	monte-carlo estimation of PI

	algorithm:
	- generate random (x,y) samples uniformly
	- count what percent fell inside (top quarter) of unit circle
 */

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

using namespace af;
// Timing wrapper function
template<typename Func>
double measure(Func&& f) {
	auto start = high_resolution_clock::now();
	f();
	auto end = high_resolution_clock::now();
	return duration<double>(end - start).count();
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

		times_host[i] = measure([&] {test.pi_host(result); });
		values_host[i] = result;

		times_omp[i] = measure([&] {  test.pi_omp(result); });
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
	file.open(csvFile,std::ios::app);
	file << samples << ","<< (double)samples * 8 / 1e9<<"," << num_runs << "," << avg_elapsed_device << "," << avg_val_device << "," << avg_elapsed_host << "," << avg_val_host << "," << avg_elapsed_omp << "," << avg_val_omp << "\n";
	file.close();

}

int main(int argc, char** argv) {
	try {
		int device = argc > 1 ? atoi(argv[1]) : 0;
		setDevice(device);
		info();
		std::string csvFile = "benchmarkResults.csv";

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
			runTest(samples, num_runs,csvFile);
		}
	}
	catch (exception& e) {
		fprintf(stderr, "%s\n", e.what());
		throw;
	}

	return 0;
}


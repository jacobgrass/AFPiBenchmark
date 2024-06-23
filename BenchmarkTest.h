#pragma once
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
using namespace af;
using namespace std::chrono;

class BenchmarkTest
{
public:
	float samples;
	int num_runs;
	BenchmarkTest(float samples, int num_runs);
	void pi_device(double& result);
	void pi_host(double& result);
	void pi_omp(double& result);
};

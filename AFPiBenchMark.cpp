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

#include <iostream>
#include <cmath>
#include <random>
#include <omp.h>
using namespace af;
// generate millions of random samples
static int samples = 10e6;
/* Self-contained code to run host and device estimates of PI.  Note that
   each is generating its own random values, so the estimates of PI
   will differ. */
static double pi_device() {
    array x = randu(samples, f32), y = randu(samples, f32);
    return 4.0 * sum<float>(sqrt(x * x + y * y) < 1) / samples;
}

static double pi_host() {
    int count = 0;
    for(seq i, samples) {
        float x = float(rand()) / float(RAND_MAX);
        float y = float(rand()) / float(RAND_MAX);
        if (sqrt(x * x + y * y) < 1) count++;
    }
    return 4.0 * count / samples;
}

static double pi_omp() {
    int count = 0;

#pragma omp parallel
    {
        // C++11 random number generation setup
        std::random_device rd;  // Seed generator
        std::mt19937 gen(rd() ^ omp_get_thread_num());  // Mersenne Twister generator seeded with random device and thread ID
        std::uniform_real_distribution<> dis(0.0, 1.0); // Uniform distribution between 0 and 1

#pragma omp for reduction(+:count)
        for (int i = 0; i < samples; i++) {
            float x = dis(gen);
            float y = dis(gen);
            if (std::sqrt(x * x + y * y) < 1.0)
                count++;
        }
    }

    return 4.0 * count / samples;
}
// void wrappers for timeit()
static void device_wrapper() { pi_device(); }
static void host_wrapper() { pi_host(); }
static void omp_wrapper() { pi_omp(); }


int main(int argc, char** argv) {
    try {
        int device = argc > 1 ? atoi(argv[1]) : 0;
        setDevice(device);
        info();

        printf("\ndevice:  %.5f seconds to estimate  pi = %.5f\n",
            timeit(device_wrapper), pi_device());
        printf("  host:  %.5f seconds to estimate  pi = %.5f\n",
            timeit(host_wrapper), pi_host());
        printf("  omp:  %.5f seconds to estimate  pi = %.5f\n",
            timeit(omp_wrapper),pi_omp());

    }
    catch (exception& e) {
        fprintf(stderr, "%s\n", e.what());
        throw;
    }

    return 0;
}


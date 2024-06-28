#include "BenchmarkTest.h"


BenchmarkTest::BenchmarkTest(int samples, int num_runs) : samples(samples), num_runs(num_runs)
{
}

/* Self-contained code to run host and device estimates of PI.  Note that
   each is generating its own random values, so the estimates of PI
   will differ. */
void BenchmarkTest::pi_device(double& result) {
    array x = randu(samples, f32), y = randu(samples, f32);
    result = 4.0 * sum<float>(sqrt(x * x + y * y) < 1) / samples;
}


void BenchmarkTest::pi_host(double& result) {
    int count = 0;
    for (int i = 0; i < samples; i++) {
        float x = float(rand()) / float(RAND_MAX);
        float y = float(rand()) / float(RAND_MAX);
        if (sqrt(x * x + y * y) < 1) count++;
    }
    result = 4.0 * count / samples;
}

void  BenchmarkTest::pi_omp(double& result) {
    int count = 0;
    #pragma omp parallel 
        {

        std::random_device rd;  // Seed generator
        std::mt19937 gen(rd() ^ omp_get_thread_num());  // Mersenne Twister generator seeded with random device and thread ID
        std::uniform_real_distribution<> dis(0.0, 1.0); // Uniform distribution between 0 and 1
#pragma omp for reduction(+:count) 
            for (auto i = 0; i < samples; i++) {
                float x = dis(gen);
                float y = dis(gen);
                if (std::sqrt(x * x + y * y) < 1.0)
                    count++;
            }

        }
    result = 4.0 * count / samples;
}

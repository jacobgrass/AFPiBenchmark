// Diagnostic: does GPU idle time between calls change the time of one call?
#include <arrayfire.h>
#include <chrono>
#include <cstdio>
#include <thread>
using namespace af;

static double call(int n) {
    auto a = std::chrono::steady_clock::now();
    array x = randu(n, f32), y = randu(n, f32);
    float c = sum<float>(sqrt(x * x + y * y) < 1);
    af::sync();
    auto b = std::chrono::steady_clock::now();
    (void)c;
    return std::chrono::duration<double>(b - a).count() * 1e3;
}

int main() {
    int n = 100000000;
    call(n);
    for (int idle_ms : {0, 100, 300, 1000, 3000}) {
        printf("idle %5d ms before each call:", idle_ms);
        for (int r = 0; r < 6; r++) {
            std::this_thread::sleep_for(std::chrono::milliseconds(idle_ms));
            printf(" %7.2f", call(n));
        }
        printf(" ms\n");
    }
}

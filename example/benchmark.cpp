#define BENCHMARK_REPEAT_NUM 1
#include "benchmark.hpp"
#include <thread>

void Delay2s() {
    std::this_thread::sleep_for(std::chrono::seconds(2));
}

void Delay1s() {
    std::this_thread::sleep_for(std::chrono::seconds(1));
}

void use_measure(benchmark::Measure measure) {
    // some prepare
    measure([]{
        // your func
    });
    // some end
}

BENCHMARK_MAIN {
    BENCHMARK_GROUP("group1") {
        BENCHMARK_ADD("Delay1s", Delay1s);
        BENCHMARK_ADD("Delay2s", Delay2s);
    }
    BENCHMARK_GROUP("group2") {
        BENCHMARK_ADD("use_measure", use_measure);
    }

    BENCHMARK_RUN("group1", "group2");
}
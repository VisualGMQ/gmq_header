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

    /*
        run some groups:
        BENCHMARK_RUN_GROUPS("group1", "group2");
    */
    /*
        run all groups(you can't point out which group in cmd parameter):
        BENCHMARK_RUN_ALL();
    */
    /*
        run groups, you can send group names in cmdline. Eg:
        ./benchmark group1
    */
    BENCHMARK_RUN();
}
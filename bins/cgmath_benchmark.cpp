// Copyright 2023 VisualGMQ
#include <limits>

#define CGMATH_NUMERIC_TYPE double
#include "cgmath.hpp"

#define BENCHMARK_REPEAT_NUM 10000000
#include "benchmark.hpp"

void SimpleAdd(benchmark::Measure measure) {
    cgmath::Vec4 vec1{1, 2, 3, 4};
    cgmath::Vec4 vec2{5, 6, 7, 8};

    measure([&](){
        auto v = vec1 + vec2;
    });
}

void SimpleDot(benchmark::Measure measure) {
    cgmath::Vec4 vec1{1, 2, 3, 4};
    cgmath::Vec4 vec2{5, 6, 7, 8};

    measure([&](){
        auto v = vec1.Dot(vec2);
    });
}

BENCHMARK_MAIN {
    BENCHMARK_GROUP("add benchmark") {
        BENCHMARK_ADD("simple", SimpleAdd);
    }

    BENCHMARK_GROUP("dot benchmark") {
        BENCHMARK_ADD("simple", SimpleDot);
    }

    BENCHMARK_RUN();
}

#define USE_SIMD
#include "cgmath.hpp"

#define BENCHMARK_REPEAT_NUM 10000000
#include "benchmark.hpp"

#include <limits>

void SIMDAdd(benchmark::Measure measure) {
    cgmath::Vec4 simd_vec1{1, 2, 3, 4};
    cgmath::Vec4 simd_vec2{5, 6, 7, 8};

    measure([&](){
        auto v = simd_vec1 + simd_vec2;
    });
}

void SimpleAdd(benchmark::Measure measure) {
    cgmath::Vector<double, 4> vec1{1, 2, 3, 4};
    cgmath::Vector<double, 4> vec2{5, 6, 7, 8};

    measure([&](){
        auto v = vec1 + vec2;
    });
}

void SIMDDot(benchmark::Measure measure) {
    cgmath::Vec4 simd_vec1{1, 2, 3, 4};
    cgmath::Vec4 simd_vec2{5, 6, 7, 8};

    measure([&](){
        auto v = simd_vec1.Dot(simd_vec2);
    });
}

void SimpleDot(benchmark::Measure measure) {
    cgmath::Vector<double, 4> vec1{1, 2, 3, 4};
    cgmath::Vector<double, 4> vec2{5, 6, 7, 8};

    measure([&](){
        auto v = vec1.Dot(vec2);
    });
}

BENCHMARK_MAIN {
    BENCHMARK_GROUP("add benchmark") {
        BENCHMARK_ADD("simd", SIMDAdd);
        BENCHMARK_ADD("simple", SimpleAdd);
    }

    BENCHMARK_GROUP("dot benchmark") {
        BENCHMARK_ADD("simd", SIMDDot);
        BENCHMARK_ADD("simple", SimpleDot);
    }

    BENCHMARK_RUN();
}
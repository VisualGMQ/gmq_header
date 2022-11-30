add_executable(benchmark ./benchmark.cpp)
target_include_directories(benchmark PRIVATE ../)
target_compile_features(benchmark PRIVATE cxx_std_17)
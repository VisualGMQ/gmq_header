# GMQ-Headers

There are some useful header-only files for C++17.

Use CMake to compile examples(or unittest):

```bash
cmake -S . -B build
cmake --build build
```


|module|description|deps|
|--|--|--|
|benchmark.hpp|a small, eazy to use benchmark framework for C++|None|
|log.hpp|a small log system|None|
|cgmath.hpp|a math library for computer graphics and computational geomentry|None, but test dependent on 3rdlibs/catch2.hpp and benchmark depends on benchmark.hpp|
|expect.hpp|a implementation of std::expect(C++23) in C++17|None, but test dependent on 3rdlibs/catch2.hpp|

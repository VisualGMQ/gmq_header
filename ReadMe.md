# GMQ-Headers

There are some useful header-only files for C++17.

Use CMake to compile examples(or unittest):

```bash
cmake -S . -B build
cmake --build build
```


|module|description|deps|support|
|--|--|--|--|
|benchmark.hpp|a small, eazy to use benchmark framework for C++|None||
|log.hpp|a small log system|None||
|cgmath.hpp|a math library for computer graphics and computational geomentry|None, but test dependent on 3rdlibs/catch2.hpp and benchmark depends on benchmark.hpp||
|expect.hpp|a implementation of std::expect(C++23) in C++17|None, but test dependent on 3rdlibs/catch2.hpp|deprecated, maybe use C++23 after few years|
|ecs.hpp|an ECS framework referenced bevy's ECS|sparse_sets.hpp|deprecated, new version is [gecs](https://github.com/VisualGMQ/gecs)|
|sparse_sets.hpp|a sparse_set data-structure implement, [reference](https://manenko.com/2021/05/23/sparse-sets.html)|None|new version in [gecs](https://github.com/VisualGMQ/gecs)|
|net.hpp|a thin layer for Win32 Socket|None|
|fp.hpp|a functional programming library referenced Haskell & Lisp.Aimed to do compile time algorithm/reflection easier.Has two implementations: pure template and constexpr function|None|new version in [mirrow](https://github.com/VisualGMQ/mirrow)|
|refl.hpp|static reflection in compile-time|None|deprecated, new version is [mirrow](https://github.com/VisualGMQ/mirrow)|
|serialize.hpp|a serialize/deserialize library for convert class to/from lua|refl.hpp & sol2(under `3rdlibs/`) & log.hpp|deprecated, new version is [mirrow](https://github.com/VisualGMQ/mirrow)|
|luabind|use static reflection and sol2 to auto-bind C++ code to lua|lua, sol and refl.hpp||
|gogl.hpp|a thin layer for OpenGL 4.3|log.hpp & cgmath.hpp & any opengl loader(glew, glad). Test dependents on glad and glfw(in `3rdlibs/`)||
|tweeny.hpp|a eazy to use tween library for game and GUI interaction|None||
|renderer2d.hpp|a 2D renderer in OpenGL for quickly embed in OpenGL Context|gogl.hpp, cgmath.hpp||

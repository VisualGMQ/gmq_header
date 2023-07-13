project(lua
        VERSION 5.4.4)

set(LUA_LIB_SRC
    src/lapi.c
    src/lcode.c
    src/lctype.c
    src/ldebug.c
    src/ldo.c
    src/ldump.c
    src/lfunc.c
    src/lgc.c
    src/llex.c
    src/lmem.c
    src/lobject.c
    src/lopcodes.c
    src/lparser.c
    src/lstate.c
    src/lstring.c
    src/ltable.c
    src/ltm.c
    src/lundump.c
    src/lvm.c
    src/lzio.c
    src/ltests.c
    src/lauxlib.c
    src/lbaselib.c
    src/ldblib.c
    src/liolib.c
    src/lmathlib.c
    src/loslib.c
    src/ltablib.c
    src/lstrlib.c
    src/lutf8lib.c
    src/loadlib.c
    src/lcorolib.c
    src/linit.c
)

add_library(lua STATIC ${LUA_LIB_SRC})
target_compile_features(lua PRIVATE c_std_99)
target_include_directories(lua PUBLIC include)
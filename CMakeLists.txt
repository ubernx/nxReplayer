cmake_minimum_required(VERSION 3.10)
project(nxReplayer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MINGW)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++ -static")
endif()

add_executable(nxReplayer main.cpp ScriptFile.hpp
        ScriptReplayer.hpp
        InputSimulator.hpp
        GameProcessUtils.hpp
        UserBinds.hpp)


target_link_libraries(nxReplayer PRIVATE winmm)

target_include_directories(nxReplayer PRIVATE
        "${CMAKE_SOURCE_DIR}"
        "C:/Program Files/JetBrains/CLion 2025.1.3/bin/mingw/include"
)
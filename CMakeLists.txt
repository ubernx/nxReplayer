cmake_minimum_required(VERSION 3.28)
project(nxReplayer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable C++20 module support
set(CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API "2182bf5c-ef0d-489a-91da-49dbc3090d2a")
set(CMAKE_EXPERIMENTAL_CXX_MODULE_DYNDEP ON)

if(MINGW)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++ -static")
endif()

add_executable(nxReplayer)

target_sources(nxReplayer
    PRIVATE
        main.cpp
    PRIVATE FILE_SET CXX_MODULES FILES
        ScriptFile.cpp
        ScriptReplayer.cpp
        InputSimulator.cpp
        GameProcessUtils.cpp
        UserBinds.cpp
        OpCodeToString.cpp
)

target_link_libraries(nxReplayer PRIVATE winmm)

target_include_directories(nxReplayer PRIVATE
    "${CMAKE_SOURCE_DIR}"
    "C:/Program Files/JetBrains/CLion 2025.1.3/bin/mingw/include"
)
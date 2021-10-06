cmake_minimum_required(VERSION 3.20)

project(Mizu LANGUAGES CXX C)

set (CMAKE_CXX_STANDARD 17)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib) # Output directory for static lib (.LIB)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin) # Output directory for shared lib (.DLL)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin) # Output directory for executables (.EXE)

set(HEADERS
 inc/DX12LibPCH.h
 inc/d3dx12.h
 inc/CommandQueue.hpp
 inc/Window.hpp
 inc/Application.hpp
 inc/EventArgs.hpp
 inc/Game.hpp
 )

set(SOURCES 
src/CommandQueue.cpp
src/Window.cpp
src/Application.cpp
src/Game.cpp
)


set(GAME_HEADERS
inc/CubeDemo.hpp
)

set(GAME_SOURCES
src/CubeDemo.cpp
)

include_directories(inc)

add_library(Engine STATIC ${HEADERS} ${SOURCES})
add_library(DEMO_LIB STATIC ${GAME_HEADERS} ${GAME_SOURCES})


add_executable(Mizu WIN32 main.cpp)
add_executable(CubeDemo WIN32 demo_main.cpp)


set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT CubeDemo)
set_property(TARGET CubeDemo PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/build/bin/Debug")

macro(print_all_variables)
    message(STATUS "print_all_variables------------------------------------------{")
    get_cmake_property(_variableNames VARIABLES)
    foreach (_variableName ${_variableNames})
        message(STATUS "${_variableName}=${${_variableName}}")
    endforeach()
    message(STATUS "print_all_variables------------------------------------------}")
endmacro()


print_all_variables()

add_definitions(-D_UNICODE)
add_definitions(-DUNICODE)
#pragma comment(linker, "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")

# TODO Make a library for the lib files
target_link_libraries(Mizu
Engine
d3d12.lib
dxguid.lib
DXGI.lib
D3DCompiler.lib
)

target_link_libraries(CubeDemo
DEMO_LIB
Engine
d3d12.lib
dxguid.lib
DXGI.lib
d3dcompiler.lib
)
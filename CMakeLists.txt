cmake_minimum_required(VERSION 3.20)

project(Mizu LANGUAGES CXX C)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib) # Output directory for static lib (.LIB)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin) # Output directory for shared lib (.DLL)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin) # Output directory for executables (.EXE)


set(HEADERS
 inc/d3dx12.h
 inc/Helpers.h
 inc/CommandQueue.hpp
 inc/Game.hpp
 inc/KeyCodes.h
 inc/Events.h
 inc/Window.hpp
 inc/Application.hpp
 inc/DX12LibPCH.h
 )

set(SOURCES 
src/CommandQueue.cpp
src/Game.cpp
src/Window.cpp
src/Application.cpp)

include_directories(inc)

add_library(Engine STATIC
					${HEADERS}
					${SOURCES})

add_executable(Mizu WIN32 main.cpp)
add_definitions(-D_UNICODE)
add_definitions(-DUNICODE)
#pragma comment(linker, "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")

target_link_libraries(Mizu
Engine
d3d12.lib
dxgi.lib
d3dcompiler.lib
)

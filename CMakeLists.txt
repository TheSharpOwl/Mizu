cmake_minimum_required(VERSION 3.20)

project(Mizu LANGUAGES CXX C)

set (CMAKE_CXX_STANDARD 17)

# Use solution folders to organize projects
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

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

set(SHADER_FILES
    shaders/VertexShader.hlsl
    shaders/PixelShader.hlsl
)

source_group("Shaders" FILES ${SHADER_FILES})

set_source_files_properties(shaders/VertexShader.hlsl PROPERTIES 
    VS_SHADER_TYPE Vertex
    VS_SHADER_MODEL 5.1
)

set_source_files_properties(shaders/PixelShader.hlsl PROPERTIES 
    VS_SHADER_TYPE Pixel
    VS_SHADER_MODEL 5.1
)

include_directories(inc)

add_library(Engine STATIC ${HEADERS} ${SOURCES})
add_library(DEMO_LIB STATIC ${GAME_HEADERS} ${GAME_SOURCES})


add_executable(CubeDemo WIN32 main.cpp ${SHADER_FILES})

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT CubeDemo)
set_property(TARGET CubeDemo PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:CubeDemo>")
#set_property(TARGET CubeDemo PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/build/bin/Debug")


add_definitions(-D_UNICODE)
add_definitions(-DUNICODE)
#pragma comment(linker, "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")

# TODO Make a library for the lib files
target_link_libraries(CubeDemo
DEMO_LIB
Engine
d3d12.lib
dxguid.lib
DXGI.lib
d3dcompiler.lib
)

# Helper to call when debugging
macro(print_all_variables)
    message(STATUS "print_all_variables------------------------------------------{")
    get_cmake_property(_variableNames VARIABLES)
    foreach (_variableName ${_variableNames})
        message(STATUS "${_variableName}=${${_variableName}}")
    endforeach()
    message(STATUS "print_all_variables------------------------------------------}")
endmacro()

add_subdirectory(expirements/triangle)
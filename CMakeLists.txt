cmake_minimum_required(VERSION 3.20)

project(Mizu LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)

# Use solution folders to organize projects
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib) # Output directory for static lib (.LIB)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin) # Output directory for shared lib (.DLL)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin) # Output directory for executables (.EXE)

add_definitions(-D_UNICODE)
add_definitions(-DUNICODE)

# add Direct3D helper headers
include_directories("${CMAKE_SOURCE_DIR}/DX")

# access the engien headers
include_directories("${CMAKE_SOURCE_DIR}/Engine/include")

# link ALL targets to directx
link_libraries(
d3d12.lib
dxguid.lib
DXGI.lib
d3dcompiler.lib
)

add_subdirectory(Engine)
add_subdirectory(CubeDemo)
add_subdirectory(Experiments)

# Make cube demo the default project
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT CubeDemo)

# Helper to call when debugging
macro(print_all_variables)
    message(STATUS "print_all_variables------------------------------------------{")
    get_cmake_property(_variableNames VARIABLES)
    foreach (_variableName ${_variableNames})
        message(STATUS "${_variableName}=${${_variableName}}")
    endforeach()
    message(STATUS "print_all_variables------------------------------------------}")
endmacro()
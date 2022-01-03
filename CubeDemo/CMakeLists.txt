########### Cube Demo
set(CUBE_DEMO_SHADERS
    shaders/VertexShader.hlsl
    shaders/PixelShader.hlsl
)

source_group("Shaders" FILES ${CUBE_DEMO_SHADERS})

set_source_files_properties(shaders/VertexShader.hlsl PROPERTIES 
    VS_SHADER_TYPE Vertex
    VS_SHADER_MODEL 5.1
)

set_source_files_properties(shaders/PixelShader.hlsl PROPERTIES 
    VS_SHADER_TYPE Pixel
    VS_SHADER_MODEL 5.1
)

add_executable(CubeDemo WIN32 main.cpp CubeDemo.hpp CubeDemo.cpp ${CUBE_DEMO_SHADERS})

add_dependencies(CubeDemo Mizu)

# In order to just use shaders folder directly from the executable
set_property(TARGET CubeDemo PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:CubeDemo>")

target_include_directories(CubeDemo PUBLIC ${CMAKE_CURRENT_LIST_DIR})
include_directories("${CMAKE_SOURCE_DIR}/DX")
target_include_directories(CubeDemo PUBLIC "${PROJECT_SOURCE_DIR}/Engine/include")

target_link_libraries(CubeDemo
"$<$<CONFIG:Debug>:${CMAKE_BINARY_DIR}/lib/Debug/Mizu.lib>"
"$<$<CONFIG:Release>:${CMAKE_BINARY_DIR}/lib/Release/Mizu.lib>"
)


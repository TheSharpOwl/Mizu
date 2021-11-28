set(EXP_SRC 
D3D12HelloTriangle.cpp
DXSample.cpp
stdafx.cpp
Win32Application.cpp
)

set(EXP_HEADERS
D3D12HelloTriangle.h
d3dx12.h
DXSample.h
DXSampleHelper.h
stdafx.h
Win32Application.h
)

set(EXP_SHADERS
shaders.hlsl
)
# -------------------------------------------------
# To make Cmake stop trying to compile the shaders file
set_source_files_properties(${EXP_SHADERS} PROPERTIES VS_TOOL_OVERRIDE "None")

add_library(triangle_microsoft_lib STATIC
${EXP_HEADERS}
${EXP_SRC}
${EXP_SHADERS}
)

# --------------------------------------------------- 
add_executable(HelloTriangle WIN32 Main.cpp)

target_include_directories(HelloTriangle PUBLIC ".")

target_link_libraries(HelloTriangle
triangle_microsoft_lib
DEMO_LIB
Engine
d3d12.lib
dxguid.lib
DXGI.lib
d3dcompiler.lib
)

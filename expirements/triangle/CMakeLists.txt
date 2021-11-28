add_library(triangle_lib STATIC
D3D12HelloTriangle.cpp
D3D12HelloTriangle.h
d3dx12.h
DXSample.cpp
DXSample.h
DXSampleHelper.h
# shaders.hlsl
stdafx.cpp
stdafx.h
Win32Application.cpp
Win32Application.h
)
add_executable(HelloTriangle WIN32 Main.cpp)
set_property(TARGET HelloTriangle PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/expirements/triangle")

target_include_directories(HelloTriangle PUBLIC ".")

target_link_libraries(HelloTriangle
triangle_lib
DEMO_LIB
Engine
d3d12.lib
dxguid.lib
DXGI.lib
d3dcompiler.lib
)

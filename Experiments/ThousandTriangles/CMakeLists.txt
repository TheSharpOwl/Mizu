set(ThousandTriangles_SOURCES
ThousandTriangles.cpp
DXSample.cpp
stdafx.cpp
Win32Application.cpp
)

set(ThousandTriangles_HEADERS
ThousandTriangles.h
d3dx12.h
DXSample.h
DXSampleHelper.h
stdafx.h
Win32Application.h
)

set(ThousandTriangles_SHADERS
resources/shaders.hlsl
)

# To make Cmake stop trying to compile the shaders file
set_source_files_properties(${EXP_SHADERS} PROPERTIES VS_TOOL_OVERRIDE "None")

# add_library(triangle_microsoft_lib STATIC
# ${EXP_HEADERS}
# ${EXP_SRC}
# ${EXP_SHADERS}
# )

# --------------------------------------------------- 
add_executable(HelloTriangle WIN32 Main.cpp ${ThousandTriangles_HEADERS} ${ThousandTriangles_SOURCES})

target_include_directories(HelloTriangle PUBLIC ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(HelloTriangle
Mizu
)

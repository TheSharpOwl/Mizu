set(ThousandTriangles_SOURCES
ThousandTriangles.cpp
DXSample.cpp
stdafx.cpp
Win32Application.cpp
)

set(ThousandTriangles_HEADERS
ThousandTriangles.h
DXSample.h
DXSampleHelper.h
stdafx.h
Win32Application.h
)

set(ThousandTriangles_SHADERS
resources/ThousandTrianglesVertexShader.hlsl
resources/ThousandTrianglesPixelShader.hlsl
resources/MeshShader.hlsl
)

set_source_files_properties(resources/MeshShader.hlsl PROPERTIES 
    VS_SHADER_TYPE Mesh
    VS_SHADER_MODEL 6.5
)
# left as a draft to learn how to use later / in case this syntax is needed later...
#set_source_files_properties(resources/ThousandTrianglesVertxShader.hlsl PROPERTIES VS_SHADER_FLAGS "/E\"main\" /vs\"_6_5\"")

set_source_files_properties(resources/ThousandTrianglesVertexShader.hlsl PROPERTIES 
    VS_SHADER_TYPE Vertex
    VS_SHADER_MODEL 6.5
)

set_source_files_properties(resources/ThousandTrianglesPixelShader.hlsl PROPERTIES 
    VS_SHADER_TYPE Pixel
    VS_SHADER_MODEL 6.5
)

# --------------------------------------------------- 
add_executable(ThousandTriangles WIN32 Main.cpp ${ThousandTriangles_HEADERS} ${ThousandTriangles_SOURCES} ${ThousandTriangles_SHADERS})

# In order to just use shaders folder directly from the executable
set_property(TARGET ThousandTriangles PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:ThousandTriangles>")

# In order to use files from the directory of the ThousandTriangles target (turned off for now to use cso files shorter way for compiling the mesh shader)
#set_property(TARGET ThousandTriangles PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}")

source_group("Shaders" FILES ${ThousandTriangles_SHADERS})

add_dependencies(ThousandTriangles Mizu)
target_include_directories(ThousandTriangles PUBLIC ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(ThousandTriangles
Mizu
)


target_link_libraries(ThousandTriangles
"$<$<CONFIG:Debug>:${CMAKE_BINARY_DIR}/lib/Debug/Mizu.lib>"
"$<$<CONFIG:Release>:${CMAKE_BINARY_DIR}/lib/Release/Mizu.lib>"
)

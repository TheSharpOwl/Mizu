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
resources/shaders.hlsl
)

# To make Cmake stop trying to compile the shaders file
set_source_files_properties(${ThousandTriangles_SHADERS} PROPERTIES VS_TOOL_OVERRIDE "None")

# --------------------------------------------------- 
add_executable(ThousandTriangles WIN32 Main.cpp ${ThousandTriangles_HEADERS} ${ThousandTriangles_SOURCES} ${ThousandTriangles_SHADERS})

source_group("Shaders" FILES ${ThousandTriangles_SHADERS})

add_dependencies(ThousandTriangles Mizu)
target_include_directories(ThousandTriangles PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(ThousandTriangles
Mizu
)

set_property(TARGET ThousandTriangles PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}")

target_link_libraries(ThousandTriangles
"$<$<CONFIG:Debug>:${CMAKE_BINARY_DIR}/lib/Debug/Mizu.lib>"
"$<$<CONFIG:Release>:${CMAKE_BINARY_DIR}/lib/Release/Mizu.lib>"
)

set(ENGINE_HEADERS
        include/Mizu/CommandQueue.hpp
        include/Mizu/Window.hpp
        include/Mizu/Application.hpp
        include/Mizu/EventArgs.hpp
        include/Mizu/Game.hpp
        include/Mizu/Utils.hpp
        include/Mizu/UploadBuffer.hpp
        include/Mizu/DescriptorAllocator.hpp
        include/Mizu/DescriptorAllocation.hpp
        include/Mizu/DescriptorAllocatorPage.hpp
        )

set(ENGINE_SOURCES
        src/CommandQueue.cpp
        src/Window.cpp
        src/Application.cpp
        src/Game.cpp
        src/UploadBuffer.cpp
        src/DescriptorAllocator.cpp
        src/DescriptorAllocation.cpp
        src/DescriptorAllocatorPage.cpp
        )

add_library(Mizu STATIC ${ENGINE_HEADERS} ${ENGINE_SOURCES})

set_target_properties(Mizu PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY_DEBUG  "${CMAKE_BINARY_DIR}/lib/Debug/")

set_target_properties(Mizu PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/lib/Release/")

#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "$<$<CONFIG:Debug>:${CMAKE_BINARY_DIR}/lib/Debug/>")
#set_target_properties(Mizu PROPERTIES LIBRARY_OUTPUT_DIRECTORY "$<$<CONFIG:Release>:${CMAKE_BINARY_DIR}/lib/Release/Mizu.lib>")

# Make the headers available to the engine without Mizu prefix
target_include_directories(Mizu PRIVATE "${PROJECT_SOURCE_DIR}/Engine/include/Mizu")


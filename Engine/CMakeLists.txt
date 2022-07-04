set(ENGINE_HEADERS
    include/Mizu/CommandQueue.hpp
    include/Mizu/Window.hpp
    include/Mizu/Application.hpp
    include/Mizu/EventArgs.hpp
    include/Mizu/Game.hpp
    include/Mizu/Utils.hpp
    include/Mizu/UploadBuffer.hpp
    include/Mizu/DescriptorAllocator.hpp
    include/Mizu/DescriptorAllocatorPage.hpp
    include/Mizu/DescriptorAllocation.hpp
    include/Mizu//DynamicDescriptorHeap.hpp
)

set(ENGINE_SOURCES 
    src/CommandQueue.cpp
    src/Window.cpp
    src/Application.cpp
    src/Game.cpp
    src/UploadBuffer.cpp
    src/DescriptorAllocator.cpp
    src/DescriptorAllocatorPage.cpp
    src/DescriptorAllocation.cpp
    src/DynamicDescriptorHeap.cpp
)


add_library(Mizu STATIC ${ENGINE_HEADERS} ${ENGINE_SOURCES})

# Make the headers available to the engine without Mizu prefix
target_include_directories(Mizu PRIVATE "${PROJECT_SOURCE_DIR}/Engine/include/Mizu")

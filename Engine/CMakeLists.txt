set(ENGINE_HEADERS
 include/Mizu/CommandQueue.hpp
 include/Mizu/Window.hpp
 include/Mizu/Application.hpp
 include/Mizu/EventArgs.hpp
 include/Mizu/Game.hpp
 include/Mizu/Utils.hpp
 )

set(ENGINE_SOURCES 
src/CommandQueue.cpp
src/Window.cpp
src/Application.cpp
src/Game.cpp
)


add_library(Mizu STATIC ${ENGINE_HEADERS} ${ENGINE_SOURCES})

# Make the headers available to the engine without Mizu prefix
target_include_directories(Mizu PRIVATE "${PROJECT_SOURCE_DIR}/Engine/include/Mizu")


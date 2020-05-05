add_subdirectory(glad)
add_subdirectory(SDL-mirror)
add_library(SDL_Includes INTERFACE)
target_include_directories(SDL_Includes INTERFACE SDL-mirror/include)

add_library(glm INTERFACE)
target_include_directories(glm INTERFACE glm)

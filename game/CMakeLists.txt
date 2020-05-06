file(GLOB_RECURSE PONG_SRC "src/**.cpp")
file(GLOB_RECURSE PONG_INC "include/**.hpp" "include/**.h")

add_executable(pong ${PONG_SRC} ${PONG_INC})
target_include_directories(pong PUBLIC "include")

find_package(OpenAL CONFIG REQUIRED)
target_link_libraries(pong PRIVATE OpenAL::OpenAL)

target_link_libraries(pong PUBLIC SDL2 SDL_Includes glm glad)
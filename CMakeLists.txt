cmake_minimum_required(VERSION 3.23)

project(vktest LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

find_package(glfw3 3.3 REQUIRED)

add_library(glad OBJECT "external/glad/src/vulkan.c")
target_include_directories(glad PUBLIC "external/glad/include/")
target_compile_definitions(glad PRIVATE "VK_USE_PLATFORM_WAYLAND_KHR")

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.h" "src/*.c")
add_executable(vktest ${SOURCES})
unset(SOURCES)
target_link_libraries(vktest PRIVATE glfw glad)

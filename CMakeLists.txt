project(vulkanalia)
cmake_minimum_required(VERSION 3.8)

find_package(Vulkan REQUIRED)

add_executable(vulkanalia main.cpp)
target_link_libraries(vulkanalia PRIVATE Vulkan::Vulkan)

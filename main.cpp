/*
 *Copyright 2017 Lukasz Towarek
 *
 *Licensed under the Apache License, Version 2.0 (the "License");
 *you may not use this file except in compliance with the License.
 *You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *Unless required by applicable law or agreed to in writing, software
 *distributed under the License is distributed on an "AS IS" BASIS,
 *WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *See the License for the specific language governing permissions and
 *limitations under the License.
 */

#define GLFW_INCLUDE_VULKAN
#include "triangle.hpp"
#include <GLFW/glfw3.h>

int main(int argc, char *argv[]) {
  const std::string application_name = "Triangle";
  int width = 500;
  int height = 500;

  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  GLFWwindow *window = glfwCreateWindow(width, height, application_name.c_str(),
                                        nullptr, nullptr);

  const vk::ApplicationInfo application_info =
      vka::create_application_info(application_name, {0, 1, 0});
  const vk::UniqueInstance instance = vka::create_instance(application_info);

  VkSurfaceKHR raw_surface;
  glfwCreateWindowSurface(*instance, window, nullptr, &raw_surface);
  vk::UniqueSurfaceKHR surface(raw_surface);

  VulkanController vulkan_controller;
  vulkan_controller.initialize(*instance, *surface,
                               vk::Extent2D(width, height));

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
    vulkan_controller.draw();
  }

  surface.release();
}

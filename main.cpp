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

class TriangleApplication {
public:
  void run() {
    const std::string application_name = "Triangle";
    int width = 500;
    int height = 500;

    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window_ = glfwCreateWindow(width, height, application_name.c_str(), nullptr,
                               nullptr);
    glfwSetWindowUserPointer(window_, this);
    glfwSetWindowSizeCallback(window_, TriangleApplication::resize);

    const vk::ApplicationInfo application_info =
        vka::create_application_info(application_name, {0, 1, 0});
    vk::UniqueInstance instance = vka::create_instance(application_info);

    VkSurfaceKHR raw_surface;
    glfwCreateWindowSurface(*instance, window_, nullptr, &raw_surface);
    vk::UniqueSurfaceKHR surface(raw_surface);

    vulkan_controller_.initialize(std::move(instance), std::move(surface),
                                  vk::Extent2D(width, height));

    while (!glfwWindowShouldClose(window_)) {
      glfwPollEvents();
      vulkan_controller_.draw();
    }

    glfwDestroyWindow(window_);
    glfwTerminate();
  }
  void recreate_swapchain() {
    int width, height;
    glfwGetWindowSize(window_, &width, &height);
    vulkan_controller_.recreate_swapchain(vk::Extent2D(width, height));
  }
  static void resize(GLFWwindow *window, int width, int height) {
    auto application = reinterpret_cast<TriangleApplication *>(
        glfwGetWindowUserPointer(window));
    application->recreate_swapchain();
  }

private:
  VulkanController vulkan_controller_;
  GLFWwindow *window_;
};

int main(int argc, char *argv[]) {
  TriangleApplication application;
  application.run();
  return 0;
}

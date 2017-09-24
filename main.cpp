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

#include "triangle.hpp"

int main(int argc, char *argv[]) {
  const std::string application_name = "Triangle";
  const vk::ApplicationInfo application_info =
      vka::create_application_info(application_name, {0, 1, 0});
  const vk::UniqueInstance instance = vka::create_instance(application_info);

  vka::WindowManager window_manager(application_name);
  const vk::UniqueSurfaceKHR surface = vka::create_surface(
      *instance, window_manager.hInstance(), window_manager.hWnd());

  const vk::Extent2D swapchain_extent(500, 500);

  VulkanController vulkan_controller;
  vulkan_controller.initialize(*instance, *surface, swapchain_extent);

  MSG msg;
  bool quit = false;
  while (!quit) {
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
      if (msg.message == WM_QUIT) {
        quit = true;
      }
    }
    vulkan_controller.draw();
  }
}

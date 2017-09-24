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

  const std::vector<vk::PhysicalDevice> devices =
      (*instance).enumeratePhysicalDevices();
  const vk::PhysicalDevice physical_device =
      vka::select_physical_device(devices);

  vka::WindowManager window_manager(application_name);
  const vk::UniqueSurfaceKHR surface = vka::create_surface(
      *instance, window_manager.hInstance(), window_manager.hWnd());

  const std::vector<vk::SurfaceFormatKHR> formats =
      physical_device.getSurfaceFormatsKHR(*surface);
  const vk::SurfaceFormatKHR surface_format =
      vka::select_surface_format(formats);
  const vk::SurfaceCapabilitiesKHR capabilities =
      physical_device.getSurfaceCapabilitiesKHR(*surface);

  uint32_t width = 500;
  uint32_t height = 500;
  const vk::Extent2D swapchain_extent =
      vka::select_swapchain_extent(capabilities, width, height);

  const std::vector<vk::QueueFamilyProperties> queue_family_properties =
      physical_device.getQueueFamilyProperties();

  const std::vector<vk::Bool32> presentation_support =
      vka::get_presentation_support(physical_device, *surface,
                                    queue_family_properties.size());

  const uint32_t queue_index =
      vka::find_graphics_and_presentation_queue_family_index(
          queue_family_properties, presentation_support);

  const vk::UniqueDevice device =
      vka::create_device(physical_device, queue_index);

  vk::UniqueSwapchainKHR swapchain = vka::create_swapchain(
      surface_format, swapchain_extent, capabilities, *device, *surface);

  std::vector<vk::Image> swapchain_images =
      (*device).getSwapchainImagesKHR(*swapchain);

  std::vector<vk::UniqueImageView> swapchain_image_views =
      vka::create_swapchain_image_views(*device, swapchain_images,
                                        surface_format);
  std::vector<vk::ImageView> swapchain_image_view_pointers;
  for (const auto &image_view : swapchain_image_views) {
    swapchain_image_view_pointers.push_back(*image_view);
  }

  vk::UniqueRenderPass render_pass =
      vka::create_render_pass(*device, surface_format.format);

  vk::UniquePipeline graphics_pipeline =
      vka::create_graphics_pipeline(*device, *render_pass, swapchain_extent);

  std::vector<vk::UniqueFramebuffer> framebuffers = vka::create_framebuffers(
      *device, *render_pass, swapchain_extent, swapchain_image_view_pointers);
  std::vector<vk::Framebuffer> framebuffer_pointers;
  for (const auto &framebuffer : framebuffers) {
    framebuffer_pointers.push_back(*framebuffer);
  }

  vk::UniqueCommandPool command_pool =
      vka::create_command_pool(*device, queue_index);
  std::vector<vk::UniqueCommandBuffer> command_buffers =
      vka::create_command_buffers(*device, *command_pool,
                                  static_cast<uint32_t>(framebuffers.size()));
  std::vector<vk::CommandBuffer> command_buffer_pointers;
  for (const auto &command_buffer : command_buffers) {
    command_buffer_pointers.push_back(*command_buffer);
  }

  vka::record_command_buffers(*device, command_buffer_pointers, *render_pass,
                              *graphics_pipeline, framebuffer_pointers,
                              swapchain_extent);

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
    vka::draw_frame(*device, *swapchain, command_buffer_pointers, queue_index);
  }

  (*device).waitIdle();
}

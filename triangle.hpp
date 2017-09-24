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

#ifndef VULKANALIA_TRIANGLE
#define VULKANALIA_TRIANGLE

#ifndef VK_USE_PLATFORM_WIN32_KHR
#define VK_USE_PLATFORM_WIN32_KHR
#endif
#include <vulkan/vulkan.hpp>

class VulkanController {
public:
  ~VulkanController();
  void initialize(const vk::Instance &instance, const vk::SurfaceKHR &surface,
                  const vk::Extent2D swapchain_extent);
  void recreate_swapchain(vk::Extent2D swapchain_extent);
  void release_swapchain();
  void draw();

private:
  vk::PhysicalDevice physical_device_;
  vk::UniqueDevice device_;
  vk::UniqueCommandPool command_pool_;
  uint32_t queue_index_;

  vk::SurfaceKHR surface_;
  vk::SurfaceFormatKHR surface_format_;
  vk::Extent2D swapchain_extent_;
  vk::UniqueSwapchainKHR swapchain_;
  std::vector<vk::UniqueImageView> swapchain_image_views_;

  vk::UniqueRenderPass render_pass_;
  vk::UniquePipeline graphics_pipeline_;
  std::vector<vk::UniqueFramebuffer> framebuffers_;
  std::vector<vk::UniqueCommandBuffer> command_buffers_;
};

namespace vka {
class WindowManager {
public:
  WindowManager(const std::string &name);
  ~WindowManager();
  HINSTANCE hInstance() const;
  HWND hWnd() const;

private:
  HINSTANCE hInstance_;
  HWND hWnd_;
  const std::string class_name_;
  ATOM register_window_class(HINSTANCE hInstance,
                             const std::string &class_name);
  HWND create_window(HINSTANCE hInstance, const std::string &class_name);
};
struct Version {
  uint32_t major;
  uint32_t minor;
  uint32_t patch;
};
vk::ApplicationInfo create_application_info(const std::string name,
                                            const Version version);
vk::UniqueInstance create_instance(const vk::ApplicationInfo application_info);
vk::PhysicalDevice
select_physical_device(const std::vector<vk::PhysicalDevice> &devices);
uint32_t find_graphics_queue_family_index(
    const std::vector<vk::QueueFamilyProperties> &queues);
vk::UniqueDevice create_device(const vk::PhysicalDevice &physical_device,
                               const uint32_t queue_index);
vk::UniqueCommandPool create_command_pool(const vk::Device &device,
                                          const uint32_t queue_index);
std::vector<vk::UniqueCommandBuffer>
create_command_buffers(const vk::Device &device,
                       const vk::CommandPool &command_pool,
                       const uint32_t command_buffer_count);
vk::UniqueSurfaceKHR create_surface(const vk::Instance &instance,
                                    HINSTANCE hInstance, HWND hWnd);
std::vector<vk::Bool32>
get_presentation_support(const vk::PhysicalDevice &physical_device,
                         const vk::SurfaceKHR &surface,
                         const size_t queue_family_count);
uint32_t find_graphics_and_presentation_queue_family_index(
    const std::vector<vk::QueueFamilyProperties> &queue_properties,
    const std::vector<vk::Bool32> &presentation_support);
vk::SurfaceFormatKHR
select_surface_format(const std::vector<vk::SurfaceFormatKHR> &formats);
vk::Extent2D
select_swapchain_extent(const vk::SurfaceCapabilitiesKHR &capabilities,
                        uint32_t &width, uint32_t &height);
vk::UniqueSwapchainKHR
create_swapchain(const vk::SurfaceFormatKHR &surface_format,
                 const vk::Extent2D &extent,
                 const vk::SurfaceCapabilitiesKHR &capabilities,
                 const vk::Device &device, const vk::SurfaceKHR &surface);
std::vector<vk::UniqueImageView>
create_swapchain_image_views(const vk::Device &device,
                             const std::vector<vk::Image> images,
                             const vk::SurfaceFormatKHR &surface_format);
std::vector<char> read_file(const std::string &file_name);
vk::UniqueShaderModule create_shader_module(const vk::Device &device,
                                            const std::vector<char> &code);
vk::UniquePipelineLayout create_pipeline_layout(const vk::Device &device);
vk::UniqueRenderPass create_render_pass(const vk::Device &device,
                                        const vk::Format &surface_format);
vk::UniquePipeline
create_graphics_pipeline(const vk::Device &device,
                         const vk::RenderPass &render_pass,
                         const vk::Extent2D &swapchain_extent);
std::vector<vk::UniqueFramebuffer>
create_framebuffers(const vk::Device &device, const vk::RenderPass &render_pass,
                    const vk::Extent2D &swapchain_extent,
                    const std::vector<vk::ImageView> &swapchain_image_views);
void record_command_buffers(
    const vk::Device &device,
    const std::vector<vk::CommandBuffer> &command_buffers,
    const vk::RenderPass &render_pass, const vk::Pipeline &graphics_pipeline,
    const std::vector<vk::Framebuffer> &framebuffers,
    const vk::Extent2D &swapchain_extent);
void draw_frame(const vk::Device &device, const vk::SwapchainKHR &swapchain,
                const std::vector<vk::CommandBuffer> &command_buffers,
                const uint32_t queue_index);
}
#endif
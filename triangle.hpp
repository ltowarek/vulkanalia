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

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

namespace vka {
struct Vertex {
  glm::vec2 position;
  glm::vec3 color;
};
vk::VertexInputBindingDescription get_binding_description();
std::vector<vk::VertexInputAttributeDescription> get_attribute_descriptions();
struct Version {
  uint32_t major;
  uint32_t minor;
  uint32_t patch;
};
vk::ApplicationInfo create_application_info(const std::string name,
                                            const Version version);
vk::UniqueInstance
create_instance(const vk::ApplicationInfo application_info,
                const std::vector<const char *> &required_extension_names);
vk::PhysicalDevice
select_physical_device(const std::vector<vk::PhysicalDevice> &devices);
uint32_t find_graphics_queue_family_index(
    const std::vector<vk::QueueFamilyProperties> &queues);
vk::UniqueDevice create_device(const vk::PhysicalDevice &physical_device,
                               const uint32_t queue_index);
vk::UniqueCommandPool create_command_pool(const vk::Device &device,
                                          const uint32_t queue_index);
vk::UniqueBuffer create_vertex_buffer(const vk::Device &device,
                                      const uint32_t size);
uint32_t find_memory_type(
    const vk::PhysicalDeviceMemoryProperties physical_device_memory_properties,
    const uint32_t required_memory_type,
    const vk::MemoryPropertyFlags required_memory_properties);
vk::UniqueDeviceMemory
allocate_buffer_memory(const vk::Device &device, const vk::Buffer buffer,
                       const vk::PhysicalDeviceMemoryProperties
                           &physical_device_memory_properties);
void fill_vertex_buffer(const vk::Device &device,
                        const vk::DeviceMemory &buffer_memory,
                        const std::vector<vka::Vertex> &vertices);
std::vector<vk::UniqueCommandBuffer>
create_command_buffers(const vk::Device &device,
                       const vk::CommandPool &command_pool,
                       const uint32_t command_buffer_count);
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
vk::UniqueSwapchainKHR create_swapchain(
    const vk::SurfaceFormatKHR &surface_format, const vk::Extent2D &extent,
    const vk::SurfaceCapabilitiesKHR &capabilities, const vk::Device &device,
    const vk::SurfaceKHR &surface, const vk::SwapchainKHR &old_swapchain);
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
    const vk::Extent2D &swapchain_extent, const vk::Buffer &vertex_buffer,
    const std::vector<Vertex> &vertices);
void draw_frame(const vk::Device &device, const vk::SwapchainKHR &swapchain,
                const std::vector<vk::CommandBuffer> &command_buffers,
                const uint32_t queue_index);
class VulkanController {
public:
  VulkanController();
  ~VulkanController();
  void initialize(vk::UniqueInstance instance, vk::UniqueSurfaceKHR surface,
                  const vk::Extent2D swapchain_extent);
  void recreate_swapchain(vk::Extent2D swapchain_extent);
  void release();
  void release_swapchain();
  void draw();

private:
  vk::PhysicalDevice physical_device_;
  uint32_t queue_index_;
  vk::SurfaceFormatKHR surface_format_;
  vk::Extent2D swapchain_extent_;

  vk::UniqueInstance instance_;
  vk::UniqueSurfaceKHR surface_;
  vk::UniqueDevice device_;
  vk::UniqueCommandPool command_pool_;
  vk::UniqueBuffer vertex_buffer_;
  vk::UniqueDeviceMemory vertex_buffer_memory_;
  vk::UniqueSwapchainKHR swapchain_;
  std::vector<vk::UniqueImageView> swapchain_image_views_;
  vk::UniqueRenderPass render_pass_;
  vk::UniquePipeline graphics_pipeline_;
  std::vector<vk::UniqueCommandBuffer> command_buffers_;
  std::vector<vk::UniqueFramebuffer> framebuffers_;

  const std::vector<Vertex> vertices_;
};
class TriangleApplication {
public:
  void run();
  void recreate_swapchain();
  static void resize(GLFWwindow *window, int width, int height);

private:
  vka::VulkanController vulkan_controller_;
  GLFWwindow *window_;
};
}
#endif
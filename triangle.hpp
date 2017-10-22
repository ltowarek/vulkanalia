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
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

#include <chrono>

namespace vka {
struct Texture {
  uint32_t width;
  uint32_t height;
  uint64_t size;
  std::unique_ptr<uint8_t, void (*)(uint8_t *)> data;
  Texture();
  Texture(const std::string &file_name);
};
float get_delta_time_per_second(
    const std::chrono::time_point<std::chrono::high_resolution_clock>
        start_time,
    const std::chrono::time_point<std::chrono::high_resolution_clock>
        current_time);
struct Vertex {
  glm::vec2 position;
  glm::vec3 color;
};
struct UniformBufferObject {
  glm::mat4 model;
  glm::mat4 view;
  glm::mat4 projection;
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
vk::UniqueBuffer create_buffer(const vk::Device &device, const uint32_t size,
                               const vk::BufferUsageFlags usage);
uint32_t find_memory_type(
    const vk::PhysicalDeviceMemoryProperties physical_device_memory_properties,
    const uint32_t required_memory_type,
    const vk::MemoryPropertyFlags required_memory_properties);
vk::UniqueDeviceMemory allocate_buffer_memory(
    const vk::Device &device, const vk::Buffer &buffer,
    const vk::PhysicalDeviceMemoryProperties &physical_device_memory_properties,
    const vk::MemoryPropertyFlags &buffer_memory_properties);
template <typename T>
void fill_buffer(const vk::Device &device,
                 const vk::DeviceMemory &buffer_memory, const T &data);
template <typename T>
void fill_buffer(const vk::Device &device,
                 const vk::DeviceMemory &buffer_memory,
                 const std::vector<T> &data);
void fill_buffer(const vk::Device &device,
                 const vk::DeviceMemory &buffer_memory, const Texture &data);
vk::UniqueCommandBuffer begin_command(const vk::Device &device,
                                      const vk::CommandPool &command_pool);
void end_command(const vk::Device &device,
                 vk::UniqueCommandBuffer command_buffer,
                 const uint32_t queue_index);
void copy_buffer_to_buffer(const vk::Device &device,
                           const vk::Buffer &source_buffer,
                           const vk::Buffer &destination_buffer,
                           const uint32_t size,
                           const vk::CommandPool &command_pool,
                           const uint32_t queue_index);
void copy_buffer_to_image(const vk::Device &device,
                          const vk::Buffer &source_buffer,
                          const vk::Image &destination_image,
                          const uint32_t width, const uint32_t height,
                          const vk::CommandPool &command_pool,
                          const uint32_t queue_index);
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
vk::UniqueImageView create_image_view(const vk::Device &device,
                                      const vk::Image &image,
                                      const vk::Format &format);
std::vector<vk::UniqueImageView>
create_swapchain_image_views(const vk::Device &device,
                             const std::vector<vk::Image> images,
                             const vk::SurfaceFormatKHR &surface_format);
std::vector<char> read_file(const std::string &file_name);
vk::UniqueShaderModule create_shader_module(const vk::Device &device,
                                            const std::vector<char> &code);
vk::UniquePipelineLayout
create_pipeline_layout(const vk::Device &device,
                       const vk::DescriptorSetLayout &descriptor_set_layout);
vk::UniqueRenderPass create_render_pass(const vk::Device &device,
                                        const vk::Format &surface_format);
vk::UniqueDescriptorPool create_descriptor_pool(const vk::Device &device);
vk::UniqueDescriptorSetLayout
create_descriptor_set_layout(const vk::Device &device);
std::vector<vk::UniqueDescriptorSet>
create_descriptor_sets(const vk::Device &device,
                       const vk::DescriptorPool &descriptor_pool,
                       const vk::DescriptorSetLayout &descriptor_set_layout);
void update_descriptor_sets(
    const vk::Device &device,
    const std::vector<vk::DescriptorSet> &descriptor_sets,
    const vk::Buffer &uniform_buffer);
vk::UniquePipeline
create_graphics_pipeline(const vk::Device &device,
                         const vk::RenderPass &render_pass,
                         const vk::Extent2D &swapchain_extent,
                         const vk::PipelineLayout &pipeline_layout);
std::vector<vk::UniqueFramebuffer>
create_framebuffers(const vk::Device &device, const vk::RenderPass &render_pass,
                    const vk::Extent2D &swapchain_extent,
                    const std::vector<vk::ImageView> &swapchain_image_views);
void record_command_buffers(
    const vk::Device &device,
    const std::vector<vk::CommandBuffer> &command_buffers,
    const vk::RenderPass &render_pass, const vk::Pipeline &graphics_pipeline,
    const vk::PipelineLayout &pipeline_layout,
    const std::vector<vk::Framebuffer> &framebuffers,
    const vk::Extent2D &swapchain_extent, const vk::Buffer &vertex_buffer,
    const vk::Buffer &index_buffer, const std::vector<uint16_t> &indices,
    const std::vector<vk::DescriptorSet> &descriptor_sets);
void draw_frame(const vk::Device &device, const vk::SwapchainKHR &swapchain,
                const std::vector<vk::CommandBuffer> &command_buffers,
                const uint32_t queue_index);
vk::UniqueImage create_image(const vk::Device &device, const uint32_t width,
                             const uint32_t height, const vk::Format format,
                             const vk::ImageTiling tiling,
                             const vk::ImageUsageFlags usage);
vk::UniqueDeviceMemory allocate_image_memory(
    const vk::Device &device, const vk::Image &image,
    const vk::PhysicalDeviceMemoryProperties &physical_device_memory_properties,
    const vk::MemoryPropertyFlags &image_memory_properties);
void transition_image_layout(const vk::Device &device,
                             const vk::CommandPool &command_pool,
                             const uint32_t queue_index,
                             const vk::ImageLayout old_layout,
                             const vk::ImageLayout new_layout,
                             const vk::Image &image);
vk::UniqueImageView create_texture_image_view(const vk::Device &device,
                                              const vk::Image &image);
vk::UniqueSampler create_texture_sampler(const vk::Device &device);
class VulkanController {
public:
  VulkanController();
  ~VulkanController();
  void initialize(vk::UniqueInstance instance, vk::UniqueSurfaceKHR surface,
                  const vk::Extent2D swapchain_extent);
  void recreate_swapchain(vk::Extent2D swapchain_extent);
  void release();
  void release_swapchain();
  void update();
  void draw();

private:
  void create_uniform_buffer();
  void update_uniform_buffer(const float delta_time);
  void create_vertex_buffer();
  void create_index_buffer();
  vk::PhysicalDevice physical_device_;
  uint32_t queue_index_;
  vk::SurfaceFormatKHR surface_format_;
  vk::Extent2D swapchain_extent_;

  vk::UniqueInstance instance_;
  vk::UniqueSurfaceKHR surface_;
  vk::UniqueDevice device_;
  vk::UniqueCommandPool command_pool_;
  vk::UniqueDescriptorPool descriptor_pool_;
  vk::UniqueDescriptorSetLayout descriptor_set_layout_;
  std::vector<vk::UniqueDescriptorSet> descriptor_sets_;
  vk::UniqueBuffer uniform_buffer_;
  vk::UniqueDeviceMemory uniform_buffer_memory_;
  vk::UniqueBuffer vertex_buffer_;
  vk::UniqueDeviceMemory vertex_buffer_memory_;
  vk::UniqueBuffer index_buffer_;
  vk::UniqueDeviceMemory index_buffer_memory_;
  vk::UniqueSwapchainKHR swapchain_;
  std::vector<vk::UniqueImageView> swapchain_image_views_;
  vk::UniqueRenderPass render_pass_;
  vk::UniquePipelineLayout pipeline_layout_;
  vk::UniquePipeline graphics_pipeline_;
  std::vector<vk::UniqueCommandBuffer> command_buffers_;
  std::vector<vk::UniqueFramebuffer> framebuffers_;

  const std::vector<Vertex> vertices_;
  const std::vector<uint16_t> indices_;
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
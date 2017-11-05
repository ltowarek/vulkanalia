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
#include <fstream>

#include <glm/gtc/matrix_transform.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

namespace vka {
void image_free(uint8_t *t) {
  if (t != nullptr)
    stbi_image_free(t);
}
Texture::Texture() : width(0), height(0), size(0), data(nullptr, image_free) {}
Texture::Texture(const std::string &file_name) : Texture() {
  int tmp_width, tmp_height, tmp_channels;
  uint8_t *tmp_data = stbi_load(file_name.c_str(), &tmp_width, &tmp_height,
                                &tmp_channels, STBI_rgb_alpha);
  data = std::unique_ptr<uint8_t, void (*)(uint8_t *)>(tmp_data, image_free);
  width = static_cast<uint32_t>(tmp_width);
  height = static_cast<uint32_t>(tmp_height);
  size = width * height * STBI_rgb_alpha;
}
float get_delta_time_per_second(
    const std::chrono::time_point<std::chrono::high_resolution_clock>
        start_time,
    const std::chrono::time_point<std::chrono::high_resolution_clock>
        current_time) {
  const auto delta_time = std::chrono::duration_cast<std::chrono::milliseconds>(
      current_time - start_time);
  return delta_time.count() / static_cast<float>(std::milli::den);
}
vk::VertexInputBindingDescription get_binding_description() {
  vk::VertexInputBindingDescription description;
  description.binding = 0;
  description.stride = sizeof(Vertex);
  description.inputRate = vk::VertexInputRate::eVertex;
  return description;
}
std::vector<vk::VertexInputAttributeDescription> get_attribute_descriptions() {
  std::vector<vk::VertexInputAttributeDescription> descriptions(3);
  descriptions[0].binding = 0;
  descriptions[0].location = 0;
  descriptions[0].format = vk::Format::eR32G32B32Sfloat;
  descriptions[0].offset = offsetof(Vertex, position);

  descriptions[1].binding = 0;
  descriptions[1].location = 1;
  descriptions[1].format = vk::Format::eR32G32B32Sfloat;
  descriptions[1].offset = offsetof(Vertex, color);

  descriptions[2].binding = 0;
  descriptions[2].location = 2;
  descriptions[2].format = vk::Format::eR32G32Sfloat;
  descriptions[2].offset = offsetof(Vertex, texture_coordinates);

  return descriptions;
}
vk::ApplicationInfo create_application_info(const std::string name,
                                            const Version version) {
  vk::ApplicationInfo info;
  info.pApplicationName = name.c_str();
  info.applicationVersion =
      VK_MAKE_VERSION(version.major, version.minor, version.patch);
  info.apiVersion = VK_API_VERSION_1_0;
  return info;
}
vk::UniqueInstance
create_instance(const vk::ApplicationInfo application_info,
                const std::vector<const char *> &required_extension_names) {
  vk::InstanceCreateInfo info;
  info.pApplicationInfo = &application_info;
  std::vector<const char *> extension_names = required_extension_names;
  info.enabledExtensionCount = static_cast<uint32_t>(extension_names.size());
  info.ppEnabledExtensionNames = extension_names.data();
  return vk::createInstanceUnique(info);
}
vk::PhysicalDevice
select_physical_device(const std::vector<vk::PhysicalDevice> &devices) {
  if (devices.empty()) {
    return vk::PhysicalDevice();
  }
  return devices[0];
}
uint32_t find_graphics_queue_family_index(
    const std::vector<vk::QueueFamilyProperties> &queues) {
  auto queue = std::find_if(
      queues.begin(), queues.end(), [](const vk::QueueFamilyProperties &q) {
        return q.queueFlags & vk::QueueFlagBits::eGraphics;
      });
  if (queue == queues.end()) {
    return UINT32_MAX;
  }
  return static_cast<uint32_t>(queue - queues.begin());
}
vk::UniqueDevice create_device(const vk::PhysicalDevice &physical_device,
                               const uint32_t queue_index) {
  const std::vector<float> queues_priorities = {0.0f};
  vk::DeviceQueueCreateInfo queue_info;
  queue_info.queueCount = 1;
  queue_info.queueFamilyIndex = queue_index;
  queue_info.pQueuePriorities = queues_priorities.data();

  vk::DeviceCreateInfo device_info;
  device_info.queueCreateInfoCount = 1;
  device_info.pQueueCreateInfos = &queue_info;
  std::vector<const char *> extension_names;
  extension_names.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
  device_info.enabledExtensionCount =
      static_cast<uint32_t>(extension_names.size());
  device_info.ppEnabledExtensionNames = extension_names.data();

  vk::PhysicalDeviceFeatures physical_device_features;
  physical_device_features.samplerAnisotropy = VK_TRUE;

  device_info.pEnabledFeatures = &physical_device_features;

  return physical_device.createDeviceUnique(device_info);
}
vk::UniqueCommandPool create_command_pool(const vk::Device &device,
                                          const uint32_t queue_index) {
  vk::CommandPoolCreateInfo info;
  info.queueFamilyIndex = queue_index;
  return device.createCommandPoolUnique(info);
}
vk::UniqueBuffer create_buffer(const vk::Device &device, const uint32_t size,
                               const vk::BufferUsageFlags usage) {
  vk::BufferCreateInfo info;
  info.size = size;
  info.usage = usage;
  info.sharingMode = vk::SharingMode::eExclusive;
  return device.createBufferUnique(info);
}
uint32_t find_memory_type(
    const vk::PhysicalDeviceMemoryProperties physical_device_memory_properties,
    const uint32_t required_memory_type,
    const vk::MemoryPropertyFlags required_memory_properties) {
  for (uint32_t i = 0; i < physical_device_memory_properties.memoryTypeCount;
       ++i) {
    if ((required_memory_type & (1 << i)) &&
        (physical_device_memory_properties.memoryTypes[i].propertyFlags &
         required_memory_properties) == required_memory_properties) {
      return i;
    }
  }
  return UINT32_MAX;
}
vk::UniqueDeviceMemory allocate_buffer_memory(
    const vk::Device &device, const vk::Buffer &buffer,
    const vk::PhysicalDeviceMemoryProperties &physical_device_memory_properties,
    const vk::MemoryPropertyFlags &buffer_memory_properties) {
  vk::MemoryRequirements memory_requirements =
      device.getBufferMemoryRequirements(buffer);
  vk::MemoryAllocateInfo info;
  info.allocationSize = memory_requirements.size;
  info.memoryTypeIndex = find_memory_type(physical_device_memory_properties,
                                          memory_requirements.memoryTypeBits,
                                          buffer_memory_properties);
  return device.allocateMemoryUnique(info);
}
template <typename T>
void fill_buffer(const vk::Device &device,
                 const vk::DeviceMemory &buffer_memory, const T &data) {
  const size_t size = sizeof(T);
  void *pointer;
  device.mapMemory(buffer_memory, 0, size, vk::MemoryMapFlags(), &pointer);
  std::memcpy(pointer, &data, size);
  device.unmapMemory(buffer_memory);
}
void fill_buffer(const vk::Device &device,
                 const vk::DeviceMemory &buffer_memory, const Texture &data) {
  void *pointer;
  device.mapMemory(buffer_memory, 0, data.size, vk::MemoryMapFlags(), &pointer);
  std::memcpy(pointer, data.data.get(), data.size);
  device.unmapMemory(buffer_memory);
}
template <typename T>
void fill_buffer(const vk::Device &device,
                 const vk::DeviceMemory &buffer_memory,
                 const std::vector<T> &data) {
  const size_t size = sizeof(T) * data.size();
  void *pointer;
  device.mapMemory(buffer_memory, 0, size, vk::MemoryMapFlags(), &pointer);
  std::memcpy(pointer, data.data(), size);
  device.unmapMemory(buffer_memory);
}
vk::UniqueCommandBuffer begin_command(const vk::Device &device,
                                      const vk::CommandPool &command_pool) {
  std::vector<vk::UniqueCommandBuffer> command_buffers =
      create_command_buffers(device, command_pool, 1);
  vk::UniqueCommandBuffer command_buffer = std::move(command_buffers[0]);

  vk::CommandBufferBeginInfo command_buffer_begin_info;
  command_buffer_begin_info.flags =
      vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

  (*command_buffer).begin(command_buffer_begin_info);
  return std::move(command_buffer);
}
void end_command(const vk::Device &device,
                 vk::UniqueCommandBuffer command_buffer,
                 const uint32_t queue_index) {
  (*command_buffer).end();
  vk::SubmitInfo submit_info;
  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers = &(*command_buffer);

  vk::Queue queue = device.getQueue(queue_index, 0);
  queue.submit(submit_info, vk::Fence());
  queue.waitIdle();
}
void copy_buffer_to_buffer(const vk::Device &device,
                           const vk::Buffer &source_buffer,
                           const vk::Buffer &destination_buffer,
                           const uint32_t size,
                           const vk::CommandPool &command_pool,
                           const uint32_t queue_index) {
  vk::UniqueCommandBuffer command_buffer = begin_command(device, command_pool);

  const vk::BufferCopy region(0, 0, size);
  (*command_buffer).copyBuffer(source_buffer, destination_buffer, 1, &region);

  end_command(device, std::move(command_buffer), queue_index);
}
void copy_buffer_to_image(const vk::Device &device,
                          const vk::Buffer &source_buffer,
                          const vk::Image &destination_image,
                          const uint32_t width, const uint32_t height,
                          const vk::CommandPool &command_pool,
                          const uint32_t queue_index) {
  vk::UniqueCommandBuffer command_buffer = begin_command(device, command_pool);

  vk::BufferImageCopy region;
  region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
  region.imageSubresource.layerCount = 1;
  region.imageExtent = vk::Extent3D(width, height, 1);

  (*command_buffer)
      .copyBufferToImage(source_buffer, destination_image,
                         vk::ImageLayout::eTransferDstOptimal, 1, &region);

  end_command(device, std::move(command_buffer), queue_index);
}
std::vector<vk::UniqueCommandBuffer>
create_command_buffers(const vk::Device &device,
                       const vk::CommandPool &command_pool,
                       const uint32_t command_buffer_count) {
  vk::CommandBufferAllocateInfo info;
  info.commandPool = command_pool;
  info.commandBufferCount = command_buffer_count;
  info.level = vk::CommandBufferLevel::ePrimary;
  return device.allocateCommandBuffersUnique(info);
}
std::vector<vk::Bool32>
get_presentation_support(const vk::PhysicalDevice &physical_device,
                         const vk::SurfaceKHR &surface,
                         const size_t queue_family_count) {
  std::vector<vk::Bool32> presentation_support(queue_family_count);
  for (size_t i = 0; i < queue_family_count; ++i) {
    presentation_support[i] =
        physical_device.getSurfaceSupportKHR(static_cast<uint32_t>(i), surface);
  }
  return presentation_support;
}
uint32_t find_graphics_and_presentation_queue_family_index(
    const std::vector<vk::QueueFamilyProperties> &queue_properties,
    const std::vector<vk::Bool32> &presentation_support) {
  if (queue_properties.size() != presentation_support.size()) {
    return UINT32_MAX;
  }
  for (size_t i = 0; i < queue_properties.size(); ++i) {
    if (queue_properties[i].queueFlags & vk::QueueFlagBits::eGraphics &&
        presentation_support[i] == VK_TRUE) {
      return static_cast<uint32_t>(i);
    }
  }
  return UINT32_MAX;
}
vk::SurfaceFormatKHR
select_surface_format(const std::vector<vk::SurfaceFormatKHR> &formats) {
  if (formats.size() == 1 && formats[0].format == vk::Format::eUndefined) {
    return {vk::Format::eB8G8R8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear};
  } else {
    return formats[0];
  }
}
vk::Extent2D
select_swapchain_extent(const vk::SurfaceCapabilitiesKHR &capabilities,
                        uint32_t &width, uint32_t &height) {
  vk::Extent2D extent = capabilities.currentExtent;
  if (extent.width == UINT32_MAX) {
    extent.width = width;
    extent.height = height;
  } else {
    width = extent.width;
    height = extent.height;
  }
  return extent;
}
vk::UniqueSwapchainKHR create_swapchain(
    const vk::SurfaceFormatKHR &surface_format, const vk::Extent2D &extent,
    const vk::SurfaceCapabilitiesKHR &capabilities, const vk::Device &device,
    const vk::SurfaceKHR &surface, const vk::SwapchainKHR &old_swapchain) {
  vk::SwapchainCreateInfoKHR info;
  info.surface = surface;
  info.minImageCount = capabilities.minImageCount;
  info.imageFormat = surface_format.format;
  info.imageColorSpace = surface_format.colorSpace;
  info.imageExtent = extent;
  info.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;
  info.preTransform = capabilities.currentTransform;
  info.imageArrayLayers = 1;
  info.imageSharingMode = vk::SharingMode::eExclusive;
  info.queueFamilyIndexCount = 0;
  info.pQueueFamilyIndices = nullptr;
  info.presentMode = vk::PresentModeKHR::eFifo;
  info.oldSwapchain = old_swapchain;
  info.clipped = VK_TRUE;
  info.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
  return device.createSwapchainKHRUnique(info);
}
vk::UniqueImageView create_image_view(const vk::Device &device,
                                      const vk::Image &image,
                                      const vk::Format &format,
                                      const vk::ImageAspectFlags &aspect) {
  vk::ImageViewCreateInfo info;
  info.image = image;
  info.viewType = vk::ImageViewType::e2D;
  info.format = format;
  info.components.r = vk::ComponentSwizzle::eR;
  info.components.g = vk::ComponentSwizzle::eG;
  info.components.b = vk::ComponentSwizzle::eB;
  info.components.a = vk::ComponentSwizzle::eA;
  info.subresourceRange.aspectMask = aspect;
  info.subresourceRange.baseMipLevel = 0;
  info.subresourceRange.levelCount = 1;
  info.subresourceRange.baseArrayLayer = 0;
  info.subresourceRange.layerCount = 1;
  return device.createImageViewUnique(info);
}
std::vector<vk::UniqueImageView>
create_swapchain_image_views(const vk::Device &device,
                             const std::vector<vk::Image> images,
                             const vk::SurfaceFormatKHR &surface_format) {
  std::vector<vk::UniqueImageView> image_views(images.size());
  for (size_t i = 0; i < images.size(); ++i) {
    image_views[i] = create_image_view(device, images[i], surface_format.format,
                                       vk::ImageAspectFlagBits::eColor);
  }
  return image_views;
}
std::vector<char> read_file(const std::string &file_name) {
  std::ifstream f(file_name, std::ios::binary);
  return std::vector<char>((std::istreambuf_iterator<char>(f)),
                           std::istreambuf_iterator<char>());
}
vk::UniqueShaderModule create_shader_module(const vk::Device &device,
                                            const std::vector<char> &code) {
  vk::ShaderModuleCreateInfo info;
  info.codeSize = code.size();
  info.pCode = reinterpret_cast<const uint32_t *>(code.data());
  return device.createShaderModuleUnique(info);
}
vk::UniquePipelineLayout
create_pipeline_layout(const vk::Device &device,
                       const vk::DescriptorSetLayout &descriptor_set_layout) {
  vk::PipelineLayoutCreateInfo info;
  info.setLayoutCount = 1;
  info.pSetLayouts = &descriptor_set_layout;
  return device.createPipelineLayoutUnique(info);
}
vk::UniqueRenderPass create_render_pass(const vk::Device &device,
                                        const vk::Format &surface_format) {
  vk::AttachmentDescription color_attachment;
  color_attachment.format = surface_format;
  color_attachment.loadOp = vk::AttachmentLoadOp::eClear;
  color_attachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
  color_attachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
  color_attachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;

  vk::AttachmentReference color_attachment_reference;
  color_attachment_reference.attachment = 0;
  color_attachment_reference.layout = vk::ImageLayout::eColorAttachmentOptimal;

  vk::AttachmentDescription depth_attachment;
  depth_attachment.format = vk::Format::eD32Sfloat;
  depth_attachment.loadOp = vk::AttachmentLoadOp::eClear;
  depth_attachment.storeOp = vk::AttachmentStoreOp::eDontCare;
  depth_attachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
  depth_attachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
  depth_attachment.finalLayout =
      vk::ImageLayout::eDepthStencilAttachmentOptimal;

  vk::AttachmentReference depth_attachment_reference;
  depth_attachment_reference.layout =
      vk::ImageLayout::eDepthStencilAttachmentOptimal;
  depth_attachment_reference.attachment = 1;

  std::array<vk::AttachmentDescription, 2> attachments = {color_attachment,
                                                          depth_attachment};

  vk::SubpassDescription subpass;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &color_attachment_reference;
  subpass.pDepthStencilAttachment = &depth_attachment_reference;

  vk::RenderPassCreateInfo info;
  info.attachmentCount = static_cast<uint32_t>(attachments.size());
  info.pAttachments = attachments.data();
  info.subpassCount = 1;
  info.pSubpasses = &subpass;

  return device.createRenderPassUnique(info);
}
vk::UniqueDescriptorPool create_descriptor_pool(const vk::Device &device) {
  std::array<vk::DescriptorPoolSize, 2> pool_sizes;
  pool_sizes[0].type = vk::DescriptorType::eUniformBuffer;
  pool_sizes[0].descriptorCount = 1;
  pool_sizes[1].type = vk::DescriptorType::eCombinedImageSampler;
  pool_sizes[1].descriptorCount = 1;

  vk::DescriptorPoolCreateInfo info;
  info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
  info.pPoolSizes = pool_sizes.data();
  info.maxSets = 1;
  info.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;

  return device.createDescriptorPoolUnique(info);
}
vk::UniqueDescriptorSetLayout
create_descriptor_set_layout(const vk::Device &device) {
  std::array<vk::DescriptorSetLayoutBinding, 2> bindings;
  bindings[0].binding = 0;
  bindings[0].descriptorCount = 1;
  bindings[0].descriptorType = vk::DescriptorType::eUniformBuffer;
  bindings[0].stageFlags = vk::ShaderStageFlagBits::eVertex;

  bindings[1].binding = 1;
  bindings[1].descriptorCount = 1;
  bindings[1].descriptorType = vk::DescriptorType::eCombinedImageSampler;
  bindings[1].stageFlags = vk::ShaderStageFlagBits::eFragment;

  vk::DescriptorSetLayoutCreateInfo info;
  info.bindingCount = static_cast<uint32_t>(bindings.size());
  info.pBindings = bindings.data();

  return device.createDescriptorSetLayoutUnique(info);
}
std::vector<vk::UniqueDescriptorSet>
create_descriptor_sets(const vk::Device &device,
                       const vk::DescriptorPool &descriptor_pool,
                       const vk::DescriptorSetLayout &descriptor_set_layout) {
  std::vector<vk::DescriptorSetLayout> layouts = {descriptor_set_layout};
  vk::DescriptorSetAllocateInfo info;
  info.descriptorPool = descriptor_pool;
  info.descriptorSetCount = 1;
  info.pSetLayouts = layouts.data();
  return device.allocateDescriptorSetsUnique(info);
}
void update_descriptor_sets(
    const vk::Device &device,
    const std::vector<vk::DescriptorSet> &descriptor_sets,
    const vk::Buffer &uniform_buffer, const vk::ImageView &image_view,
    const vk::Sampler &sampler) {
  vk::DescriptorBufferInfo buffer_info;
  buffer_info.buffer = uniform_buffer;
  buffer_info.range = sizeof(vka::UniformBufferObject);

  vk::DescriptorImageInfo image_info;
  image_info.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
  image_info.imageView = image_view;
  image_info.sampler = sampler;

  std::array<vk::WriteDescriptorSet, 2> descriptor_writes;
  descriptor_writes[0].dstSet = descriptor_sets[0];
  descriptor_writes[0].dstBinding = 0;
  descriptor_writes[0].descriptorType = vk::DescriptorType::eUniformBuffer;
  descriptor_writes[0].descriptorCount = 1;
  descriptor_writes[0].pBufferInfo = &buffer_info;

  descriptor_writes[1].dstSet = descriptor_sets[0];
  descriptor_writes[1].dstBinding = 1;
  descriptor_writes[1].descriptorType =
      vk::DescriptorType::eCombinedImageSampler;
  descriptor_writes[1].descriptorCount = 1;
  descriptor_writes[1].pImageInfo = &image_info;

  device.updateDescriptorSets(descriptor_writes, {});
}
vk::UniquePipeline
create_graphics_pipeline(const vk::Device &device,
                         const vk::RenderPass &render_pass,
                         const vk::Extent2D &swapchain_extent,
                         const vk::PipelineLayout &pipeline_layout) {
  vk::GraphicsPipelineCreateInfo info;

  vk::UniqueShaderModule vertex_shader_module =
      create_shader_module(device, read_file("vert.spv"));
  vk::PipelineShaderStageCreateInfo vertex_shader_stage;
  vertex_shader_stage.stage = vk::ShaderStageFlagBits::eVertex;
  vertex_shader_stage.module = *vertex_shader_module;
  vertex_shader_stage.pName = "main";

  vk::UniqueShaderModule fragment_shader_module =
      create_shader_module(device, read_file("frag.spv"));
  vk::PipelineShaderStageCreateInfo fragment_shader_stage;
  fragment_shader_stage.stage = vk::ShaderStageFlagBits::eFragment;
  fragment_shader_stage.module = *fragment_shader_module;
  fragment_shader_stage.pName = "main";

  std::vector<vk::PipelineShaderStageCreateInfo> stages = {
      vertex_shader_stage, fragment_shader_stage};
  info.stageCount = static_cast<uint32_t>(stages.size());
  info.pStages = stages.data();

  vk::PipelineVertexInputStateCreateInfo vertex_input_state;
  vk::VertexInputBindingDescription binding_description =
      get_binding_description();
  std::vector<vk::VertexInputAttributeDescription> attribute_descriptions =
      get_attribute_descriptions();
  vertex_input_state.vertexBindingDescriptionCount = 1;
  vertex_input_state.pVertexBindingDescriptions = &binding_description;
  vertex_input_state.vertexAttributeDescriptionCount =
      static_cast<uint32_t>(attribute_descriptions.size());
  vertex_input_state.pVertexAttributeDescriptions =
      attribute_descriptions.data();
  info.pVertexInputState = &vertex_input_state;

  vk::PipelineInputAssemblyStateCreateInfo input_assembly_state;
  input_assembly_state.topology = vk::PrimitiveTopology::eTriangleList;
  info.pInputAssemblyState = &input_assembly_state;

  vk::Viewport viewport;
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = static_cast<float>(swapchain_extent.width);
  viewport.height = static_cast<float>(swapchain_extent.height);
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  vk::Rect2D scissor;
  scissor.extent = swapchain_extent;

  vk::PipelineViewportStateCreateInfo viewport_state;
  viewport_state.viewportCount = 1;
  viewport_state.pViewports = &viewport;
  viewport_state.scissorCount = 1;
  viewport_state.pScissors = &scissor;
  info.pViewportState = &viewport_state;

  vk::PipelineRasterizationStateCreateInfo rasterization_state;
  rasterization_state.depthClampEnable = VK_FALSE;
  rasterization_state.rasterizerDiscardEnable = VK_FALSE;
  rasterization_state.polygonMode = vk::PolygonMode::eFill;
  rasterization_state.lineWidth = 1.0f;
  rasterization_state.cullMode = vk::CullModeFlagBits::eBack;
  rasterization_state.frontFace = vk::FrontFace::eCounterClockwise;
  rasterization_state.depthBiasEnable = VK_FALSE;
  info.pRasterizationState = &rasterization_state;

  vk::PipelineMultisampleStateCreateInfo multisample_state;
  info.pMultisampleState = &multisample_state;

  vk::PipelineDepthStencilStateCreateInfo depth_stencil;
  depth_stencil.depthTestEnable = VK_TRUE;
  depth_stencil.depthWriteEnable = VK_TRUE;
  depth_stencil.depthCompareOp = vk::CompareOp::eLess;
  info.pDepthStencilState = &depth_stencil;

  vk::PipelineColorBlendAttachmentState color_blend_attachment_state;
  color_blend_attachment_state.colorWriteMask =
      vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
      vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
  color_blend_attachment_state.blendEnable = VK_FALSE;

  vk::PipelineColorBlendStateCreateInfo color_blend_state;
  color_blend_state.attachmentCount = 1;
  color_blend_state.pAttachments = &color_blend_attachment_state;
  info.pColorBlendState = &color_blend_state;

  info.layout = pipeline_layout;

  info.renderPass = render_pass;

  vk::UniquePipelineCache pipeline_cache;
  return device.createGraphicsPipelineUnique(*pipeline_cache, info);
}
std::vector<vk::UniqueFramebuffer>
create_framebuffers(const vk::Device &device, const vk::RenderPass &render_pass,
                    const vk::Extent2D &swapchain_extent,
                    const std::vector<vk::ImageView> &swapchain_image_views,
                    const vk::ImageView &depth_image_view) {
  std::vector<vk::UniqueFramebuffer> framebuffers;
  for (size_t i = 0; i < swapchain_image_views.size(); ++i) {
    std::array<vk::ImageView, 2> attachments = {swapchain_image_views[i],
                                                depth_image_view};
    vk::FramebufferCreateInfo info;
    info.renderPass = render_pass;
    info.attachmentCount = static_cast<uint32_t>(attachments.size());
    info.pAttachments = attachments.data();
    info.width = swapchain_extent.width;
    info.height = swapchain_extent.height;
    info.layers = 1;
    framebuffers.push_back(device.createFramebufferUnique(info));
  }
  return framebuffers;
}
void record_command_buffers(
    const vk::Device &device,
    const std::vector<vk::CommandBuffer> &command_buffers,
    const vk::RenderPass &render_pass, const vk::Pipeline &graphics_pipeline,
    const vk::PipelineLayout &pipeline_layout,
    const std::vector<vk::Framebuffer> &framebuffers,
    const vk::Extent2D &swapchain_extent, const vk::Buffer &vertex_buffer,
    const vk::Buffer &index_buffer, const std::vector<uint16_t> &indices,
    const std::vector<vk::DescriptorSet> &descriptor_sets) {
  for (size_t i = 0; i < command_buffers.size(); ++i) {
    vk::CommandBufferBeginInfo command_buffer_begin_info;
    command_buffer_begin_info.flags =
        vk::CommandBufferUsageFlagBits::eSimultaneousUse;

    command_buffers[i].begin(command_buffer_begin_info);

    vk::RenderPassBeginInfo render_pass_begin_info;
    render_pass_begin_info.renderPass = render_pass;
    render_pass_begin_info.framebuffer = framebuffers[i];
    render_pass_begin_info.renderArea.extent = swapchain_extent;

    std::array<vk::ClearValue, 2> clear_values = {
        vk::ClearValue(
            vk::ClearColorValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f})),
        vk::ClearValue(vk::ClearDepthStencilValue(1.0f, 0))};
    render_pass_begin_info.clearValueCount =
        static_cast<uint32_t>(clear_values.size());
    render_pass_begin_info.pClearValues = clear_values.data();

    command_buffers[i].beginRenderPass(render_pass_begin_info,
                                       vk::SubpassContents::eInline);
    command_buffers[i].bindPipeline(vk::PipelineBindPoint::eGraphics,
                                    graphics_pipeline);
    command_buffers[i].bindVertexBuffers(0, {vertex_buffer}, {0});
    command_buffers[i].bindIndexBuffer({index_buffer}, {0},
                                       vk::IndexType::eUint16);
    command_buffers[i].bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                                          pipeline_layout, 0, descriptor_sets,
                                          {});
    command_buffers[i].drawIndexed(static_cast<uint32_t>(indices.size()), 1, 0,
                                   0, 0);
    command_buffers[i].endRenderPass();

    command_buffers[i].end();
  }
}
void draw_frame(const vk::Device &device, const vk::SwapchainKHR &swapchain,
                const std::vector<vk::CommandBuffer> &command_buffers,
                const uint32_t queue_index) {
  vk::SemaphoreCreateInfo semaphore_info;
  vk::UniqueSemaphore is_image_available =
      device.createSemaphoreUnique(semaphore_info);
  vk::UniqueSemaphore is_rendering_finished =
      device.createSemaphoreUnique(semaphore_info);

  const uint32_t image_index =
      device
          .acquireNextImageKHR(swapchain, UINT64_MAX, *is_image_available,
                               vk::Fence())
          .value;

  vk::SubmitInfo submit_info;
  vk::PipelineStageFlags wait_stages =
      vk::PipelineStageFlagBits::eColorAttachmentOutput;
  submit_info.waitSemaphoreCount = 1;
  submit_info.pWaitSemaphores = &*is_image_available;
  submit_info.pWaitDstStageMask = &wait_stages;
  submit_info.signalSemaphoreCount = 1;
  submit_info.pSignalSemaphores = &*is_rendering_finished;
  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers = &command_buffers[image_index];

  vk::Queue queue = device.getQueue(queue_index, 0);
  queue.submit(submit_info, vk::Fence());

  vk::PresentInfoKHR present_info;
  present_info.waitSemaphoreCount = 1;
  present_info.pWaitSemaphores = &*is_rendering_finished;
  present_info.swapchainCount = 1;
  present_info.pSwapchains = &swapchain;
  present_info.pImageIndices = &image_index;

  queue.presentKHR(present_info);
  queue.waitIdle();
}
vk::UniqueImage create_image(const vk::Device &device, const uint32_t width,
                             const uint32_t height, const vk::Format format,
                             const vk::ImageTiling tiling,
                             const vk::ImageUsageFlags usage) {
  vk::ImageCreateInfo info;
  info.imageType = vk::ImageType::e2D;
  info.extent = vk::Extent3D(width, height, 1);
  info.mipLevels = 1;
  info.arrayLayers = 1;
  info.format = format;
  info.tiling = tiling;
  info.usage = usage;
  return device.createImageUnique(info);
}
vk::UniqueDeviceMemory allocate_image_memory(
    const vk::Device &device, const vk::Image &image,
    const vk::PhysicalDeviceMemoryProperties &physical_device_memory_properties,
    const vk::MemoryPropertyFlags &image_memory_properties) {
  vk::MemoryRequirements memory_requirements =
      device.getImageMemoryRequirements(image);
  vk::MemoryAllocateInfo info;
  info.allocationSize = memory_requirements.size;
  info.memoryTypeIndex = find_memory_type(physical_device_memory_properties,
                                          memory_requirements.memoryTypeBits,
                                          image_memory_properties);
  return device.allocateMemoryUnique(info);
}
struct TransitionProperties {
  vk::AccessFlags source_mask;
  vk::AccessFlags destination_mask;
  vk::PipelineStageFlags source_stage;
  vk::PipelineStageFlags destination_stage;
};
TransitionProperties
get_transition_properties(const vk::ImageLayout old_layout,
                          const vk::ImageLayout new_layout) {
  TransitionProperties properties;
  if (old_layout == vk::ImageLayout::eUndefined &&
      new_layout == vk::ImageLayout::eTransferDstOptimal) {
    properties.source_mask = vk::AccessFlags();
    properties.destination_mask = vk::AccessFlagBits::eTransferWrite;
    properties.source_stage = vk::PipelineStageFlagBits::eTopOfPipe;
    properties.destination_stage = vk::PipelineStageFlagBits::eTransfer;
  } else if (old_layout == vk::ImageLayout::eTransferDstOptimal &&
             new_layout == vk::ImageLayout::eShaderReadOnlyOptimal) {
    properties.source_mask = vk::AccessFlagBits::eTransferWrite;
    properties.destination_mask = vk::AccessFlagBits::eShaderRead;
    properties.source_stage = vk::PipelineStageFlagBits::eTransfer;
    properties.destination_stage = vk::PipelineStageFlagBits::eFragmentShader;
  } else if (old_layout == vk::ImageLayout::eUndefined &&
             new_layout == vk::ImageLayout::eDepthStencilAttachmentOptimal) {
    properties.source_mask = vk::AccessFlags();
    properties.destination_mask =
        vk::AccessFlagBits::eDepthStencilAttachmentRead |
        vk::AccessFlagBits::eDepthStencilAttachmentWrite;
    properties.source_stage = vk::PipelineStageFlagBits::eTopOfPipe;
    properties.destination_stage =
        vk::PipelineStageFlagBits::eEarlyFragmentTests;
  }
  return properties;
}
void transition_image_layout(const vk::Device &device,
                             const vk::CommandPool &command_pool,
                             const uint32_t queue_index,
                             const vk::ImageLayout old_layout,
                             const vk::ImageLayout new_layout,
                             const vk::Image &image) {
  vk::UniqueCommandBuffer command_buffer = begin_command(device, command_pool);

  vk::ImageMemoryBarrier barrier;
  barrier.oldLayout = old_layout;
  barrier.newLayout = new_layout;
  barrier.image = image;

  vk::ImageSubresourceRange subresource;
  subresource.aspectMask =
      new_layout == vk::ImageLayout::eDepthStencilAttachmentOptimal
          ? vk::ImageAspectFlagBits::eDepth
          : vk::ImageAspectFlagBits::eColor;
  subresource.levelCount = 1;
  subresource.layerCount = 1;
  barrier.subresourceRange = subresource;

  const TransitionProperties transition_properties =
      get_transition_properties(old_layout, new_layout);

  barrier.srcAccessMask = transition_properties.source_mask;
  barrier.dstAccessMask = transition_properties.destination_mask;

  (*command_buffer)
      .pipelineBarrier(transition_properties.source_stage,
                       transition_properties.destination_stage,
                       vk::DependencyFlags(), {}, {}, {barrier});

  end_command(device, std::move(command_buffer), queue_index);
}
vk::UniqueImageView create_texture_image_view(const vk::Device &device,
                                              const vk::Image &image) {
  return create_image_view(device, image, vk::Format::eR8G8B8A8Unorm,
                           vk::ImageAspectFlagBits::eColor);
}
vk::UniqueSampler create_texture_sampler(const vk::Device &device) {
  vk::SamplerCreateInfo info;
  info.magFilter = vk::Filter::eLinear;
  info.minFilter = vk::Filter::eLinear;
  info.anisotropyEnable = VK_TRUE;
  info.maxAnisotropy = 16;
  info.borderColor = vk::BorderColor::eIntOpaqueBlack;
  info.compareOp = vk::CompareOp::eAlways;
  info.mipmapMode = vk::SamplerMipmapMode::eLinear;
  return device.createSamplerUnique(info);
}
VulkanController::VulkanController()
    : vertices_({{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
                 {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
                 {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
                 {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},
                 {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
                 {{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
                 {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
                 {{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}}),
      indices_({0, 1, 2, 2, 3, 0, 4, 5, 6, 6, 7, 4}){};
VulkanController::~VulkanController() {
  if (device_) {
    (*device_).waitIdle();
  }
  release();
}
void VulkanController::initialize(vk::UniqueInstance instance,
                                  vk::UniqueSurfaceKHR surface,
                                  const vk::Extent2D swapchain_extent) {
  instance_ = std::move(instance);
  surface_ = std::move(surface);
  swapchain_extent_ = swapchain_extent;

  const std::vector<vk::PhysicalDevice> devices =
      (*instance_).enumeratePhysicalDevices();
  physical_device_ = vka::select_physical_device(devices);

  const std::vector<vk::SurfaceFormatKHR> formats =
      physical_device_.getSurfaceFormatsKHR(*surface_);
  surface_format_ = vka::select_surface_format(formats);

  const std::vector<vk::QueueFamilyProperties> queue_family_properties =
      physical_device_.getQueueFamilyProperties();

  const std::vector<vk::Bool32> presentation_support =
      vka::get_presentation_support(physical_device_, *surface_,
                                    queue_family_properties.size());

  queue_index_ = vka::find_graphics_and_presentation_queue_family_index(
      queue_family_properties, presentation_support);

  device_ = vka::create_device(physical_device_, queue_index_);

  command_pool_ = vka::create_command_pool(*device_, queue_index_);

  texture_ = vka::Texture("texture.jpg");

  create_uniform_buffer();
  create_vertex_buffer();
  create_index_buffer();
  create_texture_image();

  texture_sampler_ = vka::create_texture_sampler(*device_);

  descriptor_pool_ = vka::create_descriptor_pool(*device_);
  descriptor_set_layout_ = vka::create_descriptor_set_layout(*device_);
  descriptor_sets_ = vka::create_descriptor_sets(*device_, *descriptor_pool_,
                                                 *descriptor_set_layout_);

  std::vector<vk::DescriptorSet> descriptor_set_pointers;
  for (const auto &descriptor_set : descriptor_sets_) {
    descriptor_set_pointers.push_back(*descriptor_set);
  }

  vka::update_descriptor_sets(*device_, descriptor_set_pointers,
                              *uniform_buffer_, *texture_image_view_,
                              *texture_sampler_);

  recreate_swapchain(swapchain_extent_);
}
void VulkanController::create_uniform_buffer() {
  const uint32_t ubo_size =
      static_cast<uint32_t>(sizeof(vka::UniformBufferObject));

  uniform_buffer_ = vka::create_buffer(*device_, ubo_size,
                                       vk::BufferUsageFlagBits::eUniformBuffer);

  uniform_buffer_memory_ = vka::allocate_buffer_memory(
      *device_, *uniform_buffer_, physical_device_.getMemoryProperties(),
      vk::MemoryPropertyFlagBits::eHostVisible |
          vk::MemoryPropertyFlagBits::eHostCoherent);

  (*device_).bindBufferMemory(*uniform_buffer_, *uniform_buffer_memory_, 0);

  vka::fill_buffer(*device_, *uniform_buffer_memory_,
                   vka::UniformBufferObject());
}
void VulkanController::update_uniform_buffer(const float delta_time) {
  vka::UniformBufferObject ubo;
  ubo.model = glm::rotate(glm::mat4(1.0f), delta_time * glm::radians(90.0f),
                          glm::vec3(0.0f, 0.0f, 1.0f));
  ubo.view =
      glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                  glm::vec3(0.0f, 0.0f, 1.0f));
  ubo.projection = glm::perspective(
      glm::radians(45.0f),
      swapchain_extent_.width / static_cast<float>(swapchain_extent_.height),
      0.1f, 10.0f);
  ubo.projection[1][1] *= -1;
  vka::fill_buffer(*device_, *uniform_buffer_memory_, ubo);
}
void VulkanController::create_vertex_buffer() {
  const uint32_t vertices_size =
      static_cast<uint32_t>(sizeof(vertices_[0]) * vertices_.size());

  vertex_buffer_ =
      vka::create_buffer(*device_, vertices_size,
                         vk::BufferUsageFlagBits::eVertexBuffer |
                             vk::BufferUsageFlagBits::eTransferDst);

  vertex_buffer_memory_ = vka::allocate_buffer_memory(
      *device_, *vertex_buffer_, physical_device_.getMemoryProperties(),
      vk::MemoryPropertyFlagBits::eDeviceLocal);

  (*device_).bindBufferMemory(*vertex_buffer_, *vertex_buffer_memory_, 0);

  vk::UniqueBuffer staging_buffer = vka::create_buffer(
      *device_, vertices_size, vk::BufferUsageFlagBits::eTransferSrc);

  vk::UniqueDeviceMemory staging_buffer_memory = vka::allocate_buffer_memory(
      *device_, *staging_buffer, physical_device_.getMemoryProperties(),
      vk::MemoryPropertyFlagBits::eHostVisible |
          vk::MemoryPropertyFlagBits::eHostCoherent);

  (*device_).bindBufferMemory(*staging_buffer, *staging_buffer_memory, 0);

  vka::fill_buffer(*device_, *staging_buffer_memory, vertices_);

  vka::copy_buffer_to_buffer(*device_, *staging_buffer, *vertex_buffer_,
                             vertices_size, *command_pool_, queue_index_);
}
void VulkanController::create_index_buffer() {
  const uint32_t indices_size =
      static_cast<uint32_t>(sizeof(indices_[0]) * indices_.size());

  index_buffer_ = vka::create_buffer(*device_, indices_size,
                                     vk::BufferUsageFlagBits::eIndexBuffer |
                                         vk::BufferUsageFlagBits::eTransferDst);

  index_buffer_memory_ = vka::allocate_buffer_memory(
      *device_, *index_buffer_, physical_device_.getMemoryProperties(),
      vk::MemoryPropertyFlagBits::eDeviceLocal);

  (*device_).bindBufferMemory(*index_buffer_, *index_buffer_memory_, 0);

  vk::UniqueBuffer staging_buffer = vka::create_buffer(
      *device_, indices_size, vk::BufferUsageFlagBits::eTransferSrc);

  vk::UniqueDeviceMemory staging_buffer_memory = vka::allocate_buffer_memory(
      *device_, *staging_buffer, physical_device_.getMemoryProperties(),
      vk::MemoryPropertyFlagBits::eHostVisible |
          vk::MemoryPropertyFlagBits::eHostCoherent);

  (*device_).bindBufferMemory(*staging_buffer, *staging_buffer_memory, 0);

  vka::fill_buffer(*device_, *staging_buffer_memory, indices_);

  vka::copy_buffer_to_buffer(*device_, *staging_buffer, *index_buffer_,
                             indices_size, *command_pool_, queue_index_);
}
void VulkanController::create_texture_image() {
  texture_image_ = vka::create_image(
      *device_, texture_.width, texture_.height, vk::Format::eR8G8B8A8Unorm,
      vk::ImageTiling::eOptimal,
      vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled);

  texture_image_memory_ = vka::allocate_image_memory(
      *device_, *texture_image_, physical_device_.getMemoryProperties(),
      vk::MemoryPropertyFlagBits::eDeviceLocal);

  (*device_).bindImageMemory(*texture_image_, *texture_image_memory_, 0);

  const uint32_t size = static_cast<uint32_t>(texture_.size);

  vk::UniqueBuffer staging_buffer =
      vka::create_buffer(*device_, size, vk::BufferUsageFlagBits::eTransferSrc);

  vk::UniqueDeviceMemory staging_buffer_memory = vka::allocate_buffer_memory(
      *device_, *staging_buffer, physical_device_.getMemoryProperties(),
      vk::MemoryPropertyFlagBits::eHostVisible |
          vk::MemoryPropertyFlagBits::eHostCoherent);

  (*device_).bindBufferMemory(*staging_buffer, *staging_buffer_memory, 0);

  vka::fill_buffer(*device_, *staging_buffer_memory, texture_);

  vka::transition_image_layout(
      *device_, *command_pool_, queue_index_, vk::ImageLayout::eUndefined,
      vk::ImageLayout::eTransferDstOptimal, *texture_image_);
  vka::copy_buffer_to_image(*device_, *staging_buffer, *texture_image_,
                            texture_.width, texture_.height, *command_pool_,
                            queue_index_);
  vka::transition_image_layout(*device_, *command_pool_, queue_index_,
                               vk::ImageLayout::eTransferDstOptimal,
                               vk::ImageLayout::eShaderReadOnlyOptimal,
                               *texture_image_);

  texture_image_view_ =
      vka::create_texture_image_view(*device_, *texture_image_);
}
void VulkanController::create_depth_image() {
  depth_image_ =
      vka::create_image(*device_, texture_.width, texture_.height,
                        vk::Format::eD32Sfloat, vk::ImageTiling::eOptimal,
                        vk::ImageUsageFlagBits::eDepthStencilAttachment);

  depth_image_memory_ = vka::allocate_image_memory(
      *device_, *depth_image_, physical_device_.getMemoryProperties(),
      vk::MemoryPropertyFlagBits::eDeviceLocal);

  (*device_).bindImageMemory(*depth_image_, *depth_image_memory_, 0);

  vka::transition_image_layout(
      *device_, *command_pool_, queue_index_, vk::ImageLayout::eUndefined,
      vk::ImageLayout::eDepthStencilAttachmentOptimal, *depth_image_);

  depth_image_view_ =
      vka::create_image_view(*device_, *depth_image_, vk::Format::eD32Sfloat,
                             vk::ImageAspectFlagBits::eDepth);
}
void VulkanController::recreate_swapchain(vk::Extent2D swapchain_extent) {
  swapchain_extent_ = swapchain_extent;

  const vk::SurfaceCapabilitiesKHR capabilities =
      physical_device_.getSurfaceCapabilitiesKHR(*surface_);
  swapchain_extent_ = vka::select_swapchain_extent(
      capabilities, swapchain_extent_.width, swapchain_extent_.height);

  swapchain_ =
      vka::create_swapchain(surface_format_, swapchain_extent_, capabilities,
                            *device_, *surface_, *swapchain_);

  std::vector<vk::Image> swapchain_images =
      (*device_).getSwapchainImagesKHR(*swapchain_);

  swapchain_image_views_ = vka::create_swapchain_image_views(
      *device_, swapchain_images, surface_format_);

  std::vector<vk::ImageView> swapchain_image_view_pointers;
  for (const auto &image_view : swapchain_image_views_) {
    swapchain_image_view_pointers.push_back(*image_view);
  }

  render_pass_ = vka::create_render_pass(*device_, surface_format_.format);

  std::vector<vk::DescriptorSet> descriptor_set_pointers;
  for (const auto &descriptor_set : descriptor_sets_) {
    descriptor_set_pointers.push_back(*descriptor_set);
  }

  pipeline_layout_ =
      vka::create_pipeline_layout(*device_, *descriptor_set_layout_);

  graphics_pipeline_ = vka::create_graphics_pipeline(
      *device_, *render_pass_, swapchain_extent_, *pipeline_layout_);

  create_depth_image();

  framebuffers_ = vka::create_framebuffers(
      *device_, *render_pass_, swapchain_extent_, swapchain_image_view_pointers,
      *depth_image_view_);

  std::vector<vk::Framebuffer> framebuffer_pointers;
  for (const auto &framebuffer : framebuffers_) {
    framebuffer_pointers.push_back(*framebuffer);
  }

  command_buffers_ = vka::create_command_buffers(
      *device_, *command_pool_, static_cast<uint32_t>(framebuffers_.size()));

  std::vector<vk::CommandBuffer> command_buffer_pointers;
  for (const auto &command_buffer : command_buffers_) {
    command_buffer_pointers.push_back(*command_buffer);
  }

  vka::record_command_buffers(
      *device_, command_buffer_pointers, *render_pass_, *graphics_pipeline_,
      *pipeline_layout_, framebuffer_pointers, swapchain_extent_,
      *vertex_buffer_, *index_buffer_, indices_, descriptor_set_pointers);
}
void VulkanController::update() {
  static auto start_time = std::chrono::high_resolution_clock::now();
  const auto current_time = std::chrono::high_resolution_clock::now();
  const float delta_time =
      vka::get_delta_time_per_second(start_time, current_time);
  update_uniform_buffer(delta_time);
}
void VulkanController::draw() {
  std::vector<vk::CommandBuffer> command_buffer_pointers;
  for (const auto &command_buffer : command_buffers_) {
    command_buffer_pointers.push_back(*command_buffer);
  }
  vka::draw_frame(*device_, *swapchain_, command_buffer_pointers, queue_index_);
}
void VulkanController::release_swapchain() {
  depth_image_view_.release();
  depth_image_.release();
  depth_image_memory_.release();

  for (auto &framebuffer : framebuffers_) {
    framebuffer.release();
  }

  for (auto &command_buffer : command_buffers_) {
    command_buffer.release();
  }

  graphics_pipeline_.release();
  pipeline_layout_.release();
  render_pass_.release();

  for (auto &image_view : swapchain_image_views_) {
    image_view.release();
  }

  swapchain_.release();
}
void VulkanController::release() {
  release_swapchain();
  texture_sampler_.release();
  texture_image_view_.release();
  texture_image_.release();
  texture_image_memory_.release();
  index_buffer_.release();
  index_buffer_memory_.release();
  vertex_buffer_.release();
  vertex_buffer_memory_.release();
  uniform_buffer_.release();
  uniform_buffer_memory_.release();
  for (auto &descriptor_set : descriptor_sets_) {
    descriptor_set.release();
  }
  descriptor_set_layout_.release();
  descriptor_pool_.release();
  command_pool_.release();
  device_.release();
  surface_.release();
  instance_.release();
}
void TriangleApplication::run() {
  const std::string application_name = "Triangle";
  int width = 500;
  int height = 500;

  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  window_ = glfwCreateWindow(width, height, application_name.c_str(), nullptr,
                             nullptr);
  glfwSetWindowUserPointer(window_, this);
  glfwSetWindowSizeCallback(window_, TriangleApplication::resize);

  std::vector<const char *> extension_names;
  uint32_t glfw_extension_count = 0;
  const char **glfw_extensions =
      glfwGetRequiredInstanceExtensions(&glfw_extension_count);
  for (uint32_t i = 0; i < glfw_extension_count; ++i) {
    extension_names.push_back(glfw_extensions[i]);
  }

  const vk::ApplicationInfo application_info =
      vka::create_application_info(application_name, {0, 1, 0});
  vk::UniqueInstance instance =
      vka::create_instance(application_info, extension_names);

  VkSurfaceKHR raw_surface;
  glfwCreateWindowSurface(*instance, window_, nullptr, &raw_surface);
  vk::UniqueSurfaceKHR surface(raw_surface);

  vulkan_controller_.initialize(std::move(instance), std::move(surface),
                                vk::Extent2D(width, height));

  while (!glfwWindowShouldClose(window_)) {
    glfwPollEvents();
    vulkan_controller_.update();
    vulkan_controller_.draw();
  }

  glfwDestroyWindow(window_);
  glfwTerminate();
}
void TriangleApplication::recreate_swapchain() {
  int width, height;
  glfwGetWindowSize(window_, &width, &height);
  vulkan_controller_.recreate_swapchain(vk::Extent2D(width, height));
}
void TriangleApplication::resize(GLFWwindow *window, int width, int height) {
  auto application =
      reinterpret_cast<TriangleApplication *>(glfwGetWindowUserPointer(window));
  application->recreate_swapchain();
}
}

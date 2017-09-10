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

namespace vka {
WindowManager::WindowManager(const std::string &name) : class_name_(name) {
  hInstance_ = GetModuleHandle(nullptr);
  register_window_class(hInstance_, class_name_.c_str());
  hWnd_ = create_window(hInstance_, class_name_.c_str());
}
WindowManager::~WindowManager() {
  DestroyWindow(hWnd_);
  UnregisterClass(class_name_.c_str(), hInstance_);
}
HINSTANCE WindowManager::hInstance() const { return hInstance_; }
HWND WindowManager::hWnd() const { return hWnd_; }
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  switch (uMsg) {
  case WM_CLOSE:
    DestroyWindow(hWnd);
    PostQuitMessage(0);
    break;
  default:
    break;
  }
  return (DefWindowProc(hWnd, uMsg, wParam, lParam));
}
ATOM WindowManager::register_window_class(HINSTANCE hInstance,
                                          const std::string &class_name) {
  WNDCLASSEX wcx;
  wcx.cbSize = sizeof(WNDCLASSEX);
  wcx.style = CS_HREDRAW | CS_VREDRAW;
  wcx.lpfnWndProc = WndProc;
  wcx.cbClsExtra = 0;
  wcx.cbWndExtra = 0;
  wcx.hInstance = hInstance;
  wcx.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
  wcx.hCursor = LoadCursor(nullptr, IDC_ARROW);
  wcx.hbrBackground = static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
  wcx.lpszMenuName = nullptr;
  wcx.lpszClassName = class_name.c_str();
  wcx.hIconSm = LoadIcon(nullptr, IDI_WINLOGO);
  return RegisterClassEx(&wcx);
}
HWND WindowManager::create_window(HINSTANCE hInstance,
                                  const std::string &class_name) {
  LONG window_width = 500;
  LONG window_height = 500;
  RECT window_rect = {0, 0, window_width, window_height};
  AdjustWindowRect(&window_rect, WS_OVERLAPPEDWINDOW, FALSE);
  return CreateWindowEx(0, class_name.c_str(), class_name.c_str(),
                        WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_SYSMENU, 0, 0,
                        window_rect.right - window_rect.left,
                        window_rect.bottom - window_rect.top, nullptr, nullptr,
                        hInstance, nullptr);
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
vk::UniqueInstance create_instance(const vk::ApplicationInfo application_info) {
  vk::InstanceCreateInfo info;
  info.pApplicationInfo = &application_info;
  std::vector<const char *> extension_names;
  extension_names.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
  extension_names.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
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

  return physical_device.createDeviceUnique(device_info);
}
vk::UniqueCommandPool create_command_pool(const vk::Device &device,
                                          const uint32_t queue_index) {
  vk::CommandPoolCreateInfo info;
  info.queueFamilyIndex = queue_index;
  return device.createCommandPoolUnique(info);
}
std::vector<vk::UniqueCommandBuffer>
create_command_buffers(const vk::Device &device,
                       const vk::CommandPool &command_pool) {
  vk::CommandBufferAllocateInfo info;
  info.commandPool = command_pool;
  info.commandBufferCount = 1;
  info.level = vk::CommandBufferLevel::ePrimary;
  return device.allocateCommandBuffersUnique(info);
}
vk::UniqueSurfaceKHR create_surface(const vk::Instance &instance,
                                    HINSTANCE hInstance, HWND hWnd) {
  vk::Win32SurfaceCreateInfoKHR info;
  info.hinstance = hInstance;
  info.hwnd = hWnd;
  return instance.createWin32SurfaceKHRUnique(info);
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
vk::UniqueSwapchainKHR
create_swapchain(const vk::SurfaceFormatKHR &surface_format,
                 const vk::Extent2D &extent,
                 const vk::SurfaceCapabilitiesKHR &capabilities,
                 const vk::Device &device, const vk::SurfaceKHR &surface) {
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
  info.oldSwapchain = nullptr;
  info.clipped = VK_TRUE;
  info.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
  return device.createSwapchainKHRUnique(info);
}
std::vector<vk::UniqueImageView>
create_swapchain_image_views(const vk::Device &device,
                             const std::vector<vk::Image> images,
                             const vk::SurfaceFormatKHR &surface_format) {
  std::vector<vk::UniqueImageView> image_views(images.size());
  for (size_t i = 0; i < images.size(); ++i) {
    vk::ImageViewCreateInfo info;
    info.image = images[i];
    info.viewType = vk::ImageViewType::e2D;
    info.format = surface_format.format;
    info.components.r = vk::ComponentSwizzle::eR;
    info.components.g = vk::ComponentSwizzle::eG;
    info.components.b = vk::ComponentSwizzle::eB;
    info.components.a = vk::ComponentSwizzle::eA;
    info.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    info.subresourceRange.baseMipLevel = 0;
    info.subresourceRange.levelCount = 1;
    info.subresourceRange.baseArrayLayer = 0;
    info.subresourceRange.layerCount = 1;
    image_views[i] = device.createImageViewUnique(info);
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
vk::UniquePipelineLayout create_pipeline_layout(const vk::Device &device) {
  vk::PipelineLayoutCreateInfo info;
  return device.createPipelineLayoutUnique(info);
}
vk::UniqueRenderPass create_render_pass(const vk::Device &device,
                                        const vk::Format &surface_format) {
  vk::RenderPassCreateInfo info;
  vk::AttachmentDescription color_attachment;
  color_attachment.format = surface_format;
  color_attachment.loadOp = vk::AttachmentLoadOp::eClear;
  color_attachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
  color_attachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
  color_attachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;
  info.attachmentCount = 1;
  info.pAttachments = &color_attachment;

  vk::AttachmentReference color_attachment_reference;
  color_attachment_reference.layout = vk::ImageLayout::eColorAttachmentOptimal;

  vk::SubpassDescription subpass;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &color_attachment_reference;
  info.subpassCount = 1;
  info.pSubpasses = &subpass;

  return device.createRenderPassUnique(info);
}
vk::UniquePipeline
create_graphics_pipeline(const vk::Device &device,
                         const vk::Extent2D &swapchain_extent,
                         const vk::Format &surface_format) {
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
  info.pVertexInputState = &vertex_input_state;

  vk::PipelineInputAssemblyStateCreateInfo input_assembly_state;
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
  rasterization_state.frontFace = vk::FrontFace::eClockwise;
  rasterization_state.depthBiasEnable = VK_FALSE;
  info.pRasterizationState = &rasterization_state;

  vk::PipelineMultisampleStateCreateInfo multisample_state;
  info.pMultisampleState = &multisample_state;

  vk::PipelineColorBlendAttachmentState color_blend_attachment_state;
  color_blend_attachment_state.colorWriteMask =
      vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
      vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
  color_blend_attachment_state.blendEnable = VK_FALSE;

  vk::PipelineColorBlendStateCreateInfo color_blend_state;
  color_blend_state.attachmentCount = 1;
  color_blend_state.pAttachments = &color_blend_attachment_state;
  info.pColorBlendState = &color_blend_state;

  vk::UniquePipelineLayout layout = create_pipeline_layout(device);
  info.layout = *layout;

  vk::UniqueRenderPass render_pass = create_render_pass(device, surface_format);
  info.renderPass = *render_pass;

  vk::UniquePipelineCache pipeline_cache;
  return device.createGraphicsPipelineUnique(*pipeline_cache, info);
}
}

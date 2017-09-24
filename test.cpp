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
#include "gtest/gtest.h"
#include <fstream>

class VulkanCache {
public:
  static const vk::Instance &instance() {
    if (!instance_) {
      vk::ApplicationInfo application_info =
          vka::create_application_info("Test", {1, 2, 3});
      instance_ = vka::create_instance(application_info);
    }
    return *instance_;
  }
  static const vk::Device &device() {
    if (!device_) {
      device_ = vka::create_device(physical_device(), queue_index());
    }
    return *device_;
  }
  static const uint32_t queue_index() {
    if (queue_index_ == UINT32_MAX) {
      std::vector<vk::Bool32> presentation_support =
          vka::get_presentation_support(physical_device(), surface(),
                                        queue_family_properties().size());
      queue_index_ = vka::find_graphics_and_presentation_queue_family_index(
          queue_family_properties(), presentation_support);
    }
    return queue_index_;
  }
  static const vk::CommandPool &command_pool() {
    if (!command_pool_) {
      command_pool_ = vka::create_command_pool(device(), queue_index());
    }
    return *command_pool_;
  }
  static const vk::SurfaceKHR &surface() {
    if (!surface_) {
      static vka::WindowManager manager("Application Name");
      surface_ = vka::create_surface(VulkanCache::instance(),
                                     manager.hInstance(), manager.hWnd());
      std::vector<vk::Bool32> presentation_support =
          vka::get_presentation_support(
              VulkanCache::physical_device(), *surface_,
              VulkanCache::queue_family_properties().size());
    }
    return *surface_;
  }
  static const vk::PhysicalDevice &physical_device() {
    if (!physical_device_) {
      const std::vector<vk::PhysicalDevice> devices =
          instance().enumeratePhysicalDevices();
      physical_device_ = vka::select_physical_device(devices);
    }
    return physical_device_;
  }
  static const std::vector<vk::QueueFamilyProperties> &
  queue_family_properties() {
    if (queue_family_properties_.empty()) {
      queue_family_properties_ = physical_device().getQueueFamilyProperties();
    }
    return queue_family_properties_;
  }
  static const vk::SurfaceFormatKHR surface_format() {
    const vk::SurfaceKHR surface = VulkanCache::surface();
    const std::vector<vk::SurfaceFormatKHR> formats =
        physical_device().getSurfaceFormatsKHR(surface);
    return vka::select_surface_format(formats);
  }
  static const vk::Extent2D swapchain_extent() {
    const vk::SurfaceKHR surface = VulkanCache::surface();
    const vk::SurfaceCapabilitiesKHR capabilities =
        physical_device().getSurfaceCapabilitiesKHR(surface);
    uint32_t width = 500;
    uint32_t height = 500;
    return vka::select_swapchain_extent(capabilities, width, height);
  }
  static const vk::SwapchainKHR &swapchain() {
    if (!swapchain_) {
      const vk::SurfaceKHR surface = VulkanCache::surface();
      const vk::SurfaceCapabilitiesKHR capabilities =
          physical_device().getSurfaceCapabilitiesKHR(surface);
      uint32_t width = 500;
      uint32_t height = 500;
      const vk::Extent2D extent =
          vka::select_swapchain_extent(capabilities, width, height);
      swapchain_ = vka::create_swapchain(surface_format(), extent, capabilities,
                                         device(), surface, nullptr);
    }
    return *swapchain_;
  }
  static std::vector<vk::Image> swapchain_images() {
    if (swapchain_images_.empty()) {
      swapchain_images_ = device().getSwapchainImagesKHR(swapchain());
    }
    return swapchain_images_;
  }
  static std::vector<vk::ImageView> swapchain_image_views() {
    if (swapchain_image_views_.empty()) {
      swapchain_image_views_ = vka::create_swapchain_image_views(
          device(), swapchain_images(), surface_format());
    }
    std::vector<vk::ImageView> image_views;
    for (const auto &image_view : swapchain_image_views_) {
      image_views.push_back(*image_view);
    }
    return image_views;
  }
  static const vk::RenderPass &render_pass() {
    if (!render_pass_) {
      render_pass_ = vka::create_render_pass(device(), surface_format().format);
    }
    return *render_pass_;
  }
  static const vk::Pipeline &graphics_pipeline() {
    if (!graphics_pipeline_) {
      graphics_pipeline_ = vka::create_graphics_pipeline(
          device(), render_pass(), swapchain_extent());
    }
    return *graphics_pipeline_;
  }
  static const std::vector<vk::Framebuffer> framebuffers() {
    if (framebuffers_.empty()) {
      framebuffers_ = vka::create_framebuffers(
          device(), render_pass(), swapchain_extent(), swapchain_image_views());
    }
    std::vector<vk::Framebuffer> framebuffers;
    for (const auto &framebuffer : framebuffers_) {
      framebuffers.push_back(*framebuffer);
    }
    return framebuffers;
  }
  static const std::vector<vk::CommandBuffer> command_buffers() {
    if (command_buffers_.empty()) {
      command_buffers_ = vka::create_command_buffers(
          device(), command_pool(),
          static_cast<uint32_t>(framebuffers().size()));
    }
    std::vector<vk::CommandBuffer> command_buffers;
    for (const auto &command_buffer : command_buffers_) {
      command_buffers.push_back(*command_buffer);
    }
    return command_buffers;
  }

private:
  static vk::UniqueInstance instance_;
  static uint32_t queue_index_;
  static vk::UniqueDevice device_;
  static vk::UniqueCommandPool command_pool_;
  static vk::UniqueSurfaceKHR surface_;
  static vk::PhysicalDevice physical_device_;
  static std::vector<vk::QueueFamilyProperties> queue_family_properties_;
  static vk::UniqueSwapchainKHR swapchain_;
  static std::vector<vk::Image> swapchain_images_;
  static std::vector<vk::UniqueImageView> swapchain_image_views_;
  static vk::UniqueRenderPass render_pass_;
  static vk::UniquePipeline graphics_pipeline_;
  static std::vector<vk::UniqueFramebuffer> framebuffers_;
  static std::vector<vk::UniqueCommandBuffer> command_buffers_;
};

vk::UniqueInstance VulkanCache::instance_ = vk::UniqueInstance();
vk::UniqueDevice VulkanCache::device_ = vk::UniqueDevice();
uint32_t VulkanCache::queue_index_ = UINT32_MAX;
vk::UniqueCommandPool VulkanCache::command_pool_ = vk::UniqueCommandPool();
vk::UniqueSurfaceKHR VulkanCache::surface_ = vk::UniqueSurfaceKHR();
vk::PhysicalDevice VulkanCache::physical_device_ = vk::PhysicalDevice();
std::vector<vk::QueueFamilyProperties> VulkanCache::queue_family_properties_ =
    std::vector<vk::QueueFamilyProperties>();
vk::UniqueSwapchainKHR VulkanCache::swapchain_ = vk::UniqueSwapchainKHR();
std::vector<vk::Image> VulkanCache::swapchain_images_ =
    std::vector<vk::Image>();
std::vector<vk::UniqueImageView> VulkanCache::swapchain_image_views_ =
    std::vector<vk::UniqueImageView>();
vk::UniqueRenderPass VulkanCache::render_pass_ = vk::UniqueRenderPass();
vk::UniquePipeline VulkanCache::graphics_pipeline_ = vk::UniquePipeline();
std::vector<vk::UniqueFramebuffer> VulkanCache::framebuffers_ =
    std::vector<vk::UniqueFramebuffer>();
std::vector<vk::UniqueCommandBuffer> VulkanCache::command_buffers_ =
    std::vector<vk::UniqueCommandBuffer>();

TEST(WindowManager,
     ReturnsNonNullHInstanceGivenModuleHandleIsSuccessfullyRetrieved) {
  const std::string name = "Application Name";
  vka::WindowManager manager(name);
  EXPECT_TRUE(manager.hInstance());
}

TEST(WindowManager, ReturnsNonNullHWndGivenWindowIsSuccessfullyCreated) {
  const std::string name = "Application Name";
  vka::WindowManager manager(name);
  EXPECT_TRUE(manager.hWnd());
}

TEST(TriangleExample, CreatesInstanceWithoutThrowingException) {
  vk::ApplicationInfo application_info =
      vka::create_application_info("Test", {1, 2, 3});
  EXPECT_NO_THROW(vka::create_instance(application_info));
}

TEST(TriangleExample, SelectsNonEmptyPhysicalDeviceIfAnyIsAvailable) {
  std::vector<vk::PhysicalDevice> devices =
      VulkanCache::instance().enumeratePhysicalDevices();
  EXPECT_NE(vka::select_physical_device(devices), vk::PhysicalDevice());
}

TEST(TriangleExample, SelectsEmptyPhysicalDeviceIfNoneIsAvailable) {
  std::vector<vk::PhysicalDevice> devices;
  EXPECT_EQ(vka::select_physical_device(devices), vk::PhysicalDevice());
}

TEST(TriangleExample, FindsGraphicsQueueFamilyIndexGivenItExists) {
  vk::QueueFamilyProperties compute_queue;
  compute_queue.queueFlags = vk::QueueFlagBits::eCompute;

  vk::QueueFamilyProperties graphics_queue;
  graphics_queue.queueFlags = vk::QueueFlagBits::eGraphics;

  std::vector<vk::QueueFamilyProperties> queue_family_properties = {
      compute_queue, graphics_queue, compute_queue};

  EXPECT_EQ(vka::find_graphics_queue_family_index(queue_family_properties), 1);
}

TEST(TriangleExample,
     ReturnsUINT32MaxValueGivenGraphicsQueueFamilyDoesNotExist) {
  std::vector<vk::QueueFamilyProperties> queue_family_properties;
  EXPECT_EQ(vka::find_graphics_queue_family_index(queue_family_properties),
            UINT32_MAX);
}

TEST(TriangleExample, CreatesLogicalDeviceWithoutThrowingException) {
  const std::vector<vk::PhysicalDevice> devices =
      VulkanCache::instance().enumeratePhysicalDevices();
  const vk::PhysicalDevice physical_device =
      vka::select_physical_device(devices);
  const std::vector<vk::QueueFamilyProperties> queues =
      physical_device.getQueueFamilyProperties();
  const uint32_t queue_index = 0;
  EXPECT_NO_THROW(vka::create_device(physical_device, queue_index));
}

TEST(TriangleExample, CreatesCommandPoolWithoutThrowingException) {
  EXPECT_NO_THROW(vka::create_command_pool(VulkanCache::device(),
                                           VulkanCache::queue_index()));
}

TEST(TriangleExample, CreatesCommandBuffersWithoutThrowingException) {
  const uint32_t command_buffer_count = 3;
  EXPECT_NO_THROW(vka::create_command_buffers(VulkanCache::device(),
                                              VulkanCache::command_pool(), 3));
}

TEST(TriangleExample, CreatesSurfaceWithoutThrowingException) {
  vka::WindowManager manager("Application Name");
  EXPECT_NO_THROW(vka::create_surface(VulkanCache::instance(),
                                      manager.hInstance(), manager.hWnd()));
}

TEST(TriangleExample,
     ReturnsVectorWithPresentationSupportForEachAvailableQueueFamily) {
  std::vector<vk::Bool32> presentation_support = vka::get_presentation_support(
      VulkanCache::physical_device(), VulkanCache::surface(),
      VulkanCache::queue_family_properties().size());
  EXPECT_EQ(presentation_support.size(),
            VulkanCache::queue_family_properties().size());
}

TEST(TriangleExample,
     FindsQueueFamilyIndexWithGraphicsAndPresentationSupportGivenItExists) {
  vk::QueueFamilyProperties compute_queue;
  compute_queue.queueFlags = vk::QueueFlagBits::eCompute;

  vk::QueueFamilyProperties graphics_queue;
  graphics_queue.queueFlags = vk::QueueFlagBits::eGraphics;

  std::vector<vk::QueueFamilyProperties> queue_family_properties = {
      compute_queue, compute_queue, graphics_queue, graphics_queue};
  std::vector<vk::Bool32> presentation_support = {VK_FALSE, VK_TRUE, VK_FALSE,
                                                  VK_TRUE};

  EXPECT_EQ(vka::find_graphics_and_presentation_queue_family_index(
                queue_family_properties, presentation_support),
            3);
}

TEST(
    TriangleExample,
    ReturnsUINT32MaxValueGivenGraphicsQueueFamilyDoesNotExistAndQueueSupportsPresentation) {
  vk::QueueFamilyProperties compute_queue;
  compute_queue.queueFlags = vk::QueueFlagBits::eCompute;

  std::vector<vk::QueueFamilyProperties> queue_family_properties = {
      compute_queue};
  std::vector<vk::Bool32> presentation_support = {VK_TRUE};

  EXPECT_EQ(vka::find_graphics_and_presentation_queue_family_index(
                queue_family_properties, presentation_support),
            UINT32_MAX);
}

TEST(
    TriangleExample,
    ReturnsUINT32MaxValueGivenGraphicsQueueFamilyExistsAndQueueDoesNotSupportPresentation) {
  vk::QueueFamilyProperties graphics_queue;
  graphics_queue.queueFlags = vk::QueueFlagBits::eGraphics;

  std::vector<vk::QueueFamilyProperties> queue_family_properties = {
      graphics_queue};
  std::vector<vk::Bool32> presentation_support = {VK_FALSE};

  EXPECT_EQ(vka::find_graphics_and_presentation_queue_family_index(
                queue_family_properties, presentation_support),
            UINT32_MAX);
}

TEST(
    TriangleExample,
    ReturnsUINT32MaxValueGivenGraphicsQueueFamilyDoesNotExistAndQueueDoesNotSupportPresentation) {
  vk::QueueFamilyProperties compute_queue;
  compute_queue.queueFlags = vk::QueueFlagBits::eCompute;

  std::vector<vk::QueueFamilyProperties> queue_family_properties = {
      compute_queue};
  std::vector<vk::Bool32> presentation_support = {VK_FALSE};

  EXPECT_EQ(vka::find_graphics_and_presentation_queue_family_index(
                queue_family_properties, presentation_support),
            UINT32_MAX);
}

TEST(TriangleExample, ReturnsUINT32MaxValueGivenInputVectorsHaveDifferentSize) {
  vk::QueueFamilyProperties graphics_queue;
  graphics_queue.queueFlags = vk::QueueFlagBits::eGraphics;

  std::vector<vk::QueueFamilyProperties> queue_family_properties = {
      graphics_queue};
  std::vector<vk::Bool32> presentation_support = {VK_TRUE, VK_FALSE};

  EXPECT_EQ(vka::find_graphics_and_presentation_queue_family_index(
                queue_family_properties, presentation_support),
            UINT32_MAX);
}

TEST(
    TriangleExample,
    SelectsB8G8R8A8UnormColorFormatAndSRGBNonlinearColorSpaceGivenThereAreNoPreferedFormat) {
  std::vector<vk::SurfaceFormatKHR> formats = {
      {vk::Format::eUndefined, vk::ColorSpaceKHR::eAdobergbLinearEXT}};
  vk::SurfaceFormatKHR expected_format = {vk::Format::eB8G8R8A8Unorm,
                                          vk::ColorSpaceKHR::eSrgbNonlinear};
  EXPECT_EQ(vka::select_surface_format(formats), expected_format);
}

TEST(TriangleExample, SelectsFirstSurfaceFormatGivenThereArePreferedFormats) {
  std::vector<vk::SurfaceFormatKHR> formats = {
      {vk::Format::eA1R5G5B5UnormPack16, vk::ColorSpaceKHR::eAdobergbLinearEXT},
      {vk::Format::eAstc10x5SrgbBlock, vk::ColorSpaceKHR::eBt709LinearEXT}};
  EXPECT_EQ(vka::select_surface_format(formats), formats[0]);
}

TEST(TriangleExample,
     ReturnsSwapchainExtentEqualToSurfaceCapabilitiesGivenCurrentExtentIsSet) {
  vk::SurfaceCapabilitiesKHR capabilities = {};
  capabilities.currentExtent = vk::Extent2D(1, 2);
  uint32_t width = 0;
  uint32_t height = 0;
  EXPECT_EQ(vka::select_swapchain_extent(capabilities, width, height),
            capabilities.currentExtent);
}

TEST(TriangleExample, SetsWidthAndHeightGivenCurrentExtentIsSet) {
  vk::SurfaceCapabilitiesKHR capabilities = {};
  capabilities.currentExtent = vk::Extent2D(1, 2);
  uint32_t width = 0;
  uint32_t height = 0;
  vka::select_swapchain_extent(capabilities, width, height);
  EXPECT_EQ(vk::Extent2D(width, height), capabilities.currentExtent);
}

TEST(TriangleExample,
     ReturnsSwapchainExtentEqualToWidthAndHeightGivenCurrentExtentIsUndefined) {
  vk::SurfaceCapabilitiesKHR capabilities = {};
  capabilities.currentExtent = vk::Extent2D(UINT32_MAX, UINT32_MAX);
  uint32_t width = 1;
  uint32_t height = 2;
  EXPECT_EQ(vka::select_swapchain_extent(capabilities, width, height),
            vk::Extent2D(width, height));
}

TEST(TriangleExample,
     LeavesWidthAndHeightUnchangedGivenCurrentExtentIsUndefined) {
  vk::SurfaceCapabilitiesKHR capabilities = {};
  capabilities.currentExtent = vk::Extent2D(UINT32_MAX, UINT32_MAX);
  uint32_t width = 1;
  uint32_t old_width = width;
  uint32_t height = 2;
  uint32_t old_height = height;
  vka::select_swapchain_extent(capabilities, width, height);
  EXPECT_EQ(vk::Extent2D(width, height), vk::Extent2D(old_width, old_height));
}

TEST(TriangleExample, CreatesSwapchainWithoutThrowingException) {
  vka::WindowManager manager("Application Name");
  const vk::UniqueSurfaceKHR surface = vka::create_surface(
      VulkanCache::instance(), manager.hInstance(), manager.hWnd());
  std::vector<vk::Bool32> presentation_support = vka::get_presentation_support(
      VulkanCache::physical_device(), *surface,
      VulkanCache::queue_family_properties().size());
  const vk::SurfaceCapabilitiesKHR capabilities =
      VulkanCache::physical_device().getSurfaceCapabilitiesKHR(*surface);
  const std::vector<vk::SurfaceFormatKHR> formats =
      VulkanCache::physical_device().getSurfaceFormatsKHR(*surface);
  const vk::SurfaceFormatKHR format = vka::select_surface_format(formats);
  uint32_t width = 500;
  uint32_t height = 500;
  const vk::Extent2D extent =
      vka::select_swapchain_extent(capabilities, width, height);
  vk::SwapchainKHR old_swapchain = nullptr;
  EXPECT_NO_THROW(vka::create_swapchain(format, extent, capabilities,
                                        VulkanCache::device(), *surface,
                                        old_swapchain));
}

TEST(TriangleExample, CreatesSwapchainImageViewsWithoutThrowingException) {
  EXPECT_NO_THROW(vka::create_swapchain_image_views(
      VulkanCache::device(), VulkanCache::swapchain_images(),
      VulkanCache::surface_format()));
}

TEST(TriangleExample, ReturnsVectorOfBytesGivenFileExists) {
  const std::string file_name = "file.txt";
  const std::string content = "Content";
  std::ofstream f;
  f.open(file_name);
  f << content;
  f.close();
  EXPECT_EQ(vka::read_file(file_name).size(), content.size());
}

TEST(TriangleExample, ReturnsVectorWithSizeEqualToZeroGivenFileDoesExist) {
  EXPECT_EQ(vka::read_file("unknown").size(), 0);
}

TEST(TriangleExample, CreatesShaderModuleWithoutThrowingException) {
  const std::vector<char> code = vka::read_file("vert.spv");
  EXPECT_NO_THROW(vka::create_shader_module(VulkanCache::device(), code));
}

TEST(TriangleExample, CreatesPipelineLayoutWithoutThrowingException) {
  EXPECT_NO_THROW(vka::create_pipeline_layout(VulkanCache::device()));
}

TEST(TriangleExample, CreatesRenderPassWithoutThrowingException) {
  EXPECT_NO_THROW(vka::create_render_pass(VulkanCache::device(),
                                          vk::Format::eB8G8R8A8Unorm));
}

TEST(TriangleExample, CreatesGraphicsPipelineWithoutThrowingException) {
  EXPECT_NO_THROW(vka::create_graphics_pipeline(
      VulkanCache::device(), VulkanCache::render_pass(),
      VulkanCache::swapchain_extent()));
}

TEST(TriangleExample, CreatesFramebuffersWithoutThrowingException) {
  EXPECT_NO_THROW(vka::create_framebuffers(
      VulkanCache::device(), VulkanCache::render_pass(),
      VulkanCache::swapchain_extent(), VulkanCache::swapchain_image_views()));
}

TEST(TriangleExample, RecordsCommandBuffersWithoutThrowingException) {
  EXPECT_NO_THROW(vka::record_command_buffers(
      VulkanCache::device(), VulkanCache::command_buffers(),
      VulkanCache::render_pass(), VulkanCache::graphics_pipeline(),
      VulkanCache::framebuffers(), VulkanCache::swapchain_extent()));
}

TEST(TriangleExample, DrawsFrameWithoutThrowingException) {
  std::vector<vk::UniqueCommandBuffer> command_buffers =
      vka::create_command_buffers(
          VulkanCache::device(), VulkanCache::command_pool(),
          static_cast<uint32_t>(VulkanCache::framebuffers().size()));
  std::vector<vk::CommandBuffer> command_buffer_pointers;
  for (const auto &command_buffer : command_buffers) {
    command_buffer_pointers.push_back(*command_buffer);
  }
  vka::record_command_buffers(
      VulkanCache::device(), command_buffer_pointers,
      VulkanCache::render_pass(), VulkanCache::graphics_pipeline(),
      VulkanCache::framebuffers(), VulkanCache::swapchain_extent());
  EXPECT_NO_THROW(
      vka::draw_frame(VulkanCache::device(), VulkanCache::swapchain(),
                      command_buffer_pointers, VulkanCache::queue_index()));
}
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

class WindowManager {
public:
  WindowManager() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window_ = glfwCreateWindow(500, 500, "Test", nullptr, nullptr);
  }
  ~WindowManager() { glfwDestroyWindow(window_); }
  std::vector<const char *> extension_names() {
    std::vector<const char *> extension_names;
    uint32_t glfw_extension_count = 0;
    const char **glfw_extensions =
        glfwGetRequiredInstanceExtensions(&glfw_extension_count);
    for (uint32_t i = 0; i < glfw_extension_count; ++i) {
      extension_names.push_back(glfw_extensions[i]);
    }
    return extension_names;
  }
  VkSurfaceKHR surface(VkInstance instance) {
    VkSurfaceKHR surface;
    glfwCreateWindowSurface(instance, window_, nullptr, &surface);
    return surface;
  }

private:
  GLFWwindow *window_;
};

class TriangleTest : public ::testing::Test {
protected:
  static void TearDownTestCase() { release(); }
  static const vk::Instance &instance() {
    if (!instance_) {
      vk::ApplicationInfo application_info =
          vka::create_application_info("Test", {1, 2, 3});
      instance_ = vka::create_instance(application_info,
                                       window_manager_.extension_names());
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
  static const vk::Buffer &vertex_buffer() {
    if (!vertex_buffer_) {
      const uint32_t size =
          static_cast<uint32_t>(sizeof(vertices()[0]) * vertices().size());
      vertex_buffer_ = vka::create_buffer(
          device(), size, vk::BufferUsageFlagBits::eVertexBuffer);
    }
    return *vertex_buffer_;
  }
  static const vk::DeviceMemory &vertex_buffer_memory() {
    if (!vertex_buffer_memory_) {
      vertex_buffer_memory_ = vka::allocate_buffer_memory(
          device(), vertex_buffer(), physical_device().getMemoryProperties(),
          vk::MemoryPropertyFlagBits::eHostVisible |
              vk::MemoryPropertyFlagBits::eHostCoherent);
    }
    device().bindBufferMemory(vertex_buffer(), *vertex_buffer_memory_, 0);
    return *vertex_buffer_memory_;
  }
  static const vk::SurfaceKHR &surface() {
    if (!surface_) {
      surface_ = vk::UniqueSurfaceKHR(window_manager_.surface(instance()));
      std::vector<vk::Bool32> presentation_support =
          vka::get_presentation_support(physical_device(), *surface_,
                                        queue_family_properties().size());
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
    const vk::SurfaceKHR s = surface();
    const std::vector<vk::SurfaceFormatKHR> formats =
        physical_device().getSurfaceFormatsKHR(s);
    return vka::select_surface_format(formats);
  }
  static const vk::Extent2D swapchain_extent() {
    const vk::SurfaceKHR s = surface();
    const vk::SurfaceCapabilitiesKHR capabilities =
        physical_device().getSurfaceCapabilitiesKHR(s);
    uint32_t width = 500;
    uint32_t height = 500;
    return vka::select_swapchain_extent(capabilities, width, height);
  }
  static const vk::SwapchainKHR &swapchain() {
    if (!swapchain_) {
      const vk::SurfaceKHR s = surface();
      const vk::SurfaceCapabilitiesKHR capabilities =
          physical_device().getSurfaceCapabilitiesKHR(s);
      uint32_t width = 500;
      uint32_t height = 500;
      const vk::Extent2D extent =
          vka::select_swapchain_extent(capabilities, width, height);
      swapchain_ = vka::create_swapchain(surface_format(), extent, capabilities,
                                         device(), s, nullptr);
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
  static const std::vector<vka::Vertex> vertices() { return vertices_; }
  static void release() {
    for (auto &framebuffer : framebuffers_) {
      framebuffer.release();
    }

    for (auto &command_buffer : command_buffers_) {
      command_buffer.release();
    }

    graphics_pipeline_.release();
    render_pass_.release();

    for (auto &image_view : swapchain_image_views_) {
      image_view.release();
    }

    swapchain_.release();

    vertex_buffer_.release();
    vertex_buffer_memory_.release();
    command_pool_.release();
    device_.release();
    surface_.release();
    instance_.release();
    surface_.release();
    instance_.release();
  }

private:
  static vk::UniqueInstance instance_;
  static uint32_t queue_index_;
  static vk::UniqueDevice device_;
  static vk::UniqueCommandPool command_pool_;
  static vk::UniqueBuffer vertex_buffer_;
  static vk::UniqueDeviceMemory vertex_buffer_memory_;
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
  static WindowManager window_manager_;
  static const std::vector<vka::Vertex> vertices_;
};

vk::UniqueInstance TriangleTest::instance_ = vk::UniqueInstance();
vk::UniqueDevice TriangleTest::device_ = vk::UniqueDevice();
uint32_t TriangleTest::queue_index_ = UINT32_MAX;
vk::UniqueCommandPool TriangleTest::command_pool_ = vk::UniqueCommandPool();
vk::UniqueBuffer TriangleTest::vertex_buffer_ = vk::UniqueBuffer();
vk::UniqueDeviceMemory TriangleTest::vertex_buffer_memory_ =
    vk::UniqueDeviceMemory();
vk::UniqueSurfaceKHR TriangleTest::surface_ = vk::UniqueSurfaceKHR();
vk::PhysicalDevice TriangleTest::physical_device_ = vk::PhysicalDevice();
std::vector<vk::QueueFamilyProperties> TriangleTest::queue_family_properties_ =
    std::vector<vk::QueueFamilyProperties>();
vk::UniqueSwapchainKHR TriangleTest::swapchain_ = vk::UniqueSwapchainKHR();
std::vector<vk::Image> TriangleTest::swapchain_images_ =
    std::vector<vk::Image>();
std::vector<vk::UniqueImageView> TriangleTest::swapchain_image_views_ =
    std::vector<vk::UniqueImageView>();
vk::UniqueRenderPass TriangleTest::render_pass_ = vk::UniqueRenderPass();
vk::UniquePipeline TriangleTest::graphics_pipeline_ = vk::UniquePipeline();
std::vector<vk::UniqueFramebuffer> TriangleTest::framebuffers_ =
    std::vector<vk::UniqueFramebuffer>();
std::vector<vk::UniqueCommandBuffer> TriangleTest::command_buffers_ =
    std::vector<vk::UniqueCommandBuffer>();
WindowManager TriangleTest::window_manager_ = WindowManager();
const std::vector<vka::Vertex> TriangleTest::vertices_ = {
    {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
    {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}};

TEST_F(TriangleTest, CreatesInstanceWithoutThrowingException) {
  vk::ApplicationInfo application_info =
      vka::create_application_info("Test", {1, 2, 3});
  std::vector<const char *> required_extensions_names = {
      VK_KHR_SURFACE_EXTENSION_NAME};
  EXPECT_NO_THROW(
      vka::create_instance(application_info, required_extensions_names));
}

TEST_F(TriangleTest, SelectsNonEmptyPhysicalDeviceIfAnyIsAvailable) {
  std::vector<vk::PhysicalDevice> devices =
      instance().enumeratePhysicalDevices();
  EXPECT_NE(vka::select_physical_device(devices), vk::PhysicalDevice());
}

TEST_F(TriangleTest, SelectsEmptyPhysicalDeviceIfNoneIsAvailable) {
  std::vector<vk::PhysicalDevice> devices;
  EXPECT_EQ(vka::select_physical_device(devices), vk::PhysicalDevice());
}

TEST_F(TriangleTest, FindsGraphicsQueueFamilyIndexGivenItExists) {
  vk::QueueFamilyProperties compute_queue;
  compute_queue.queueFlags = vk::QueueFlagBits::eCompute;

  vk::QueueFamilyProperties graphics_queue;
  graphics_queue.queueFlags = vk::QueueFlagBits::eGraphics;

  std::vector<vk::QueueFamilyProperties> queue_family_properties = {
      compute_queue, graphics_queue, compute_queue};

  EXPECT_EQ(vka::find_graphics_queue_family_index(queue_family_properties), 1);
}

TEST_F(TriangleTest,
       ReturnsUINT32MaxValueGivenGraphicsQueueFamilyDoesNotExist) {
  std::vector<vk::QueueFamilyProperties> queue_family_properties;
  EXPECT_EQ(vka::find_graphics_queue_family_index(queue_family_properties),
            UINT32_MAX);
}

TEST_F(TriangleTest, CreatesLogicalDeviceWithoutThrowingException) {
  const std::vector<vk::PhysicalDevice> devices =
      instance().enumeratePhysicalDevices();
  const vk::PhysicalDevice physical_device =
      vka::select_physical_device(devices);
  const std::vector<vk::QueueFamilyProperties> queues =
      physical_device.getQueueFamilyProperties();
  const uint32_t queue_index = 0;
  EXPECT_NO_THROW(vka::create_device(physical_device, queue_index));
}

TEST_F(TriangleTest, CreatesCommandPoolWithoutThrowingException) {
  EXPECT_NO_THROW(vka::create_command_pool(device(), queue_index()));
}

TEST_F(TriangleTest, CreatesVertexBufferWithoutThrowingException) {
  const uint32_t size =
      static_cast<uint32_t>(sizeof(vertices()[0]) * vertices().size());
  EXPECT_NO_THROW(vka::create_buffer(device(), size,
                                     vk::BufferUsageFlagBits::eVertexBuffer));
}

TEST_F(TriangleTest, ReturnsUINT32MaxValueGivenThereAreNoMemoryTypes) {
  vk::MemoryType memory_type;
  memory_type.propertyFlags = vk::MemoryPropertyFlagBits::eHostVisible;
  vk::PhysicalDeviceMemoryProperties physical_device_memory_properties;
  physical_device_memory_properties.memoryTypeCount = 0;
  physical_device_memory_properties.memoryTypes[0] = memory_type;
  const uint32_t required_memory_type = UINT32_MAX;
  const vk::MemoryPropertyFlags required_memory_properties =
      memory_type.propertyFlags;
  EXPECT_EQ(vka::find_memory_type(physical_device_memory_properties,
                                  required_memory_type,
                                  required_memory_properties),
            UINT32_MAX);
}

TEST_F(TriangleTest,
       ReturnsUINT32MaxValueGivenMemoryTypeWithRequiredMemoryTypeDoesNotExist) {
  vk::MemoryType memory_type;
  memory_type.propertyFlags = vk::MemoryPropertyFlagBits::eHostVisible;
  vk::PhysicalDeviceMemoryProperties physical_device_memory_properties;
  physical_device_memory_properties.memoryTypeCount = 1;
  physical_device_memory_properties.memoryTypes[0] = memory_type;
  const uint32_t required_memory_type = 0;
  const vk::MemoryPropertyFlags required_memory_properties =
      memory_type.propertyFlags;
  EXPECT_EQ(vka::find_memory_type(physical_device_memory_properties,
                                  required_memory_type,
                                  required_memory_properties),
            UINT32_MAX);
}

TEST_F(
    TriangleTest,
    ReturnsUINT32MaxValueGivenMemoryTypeWithRequiredMemoryPropertiesDoNotExist) {
  vk::MemoryType memory_type;
  memory_type.propertyFlags = vk::MemoryPropertyFlags();
  vk::PhysicalDeviceMemoryProperties physical_device_memory_properties;
  physical_device_memory_properties.memoryTypeCount = 1;
  physical_device_memory_properties.memoryTypes[0] = memory_type;
  const uint32_t required_memory_type = UINT32_MAX;
  const vk::MemoryPropertyFlags required_memory_properties =
      vk::MemoryPropertyFlagBits::eHostVisible;
  EXPECT_EQ(vka::find_memory_type(physical_device_memory_properties,
                                  required_memory_type,
                                  required_memory_properties),
            UINT32_MAX);
}

TEST_F(TriangleTest, ReturnsMemoryTypeIndexGivenItExist) {
  vk::MemoryType memory_type;
  memory_type.propertyFlags = vk::MemoryPropertyFlagBits::eHostVisible;
  vk::PhysicalDeviceMemoryProperties physical_device_memory_properties;
  physical_device_memory_properties.memoryTypeCount = 1;
  physical_device_memory_properties.memoryTypes[0] = memory_type;
  const uint32_t required_memory_type = UINT32_MAX;
  const vk::MemoryPropertyFlags required_memory_properties =
      memory_type.propertyFlags;
  EXPECT_EQ(vka::find_memory_type(physical_device_memory_properties,
                                  required_memory_type,
                                  required_memory_properties),
            0);
}

TEST_F(TriangleTest, AllocatesMemoryForVertexBufferWithoutThrowingException) {
  EXPECT_NO_THROW(vka::allocate_buffer_memory(
      device(), vertex_buffer(), physical_device().getMemoryProperties(),
      vk::MemoryPropertyFlagBits::eHostVisible |
          vk::MemoryPropertyFlagBits::eHostCoherent));
}

TEST_F(TriangleTest, FillsVertexBufferWithoutThrowingException) {
  EXPECT_NO_THROW(
      vka::fill_vertex_buffer(device(), vertex_buffer_memory(), vertices()));
}

TEST_F(TriangleTest, CreatesCommandBuffersWithoutThrowingException) {
  const uint32_t command_buffer_count = 3;
  EXPECT_NO_THROW(vka::create_command_buffers(device(), command_pool(), 3));
}

TEST_F(TriangleTest,
       ReturnsVectorWithPresentationSupportForEachAvailableQueueFamily) {
  std::vector<vk::Bool32> presentation_support = vka::get_presentation_support(
      physical_device(), surface(), queue_family_properties().size());
  EXPECT_EQ(presentation_support.size(), queue_family_properties().size());
}

TEST_F(TriangleTest,
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

TEST_F(
    TriangleTest,
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

TEST_F(
    TriangleTest,
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

TEST_F(
    TriangleTest,
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

TEST_F(TriangleTest, ReturnsUINT32MaxValueGivenInputVectorsHaveDifferentSize) {
  vk::QueueFamilyProperties graphics_queue;
  graphics_queue.queueFlags = vk::QueueFlagBits::eGraphics;

  std::vector<vk::QueueFamilyProperties> queue_family_properties = {
      graphics_queue};
  std::vector<vk::Bool32> presentation_support = {VK_TRUE, VK_FALSE};

  EXPECT_EQ(vka::find_graphics_and_presentation_queue_family_index(
                queue_family_properties, presentation_support),
            UINT32_MAX);
}

TEST_F(
    TriangleTest,
    SelectsB8G8R8A8UnormColorFormatAndSRGBNonlinearColorSpaceGivenThereAreNoPreferedFormat) {
  std::vector<vk::SurfaceFormatKHR> formats = {
      {vk::Format::eUndefined, vk::ColorSpaceKHR::eAdobergbLinearEXT}};
  vk::SurfaceFormatKHR expected_format = {vk::Format::eB8G8R8A8Unorm,
                                          vk::ColorSpaceKHR::eSrgbNonlinear};
  EXPECT_EQ(vka::select_surface_format(formats), expected_format);
}

TEST_F(TriangleTest, SelectsFirstSurfaceFormatGivenThereArePreferedFormats) {
  std::vector<vk::SurfaceFormatKHR> formats = {
      {vk::Format::eA1R5G5B5UnormPack16, vk::ColorSpaceKHR::eAdobergbLinearEXT},
      {vk::Format::eAstc10x5SrgbBlock, vk::ColorSpaceKHR::eBt709LinearEXT}};
  EXPECT_EQ(vka::select_surface_format(formats), formats[0]);
}

TEST_F(
    TriangleTest,
    ReturnsSwapchainExtentEqualToSurfaceCapabilitiesGivenCurrentExtentIsSet) {
  vk::SurfaceCapabilitiesKHR capabilities = {};
  capabilities.currentExtent = vk::Extent2D(1, 2);
  uint32_t width = 0;
  uint32_t height = 0;
  EXPECT_EQ(vka::select_swapchain_extent(capabilities, width, height),
            capabilities.currentExtent);
}

TEST_F(TriangleTest, SetsWidthAndHeightGivenCurrentExtentIsSet) {
  vk::SurfaceCapabilitiesKHR capabilities = {};
  capabilities.currentExtent = vk::Extent2D(1, 2);
  uint32_t width = 0;
  uint32_t height = 0;
  vka::select_swapchain_extent(capabilities, width, height);
  EXPECT_EQ(vk::Extent2D(width, height), capabilities.currentExtent);
}

TEST_F(
    TriangleTest,
    ReturnsSwapchainExtentEqualToWidthAndHeightGivenCurrentExtentIsUndefined) {
  vk::SurfaceCapabilitiesKHR capabilities = {};
  capabilities.currentExtent = vk::Extent2D(UINT32_MAX, UINT32_MAX);
  uint32_t width = 1;
  uint32_t height = 2;
  EXPECT_EQ(vka::select_swapchain_extent(capabilities, width, height),
            vk::Extent2D(width, height));
}

TEST_F(TriangleTest,
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

TEST_F(TriangleTest, CreatesSwapchainWithoutThrowingException) {
  WindowManager window_manager;
  vk::UniqueSurfaceKHR surface =
      vk::UniqueSurfaceKHR(window_manager.surface(instance()));
  std::vector<vk::Bool32> presentation_support = vka::get_presentation_support(
      physical_device(), *surface, queue_family_properties().size());
  const vk::SurfaceCapabilitiesKHR capabilities =
      physical_device().getSurfaceCapabilitiesKHR(*surface);
  const std::vector<vk::SurfaceFormatKHR> formats =
      physical_device().getSurfaceFormatsKHR(*surface);
  const vk::SurfaceFormatKHR format = vka::select_surface_format(formats);
  uint32_t width = 500;
  uint32_t height = 500;
  const vk::Extent2D extent =
      vka::select_swapchain_extent(capabilities, width, height);
  EXPECT_NO_THROW(vka::create_swapchain(format, extent, capabilities, device(),
                                        *surface, nullptr));
  surface.release();
}

TEST_F(TriangleTest, CreatesSwapchainImageViewsWithoutThrowingException) {
  EXPECT_NO_THROW(vka::create_swapchain_image_views(
      device(), swapchain_images(), surface_format()));
}

TEST_F(TriangleTest, ReturnsVectorOfBytesGivenFileExists) {
  const std::string file_name = "file.txt";
  const std::string content = "Content";
  std::ofstream f;
  f.open(file_name);
  f << content;
  f.close();
  EXPECT_EQ(vka::read_file(file_name).size(), content.size());
}

TEST_F(TriangleTest, ReturnsVectorWithSizeEqualToZeroGivenFileDoesExist) {
  EXPECT_EQ(vka::read_file("unknown").size(), 0);
}

TEST_F(TriangleTest, CreatesShaderModuleWithoutThrowingException) {
  const std::vector<char> code = vka::read_file("vert.spv");
  EXPECT_NO_THROW(vka::create_shader_module(device(), code));
}

TEST_F(TriangleTest, CreatesPipelineLayoutWithoutThrowingException) {
  EXPECT_NO_THROW(vka::create_pipeline_layout(device()));
}

TEST_F(TriangleTest, CreatesRenderPassWithoutThrowingException) {
  EXPECT_NO_THROW(
      vka::create_render_pass(device(), vk::Format::eB8G8R8A8Unorm));
}

TEST_F(TriangleTest, CreatesGraphicsPipelineWithoutThrowingException) {
  EXPECT_NO_THROW(vka::create_graphics_pipeline(device(), render_pass(),
                                                swapchain_extent()));
}

TEST_F(TriangleTest, CreatesFramebuffersWithoutThrowingException) {
  EXPECT_NO_THROW(vka::create_framebuffers(
      device(), render_pass(), swapchain_extent(), swapchain_image_views()));
}

TEST_F(TriangleTest, RecordsCommandBuffersWithoutThrowingException) {
  EXPECT_NO_THROW(vka::record_command_buffers(
      device(), command_buffers(), render_pass(), graphics_pipeline(),
      framebuffers(), swapchain_extent(), vertex_buffer(), vertices()));
}

TEST_F(TriangleTest, DrawsFrameWithoutThrowingException) {
  std::vector<vk::UniqueCommandBuffer> command_buffers =
      vka::create_command_buffers(device(), command_pool(),
                                  static_cast<uint32_t>(framebuffers().size()));
  std::vector<vk::CommandBuffer> command_buffer_pointers;
  for (const auto &command_buffer : command_buffers) {
    command_buffer_pointers.push_back(*command_buffer);
  }
  vka::record_command_buffers(device(), command_buffer_pointers, render_pass(),
                              graphics_pipeline(), framebuffers(),
                              swapchain_extent(), vertex_buffer(), vertices());
  EXPECT_NO_THROW(vka::draw_frame(device(), swapchain(),
                                  command_buffer_pointers, queue_index()));
}
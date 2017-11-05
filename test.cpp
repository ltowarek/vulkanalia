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
  static std::vector<const char *> extension_names() {
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
  static void TearDownTestCase() { instance_.release(); }
  void TearDown() { release(); }
  static const vk::Instance &instance() {
    if (!instance_) {
      vk::ApplicationInfo application_info =
          vka::create_application_info("Test", {1, 2, 3});
      instance_ = vka::create_instance(application_info,
                                       WindowManager::extension_names());
    }
    return *instance_;
  }
  const vk::Device &device() {
    if (!device_) {
      device_ = vka::create_device(physical_device(), queue_index());
    }
    return *device_;
  }
  const uint32_t queue_index() {
    if (queue_index_ == UINT32_MAX) {
      std::vector<vk::Bool32> presentation_support =
          vka::get_presentation_support(physical_device(), surface(),
                                        queue_family_properties().size());
      queue_index_ = vka::find_graphics_and_presentation_queue_family_index(
          queue_family_properties(), presentation_support);
    }
    return queue_index_;
  }
  const vk::CommandPool &command_pool() {
    if (!command_pool_) {
      command_pool_ = vka::create_command_pool(device(), queue_index());
    }
    return *command_pool_;
  }
  const vk::Buffer &vertex_buffer() {
    if (!vertex_buffer_) {
      const uint32_t size =
          static_cast<uint32_t>(sizeof(vertices()[0]) * vertices().size());
      vertex_buffer_ =
          vka::create_buffer(device(), size,
                             vk::BufferUsageFlagBits::eVertexBuffer |
                                 vk::BufferUsageFlagBits::eTransferDst);
    }
    return *vertex_buffer_;
  }
  const vk::DeviceMemory &vertex_buffer_memory() {
    if (!vertex_buffer_memory_) {
      vertex_buffer_memory_ = vka::allocate_buffer_memory(
          device(), vertex_buffer(), physical_device().getMemoryProperties(),
          vk::MemoryPropertyFlagBits::eDeviceLocal);
      device().bindBufferMemory(vertex_buffer(), *vertex_buffer_memory_, 0);
    }
    return *vertex_buffer_memory_;
  }
  const vk::Buffer &index_buffer() {
    if (!index_buffer_) {
      const uint32_t size =
          static_cast<uint32_t>(sizeof(indices()[0]) * indices().size());
      index_buffer_ =
          vka::create_buffer(device(), size,
                             vk::BufferUsageFlagBits::eIndexBuffer |
                                 vk::BufferUsageFlagBits::eTransferDst);
    }
    return *index_buffer_;
  }
  const vk::DeviceMemory &index_buffer_memory() {
    if (!index_buffer_memory_) {
      index_buffer_memory_ = vka::allocate_buffer_memory(
          device(), index_buffer(), physical_device().getMemoryProperties(),
          vk::MemoryPropertyFlagBits::eDeviceLocal);
      device().bindBufferMemory(index_buffer(), *index_buffer_memory_, 0);
    }
    return *index_buffer_memory_;
  }
  const vk::Buffer &staging_vertex_buffer() {
    if (!staging_vertex_buffer_) {
      const uint32_t size =
          static_cast<uint32_t>(sizeof(vertices()[0]) * vertices().size());
      staging_vertex_buffer_ = vka::create_buffer(
          device(), size, vk::BufferUsageFlagBits::eTransferSrc);
    }
    return *staging_vertex_buffer_;
  }
  const vk::DeviceMemory &staging_vertex_buffer_memory() {
    if (!staging_vertex_buffer_memory_) {
      staging_vertex_buffer_memory_ = vka::allocate_buffer_memory(
          device(), staging_vertex_buffer(),
          physical_device().getMemoryProperties(),
          vk::MemoryPropertyFlagBits::eHostVisible |
              vk::MemoryPropertyFlagBits::eHostCoherent);
      device().bindBufferMemory(staging_vertex_buffer(),
                                *staging_vertex_buffer_memory_, 0);
    }
    return *staging_vertex_buffer_memory_;
  }
  const vk::Buffer &staging_index_buffer() {
    if (!staging_index_buffer_) {
      const uint32_t size =
          static_cast<uint32_t>(sizeof(indices()[0]) * indices().size());
      staging_index_buffer_ = vka::create_buffer(
          device(), size, vk::BufferUsageFlagBits::eTransferSrc);
    }
    return *staging_index_buffer_;
  }
  const vk::DeviceMemory &staging_index_buffer_memory() {
    if (!staging_index_buffer_memory_) {
      staging_index_buffer_memory_ = vka::allocate_buffer_memory(
          device(), staging_index_buffer(),
          physical_device().getMemoryProperties(),
          vk::MemoryPropertyFlagBits::eHostVisible |
              vk::MemoryPropertyFlagBits::eHostCoherent);
      device().bindBufferMemory(staging_index_buffer(),
                                *staging_index_buffer_memory_, 0);
    }
    return *staging_index_buffer_memory_;
  }
  const vk::Buffer &uniform_buffer() {
    if (!uniform_buffer_) {
      const uint32_t size =
          static_cast<uint32_t>(sizeof(vka::UniformBufferObject));
      uniform_buffer_ = vka::create_buffer(
          device(), size, vk::BufferUsageFlagBits::eUniformBuffer);
    }
    return *uniform_buffer_;
  }
  const vk::DeviceMemory &uniform_buffer_memory() {
    if (!uniform_buffer_memory_) {
      uniform_buffer_memory_ = vka::allocate_buffer_memory(
          device(), uniform_buffer(), physical_device().getMemoryProperties(),
          vk::MemoryPropertyFlagBits::eHostVisible |
              vk::MemoryPropertyFlagBits::eHostCoherent);
      device().bindBufferMemory(uniform_buffer(), *uniform_buffer_memory_, 0);
    }
    return *uniform_buffer_memory_;
  }
  const vk::DescriptorPool &descriptor_pool() {
    if (!descriptor_pool_) {
      descriptor_pool_ = vka::create_descriptor_pool(device());
    }
    return *descriptor_pool_;
  }
  const vk::DescriptorSetLayout &descriptor_set_layout() {
    if (!descriptor_set_layout_) {
      descriptor_set_layout_ = vka::create_descriptor_set_layout(device());
    }
    return *descriptor_set_layout_;
  }
  const std::vector<vk::DescriptorSet> descriptor_sets() {
    if (descriptor_sets_.empty()) {
      descriptor_sets_ = vka::create_descriptor_sets(
          device(), descriptor_pool(), descriptor_set_layout());
    }
    std::vector<vk::DescriptorSet> descriptor_sets;
    for (const auto &descriptor_set : descriptor_sets_) {
      descriptor_sets.push_back(*descriptor_set);
    }
    return descriptor_sets;
  }
  const vk::SurfaceKHR &surface() {
    if (!surface_) {
      surface_ = vk::UniqueSurfaceKHR(window_manager_.surface(instance()));
      std::vector<vk::Bool32> presentation_support =
          vka::get_presentation_support(physical_device(), *surface_,
                                        queue_family_properties().size());
    }
    return *surface_;
  }
  const vk::PhysicalDevice &physical_device() {
    if (!physical_device_) {
      const std::vector<vk::PhysicalDevice> devices =
          instance().enumeratePhysicalDevices();
      physical_device_ = vka::select_physical_device(devices);
    }
    return physical_device_;
  }
  const std::vector<vk::QueueFamilyProperties> &queue_family_properties() {
    if (queue_family_properties_.empty()) {
      queue_family_properties_ = physical_device().getQueueFamilyProperties();
    }
    return queue_family_properties_;
  }
  const vk::SurfaceFormatKHR surface_format() {
    const vk::SurfaceKHR s = surface();
    const std::vector<vk::SurfaceFormatKHR> formats =
        physical_device().getSurfaceFormatsKHR(s);
    return vka::select_surface_format(formats);
  }
  const vk::Extent2D swapchain_extent() {
    const vk::SurfaceKHR s = surface();
    const vk::SurfaceCapabilitiesKHR capabilities =
        physical_device().getSurfaceCapabilitiesKHR(s);
    uint32_t width = 500;
    uint32_t height = 500;
    return vka::select_swapchain_extent(capabilities, width, height);
  }
  const vk::SwapchainKHR &swapchain() {
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
  std::vector<vk::Image> swapchain_images() {
    if (swapchain_images_.empty()) {
      swapchain_images_ = device().getSwapchainImagesKHR(swapchain());
    }
    return swapchain_images_;
  }
  std::vector<vk::ImageView> swapchain_image_views() {
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
  const vk::RenderPass &render_pass() {
    if (!render_pass_) {
      render_pass_ = vka::create_render_pass(device(), surface_format().format);
    }
    return *render_pass_;
  }
  const vk::PipelineLayout &pipeline_layout() {
    if (!pipeline_layout_) {
      pipeline_layout_ =
          vka::create_pipeline_layout(device(), descriptor_set_layout());
    }
    return *pipeline_layout_;
  }
  const vk::Pipeline &graphics_pipeline() {
    if (!graphics_pipeline_) {
      graphics_pipeline_ = vka::create_graphics_pipeline(
          device(), render_pass(), swapchain_extent(), pipeline_layout());
    }
    return *graphics_pipeline_;
  }
  const std::vector<vk::Framebuffer> framebuffers() {
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
  const std::vector<vk::CommandBuffer> command_buffers() {
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
  const vk::Image &texture_image() {
    if (!texture_image_) {
      texture_image_ = vka::create_image(
          device(), texture().width, texture().height,
          vk::Format::eR8G8B8A8Unorm, vk::ImageTiling::eOptimal,
          vk::ImageUsageFlagBits::eTransferDst |
              vk::ImageUsageFlagBits::eSampled);
    }
    return *texture_image_;
  }
  const vk::DeviceMemory &texture_image_memory() {
    if (!texture_image_memory_) {
      texture_image_memory_ = vka::allocate_image_memory(
          device(), texture_image(), physical_device().getMemoryProperties(),
          vk::MemoryPropertyFlagBits::eDeviceLocal);
      device().bindImageMemory(texture_image(), *texture_image_memory_, 0);
    }
    return *texture_image_memory_;
  }
  const vk::ImageView &texture_image_view() {
    if (!texture_image_view_) {
      texture_image_view_ =
          vka::create_texture_image_view(device(), texture_image());
    }
    return *texture_image_view_;
  }
  const vk::Sampler &texture_sampler() {
    if (!texture_sampler_) {
      texture_sampler_ = vka::create_texture_sampler(device());
    }
    return *texture_sampler_;
  }
  const vk::Buffer &staging_texture_buffer() {
    if (!staging_texture_buffer_) {
      const uint32_t size = static_cast<uint32_t>(texture().size);
      staging_texture_buffer_ = vka::create_buffer(
          device(), size, vk::BufferUsageFlagBits::eTransferSrc);
    }
    return *staging_texture_buffer_;
  }
  const vk::DeviceMemory &staging_texture_buffer_memory() {
    if (!staging_texture_buffer_memory_) {
      staging_texture_buffer_memory_ = vka::allocate_buffer_memory(
          device(), staging_texture_buffer(),
          physical_device().getMemoryProperties(),
          vk::MemoryPropertyFlagBits::eHostVisible |
              vk::MemoryPropertyFlagBits::eHostCoherent);
      device().bindBufferMemory(staging_texture_buffer(),
                                *staging_texture_buffer_memory_, 0);
    }
    return *staging_texture_buffer_memory_;
  }
  const vka::Texture &texture() {
    if (texture_.data == nullptr) {
      texture_ = std::move(vka::Texture("texture.jpg"));
    }
    return texture_;
  }
  const std::vector<vka::Vertex> vertices() { return vertices_; }
  const std::vector<uint16_t> indices() { return indices_; }
  const vka::UniformBufferObject uniform_buffer_object() {
    return uniform_buffer_object_;
  }
  void release() {
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

    staging_texture_buffer_.release();
    staging_texture_buffer_memory_.release();
    texture_sampler_.release();
    texture_image_view_.release();
    texture_image_.release();
    texture_image_memory_.release();
    index_buffer_.release();
    index_buffer_memory_.release();
    vertex_buffer_.release();
    vertex_buffer_memory_.release();
    staging_index_buffer_.release();
    staging_index_buffer_memory_.release();
    staging_vertex_buffer_.release();
    staging_vertex_buffer_memory_.release();
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
  }

private:
  static vk::UniqueInstance instance_;

  uint32_t queue_index_ = UINT32_MAX;
  const std::vector<vka::Vertex> vertices_ = {
      {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
      {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
      {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
      {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}};
  const std::vector<uint16_t> indices_ = {0, 1, 2, 2, 3, 0};
  vka::Texture texture_;
  const vka::UniformBufferObject uniform_buffer_object_ = {};
  WindowManager window_manager_ = WindowManager();
  vk::PhysicalDevice physical_device_ = vk::PhysicalDevice();
  std::vector<vk::QueueFamilyProperties> queue_family_properties_ =
      std::vector<vk::QueueFamilyProperties>();
  std::vector<vk::Image> swapchain_images_ = std::vector<vk::Image>();

  vk::UniqueSurfaceKHR surface_ = vk::UniqueSurfaceKHR();
  vk::UniqueDevice device_ = vk::UniqueDevice();
  vk::UniqueCommandPool command_pool_ = vk::UniqueCommandPool();
  vk::UniqueDescriptorPool descriptor_pool_ = vk::UniqueDescriptorPool();
  vk::UniqueDescriptorSetLayout descriptor_set_layout_ =
      vk::UniqueDescriptorSetLayout();
  std::vector<vk::UniqueDescriptorSet> descriptor_sets_ =
      std::vector<vk::UniqueDescriptorSet>();
  vk::UniqueDeviceMemory uniform_buffer_memory_ = vk::UniqueDeviceMemory();
  vk::UniqueBuffer uniform_buffer_ = vk::UniqueBuffer();
  vk::UniqueDeviceMemory staging_vertex_buffer_memory_ =
      vk::UniqueDeviceMemory();
  vk::UniqueBuffer staging_vertex_buffer_ = vk::UniqueBuffer();
  vk::UniqueDeviceMemory staging_index_buffer_memory_ =
      vk::UniqueDeviceMemory();
  vk::UniqueBuffer staging_index_buffer_ = vk::UniqueBuffer();
  vk::UniqueDeviceMemory vertex_buffer_memory_ = vk::UniqueDeviceMemory();
  vk::UniqueBuffer vertex_buffer_ = vk::UniqueBuffer();
  vk::UniqueDeviceMemory index_buffer_memory_ = vk::UniqueDeviceMemory();
  vk::UniqueBuffer index_buffer_ = vk::UniqueBuffer();
  vk::UniqueSwapchainKHR swapchain_ = vk::UniqueSwapchainKHR();
  std::vector<vk::UniqueImageView> swapchain_image_views_ =
      std::vector<vk::UniqueImageView>();
  vk::UniqueRenderPass render_pass_ = vk::UniqueRenderPass();
  vk::UniquePipeline graphics_pipeline_ = vk::UniquePipeline();
  vk::UniquePipelineLayout pipeline_layout_ = vk::UniquePipelineLayout();
  std::vector<vk::UniqueCommandBuffer> command_buffers_ =
      std::vector<vk::UniqueCommandBuffer>();
  std::vector<vk::UniqueFramebuffer> framebuffers_ =
      std::vector<vk::UniqueFramebuffer>();
  vk::UniqueSampler texture_sampler_ = vk::UniqueSampler();
  vk::UniqueImageView texture_image_view_ = vk::UniqueImageView();
  vk::UniqueImage texture_image_ = vk::UniqueImage();
  vk::UniqueDeviceMemory texture_image_memory_ = vk::UniqueDeviceMemory();
  vk::UniqueDeviceMemory staging_texture_buffer_memory_ =
      vk::UniqueDeviceMemory();
  vk::UniqueBuffer staging_texture_buffer_ = vk::UniqueBuffer();
};

vk::UniqueInstance TriangleTest::instance_ = vk::UniqueInstance();

TEST_F(TriangleTest, SetsWidthOfTheLoadedTexture) {
  EXPECT_EQ(vka::Texture("texture.jpg").width, 512);
}

TEST_F(TriangleTest, SetsHeightOfTheLoadedTexture) {
  EXPECT_EQ(vka::Texture("texture.jpg").height, 512);
}

TEST_F(TriangleTest, SetsSizeOfTheLoadedTexture) {
  const uint32_t width = 512;
  const uint32_t height = 512;
  const uint32_t bytes_per_pixel = 4;
  EXPECT_EQ(vka::Texture("texture.jpg").size, width * height * bytes_per_pixel);
}

TEST_F(TriangleTest, SetsDataOfTheLoadedTexture) {
  EXPECT_NE(vka::Texture("texture.jpg").data, nullptr);
}

TEST_F(TriangleTest, SetsNullptrGivenTextureNotExit) {
  EXPECT_EQ(vka::Texture("").data, nullptr);
}

TEST_F(TriangleTest, Returns0GivenDeltaTimeIs0) {
  const auto time_value =
      std::chrono::time_point<std::chrono::high_resolution_clock>(
          std::chrono::milliseconds(2));
  EXPECT_FLOAT_EQ(vka::get_delta_time_per_second(time_value, time_value), 0.0f);
}

TEST_F(TriangleTest, Returns1GivenDeltaTimeIs1Second) {
  const auto start_time =
      std::chrono::time_point<std::chrono::high_resolution_clock>(
          std::chrono::milliseconds(1));
  const auto current_time =
      std::chrono::time_point<std::chrono::high_resolution_clock>(
          std::chrono::milliseconds(1001));
  EXPECT_FLOAT_EQ(vka::get_delta_time_per_second(start_time, current_time), 1);
}

TEST_F(TriangleTest, Returns1Of2GivenDeltaTimeIsHalfSecond) {
  const auto start_time =
      std::chrono::time_point<std::chrono::high_resolution_clock>(
          std::chrono::milliseconds(1));
  const auto current_time =
      std::chrono::time_point<std::chrono::high_resolution_clock>(
          std::chrono::milliseconds(501));
  EXPECT_FLOAT_EQ(vka::get_delta_time_per_second(start_time, current_time),
                  1 / 2.0f);
}

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
  EXPECT_NO_THROW(
      vka::create_buffer(device(), size,
                         vk::BufferUsageFlagBits::eVertexBuffer |
                             vk::BufferUsageFlagBits::eTransferDst));
}

TEST_F(TriangleTest, CreatesStagingBufferWithoutThrowingException) {
  const uint32_t size =
      static_cast<uint32_t>(sizeof(vertices()[0]) * vertices().size());
  EXPECT_NO_THROW(vka::create_buffer(device(), size,
                                     vk::BufferUsageFlagBits::eTransferSrc));
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
      vk::MemoryPropertyFlagBits::eDeviceLocal));
}

TEST_F(TriangleTest, AllocatesMemoryForStagingBufferWithoutThrowingException) {
  EXPECT_NO_THROW(vka::allocate_buffer_memory(
      device(), staging_vertex_buffer(),
      physical_device().getMemoryProperties(),
      vk::MemoryPropertyFlagBits::eHostVisible |
          vk::MemoryPropertyFlagBits::eHostCoherent));
}

TEST_F(TriangleTest, FillsBufferWithVectorWithoutThrowingException) {
  EXPECT_NO_THROW(
      vka::fill_buffer(device(), staging_vertex_buffer_memory(), vertices()));
}

TEST_F(TriangleTest, FillsBufferWithObjectWithoutThrowingException) {
  EXPECT_NO_THROW(vka::fill_buffer(device(), uniform_buffer_memory(),
                                   uniform_buffer_object()));
}

TEST_F(TriangleTest, BeginsCommandWithoutThrowingException) {
  EXPECT_NO_THROW(vka::begin_command(device(), command_pool()));
}

TEST_F(TriangleTest, EndsCommandWithoutThrowingException) {
  vk::UniqueCommandBuffer command_buffer =
      vka::begin_command(device(), command_pool());
  EXPECT_NO_THROW(
      vka::end_command(device(), std::move(command_buffer), queue_index()));
}

TEST_F(TriangleTest, CopiesBufferToBufferWithoutThrowingException) {
  vertex_buffer_memory();
  const uint32_t size =
      static_cast<uint32_t>(sizeof(vertices()[0]) * vertices().size());
  vka::fill_buffer(device(), staging_vertex_buffer_memory(), vertices());
  EXPECT_NO_THROW(vka::copy_buffer_to_buffer(device(), staging_vertex_buffer(),
                                             vertex_buffer(), size,
                                             command_pool(), queue_index()));
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

TEST_F(TriangleTest, CreatesImageViewWithoutThrowingException) {
  EXPECT_NO_THROW(vka::create_image_view(device(), swapchain_images()[0],
                                         surface_format().format));
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
  EXPECT_NO_THROW(
      vka::create_pipeline_layout(device(), descriptor_set_layout()));
}

TEST_F(TriangleTest, CreatesRenderPassWithoutThrowingException) {
  EXPECT_NO_THROW(
      vka::create_render_pass(device(), vk::Format::eB8G8R8A8Unorm));
}

TEST_F(TriangleTest, CreatesDescriptorPoolWithoutThrowingException) {
  EXPECT_NO_THROW(vka::create_descriptor_pool(device()));
}

TEST_F(TriangleTest, CreatesDescriptorSetLayoutWithoutThrowingException) {
  EXPECT_NO_THROW(vka::create_descriptor_set_layout(device()));
}

TEST_F(TriangleTest, CreatesDescriptorSetsWithoutThrowingException) {
  EXPECT_NO_THROW(vka::create_descriptor_sets(device(), descriptor_pool(),
                                              descriptor_set_layout()));
}

TEST_F(TriangleTest, UpdatesDescriptorSetsWithoutThrowingException) {
  uniform_buffer_memory();
  texture_image_memory();
  EXPECT_NO_THROW(
      vka::update_descriptor_sets(device(), descriptor_sets(), uniform_buffer(),
                                  texture_image_view(), texture_sampler()));
}

TEST_F(TriangleTest, CreatesGraphicsPipelineWithoutThrowingException) {
  EXPECT_NO_THROW(vka::create_graphics_pipeline(
      device(), render_pass(), swapchain_extent(), pipeline_layout()));
}

TEST_F(TriangleTest, CreatesFramebuffersWithoutThrowingException) {
  EXPECT_NO_THROW(vka::create_framebuffers(
      device(), render_pass(), swapchain_extent(), swapchain_image_views()));
}

TEST_F(TriangleTest, RecordsCommandBuffersWithoutThrowingException) {
  vertex_buffer_memory();
  index_buffer_memory();
  uniform_buffer_memory();
  texture_image_memory();
  vka::update_descriptor_sets(device(), descriptor_sets(), uniform_buffer(),
                              texture_image_view(), texture_sampler());
  EXPECT_NO_THROW(vka::record_command_buffers(
      device(), command_buffers(), render_pass(), graphics_pipeline(),
      pipeline_layout(), framebuffers(), swapchain_extent(), vertex_buffer(),
      index_buffer(), indices(), descriptor_sets()));
}

TEST_F(TriangleTest, DrawsFrameWithoutThrowingException) {
  vertex_buffer_memory();
  const uint32_t vertices_size =
      static_cast<uint32_t>(sizeof(vertices()[0]) * vertices().size());
  vka::fill_buffer(device(), staging_vertex_buffer_memory(), vertices());
  vka::copy_buffer_to_buffer(device(), staging_vertex_buffer(), vertex_buffer(),
                             vertices_size, command_pool(), queue_index());
  index_buffer_memory();
  const uint32_t indices_size =
      static_cast<uint32_t>(sizeof(indices()[0]) * indices().size());
  vka::fill_buffer(device(), staging_index_buffer_memory(), indices());
  vka::copy_buffer_to_buffer(device(), staging_index_buffer(), index_buffer(),
                             indices_size, command_pool(), queue_index());
  std::vector<vk::UniqueCommandBuffer> command_buffers =
      vka::create_command_buffers(device(), command_pool(),
                                  static_cast<uint32_t>(framebuffers().size()));
  std::vector<vk::CommandBuffer> command_buffer_pointers;
  for (const auto &command_buffer : command_buffers) {
    command_buffer_pointers.push_back(*command_buffer);
  }
  uniform_buffer_memory();
  texture_image_memory();
  vka::update_descriptor_sets(device(), descriptor_sets(), uniform_buffer(),
                              texture_image_view(), texture_sampler());
  vka::record_command_buffers(
      device(), command_buffer_pointers, render_pass(), graphics_pipeline(),
      pipeline_layout(), framebuffers(), swapchain_extent(), vertex_buffer(),
      index_buffer(), indices(), descriptor_sets());
  EXPECT_NO_THROW(vka::draw_frame(device(), swapchain(),
                                  command_buffer_pointers, queue_index()));
}

TEST_F(TriangleTest, CreatesImageWithoutThrowingException) {
  EXPECT_NO_THROW(vka::create_image(
      device(), 32, 64, vk::Format::eR8G8B8A8Unorm, vk::ImageTiling::eOptimal,
      vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled));
}

TEST_F(TriangleTest, AllocatesMemoryForImageWithoutThrowingException) {
  EXPECT_NO_THROW(vka::allocate_image_memory(
      device(), texture_image(), physical_device().getMemoryProperties(),
      vk::MemoryPropertyFlagBits::eDeviceLocal));
}

TEST_F(TriangleTest, TransitionsImageLayoutWithoutThrowingException) {
  EXPECT_NO_THROW(vka::transition_image_layout(
      device(), command_pool(), queue_index(), vk::ImageLayout::eUndefined,
      vk::ImageLayout::eTransferDstOptimal, texture_image()));
}

TEST_F(TriangleTest, CopiesBufferToImageWithoutThrowingException) {
  staging_texture_buffer_memory();
  vka::fill_buffer(device(), staging_texture_buffer_memory(), texture());
  texture_image_memory();
  vka::transition_image_layout(
      device(), command_pool(), queue_index(), vk::ImageLayout::eUndefined,
      vk::ImageLayout::eTransferDstOptimal, texture_image());
  EXPECT_NO_THROW(vka::copy_buffer_to_image(
      device(), staging_texture_buffer(), texture_image(), texture().width,
      texture().height, command_pool(), queue_index()));
}

TEST_F(TriangleTest, CreatesTextureImageViewWithoutThrowingException) {
  texture_image_memory();
  EXPECT_NO_THROW(vka::create_texture_image_view(device(), texture_image()));
}

TEST_F(TriangleTest, CreatesTextureSamplerWithoutThrowingException) {
  EXPECT_NO_THROW(vka::create_texture_sampler(device()));
}

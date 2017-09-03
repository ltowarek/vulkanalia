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

TEST(TriangleExample, CreatesInstanceWithoutThrowingException) {
  vk::ApplicationInfo application_info =
      vka::create_application_info("Test", {1, 2, 3});
  EXPECT_NO_THROW(vka::create_instance(application_info));
}

class TriangleExampleWithSharedInstance : public ::testing::Test {
protected:
  static void SetUpTestCase() {
    const vk::ApplicationInfo application_info =
        vka::create_application_info("Test", {1, 2, 3});
    instance_ = vka::create_instance(application_info);
  }
  static vk::UniqueInstance instance_;
};

vk::UniqueInstance TriangleExampleWithSharedInstance::instance_ =
    vk::UniqueInstance();

TEST_F(TriangleExampleWithSharedInstance,
       SelectsNonEmptyPhysicalDeviceIfAnyIsAvailable) {
  std::vector<vk::PhysicalDevice> devices =
      instance_.get().enumeratePhysicalDevices();
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

TEST_F(TriangleExampleWithSharedInstance,
       CreatesLogicalDeviceWithoutThrowingException) {
  const std::vector<vk::PhysicalDevice> devices =
      instance_.get().enumeratePhysicalDevices();
  const vk::PhysicalDevice physical_device =
      vka::select_physical_device(devices);
  const std::vector<vk::QueueFamilyProperties> queues =
      physical_device.getQueueFamilyProperties();
  const uint32_t queue_index = 0;
  EXPECT_NO_THROW(vka::create_device(physical_device, queue_index));
}

class TriangleExampleWithSharedDevice
    : public TriangleExampleWithSharedInstance {
protected:
  static void SetUpTestCase() {
    const std::vector<vk::PhysicalDevice> devices =
        instance_.get().enumeratePhysicalDevices();
    const vk::PhysicalDevice physical_device =
        vka::select_physical_device(devices);
    const std::vector<vk::QueueFamilyProperties> queues =
        physical_device.getQueueFamilyProperties();
    device_ = vka::create_device(physical_device, queue_index_);
  }
  static vk::UniqueDevice device_;
  static const uint32_t queue_index_;
};

vk::UniqueDevice TriangleExampleWithSharedDevice::device_ = vk::UniqueDevice();
const uint32_t TriangleExampleWithSharedDevice::queue_index_ = 0;

TEST_F(TriangleExampleWithSharedDevice,
       CreatesCommandPoolWithoutThrowingException) {
  EXPECT_NO_THROW(vka::create_command_pool(device_.get(), queue_index_));
}

class TriangleExampleWithSharedCommandPool
    : public TriangleExampleWithSharedDevice {
protected:
  static void SetUpTestCase() {
    command_pool_ = vka::create_command_pool(device_.get(), queue_index_);
  }
  static vk::UniqueCommandPool command_pool_;
};

vk::UniqueCommandPool TriangleExampleWithSharedCommandPool::command_pool_ =
    vk::UniqueCommandPool();

TEST_F(TriangleExampleWithSharedCommandPool,
       CreatesCommandBuffersWithoutThrowingException) {
  EXPECT_NO_THROW(
      vka::create_command_buffers(device_.get(), command_pool_.get()));
}

TEST(TriangleExample,
     ReturnsNonZeroValueGivenWindowClassIsSuccessfullyRegistered) {
  HINSTANCE hInstance = GetModuleHandle(nullptr);
  const std::string class_name = "class_name";
  EXPECT_TRUE(vka::register_window_class(hInstance, class_name));
}

TEST(TriangleExample, ReturnsNonNULLHandleGivenWindowIsSuccessfullyCreated) {
  HINSTANCE hInstance = GetModuleHandle(nullptr);
  const std::string class_name = "class_name";
  vka::register_window_class(hInstance, class_name);
  EXPECT_TRUE(vka::create_window(hInstance, class_name));
}
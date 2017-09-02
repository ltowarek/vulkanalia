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
    const uint32_t queue_index = 0;
    device_ = vka::create_device(physical_device, queue_index);
  }
  static vk::UniqueDevice device_;
};

vk::UniqueDevice TriangleExampleWithSharedDevice::device_ = vk::UniqueDevice();

TEST_F(TriangleExampleWithSharedDevice,
       CreatesCommandPoolWithoutThrowingException) {
  const uint32_t queue_index = 0;
  EXPECT_NO_THROW(vka::create_command_pool(device_.get(), queue_index));
}
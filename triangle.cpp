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

namespace vka {
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

  return physical_device.createDeviceUnique(device_info);
}
}

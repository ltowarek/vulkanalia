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

#ifndef VK_USE_PLATFORM_WIN32_KHR
#define VK_USE_PLATFORM_WIN32_KHR
#endif
#include <vulkan/vulkan.hpp>

namespace vka {
struct Version {
  uint32_t major;
  uint32_t minor;
  uint32_t patch;
};
vk::ApplicationInfo create_application_info(const std::string name,
                                            const Version version);
vk::UniqueInstance create_instance(const vk::ApplicationInfo application_info);
vk::PhysicalDevice
select_physical_device(const std::vector<vk::PhysicalDevice> &devices);
uint32_t find_graphics_queue_family_index(
    const std::vector<vk::QueueFamilyProperties> &queues);
vk::UniqueDevice create_device(const vk::PhysicalDevice &physical_device,
                               const uint32_t queue_index);
vk::UniqueCommandPool create_command_pool(const vk::Device &device,
                                          const uint32_t queue_index);
std::vector<vk::UniqueCommandBuffer>
create_command_buffers(const vk::Device &device,
                       const vk::CommandPool &command_pool);
}

#endif
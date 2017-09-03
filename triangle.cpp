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
vk::Format
select_surface_color_format(const std::vector<vk::SurfaceFormatKHR> &formats) {
  if (formats.size() == 1 && formats[0].format == vk::Format::eUndefined) {
    return vk::Format::eB8G8R8A8Unorm;
  } else {
    return formats[0].format;
  }
}
}

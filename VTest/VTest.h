// VTest.h : Include file for standard system include files,
// or project specific include files.

#pragma once
#define NOMINMAX
#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_INCLUDE_VULKAN
#include <iostream>
#include <optional>
#include <ostream>
#include <fstream>
#include <set>
#include <stdexcept>
#include <algorithm>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <vector>

class HelloTriangleApplication {
public:
   void run() {
      initVulkan();
      mainLoop();
      cleanup();
   }

private:
   struct QueueFamilyIndices {
      std::optional<uint32_t> graphicsFamily;
      std::optional<uint32_t> presentFamily;

      bool isComplete() {
         return graphicsFamily.has_value() &&
            presentFamily.has_value();
      }
   };

   struct SwapChainSupportDetails {
      VkSurfaceCapabilitiesKHR capabilities{};
      std::vector<VkSurfaceFormatKHR> formats;
      std::vector<VkPresentModeKHR> presentModes;
   };

   VkInstance instance{};
   const uint32_t WIDTH = 800;
   const uint32_t HEIGHT = 600;
   const std::vector<const char*> validationLayers = {
      "VK_LAYER_KHRONOS_validation"
   };
   const std::vector<const char*> deviceExtensions = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME
   };
   VkDebugUtilsMessengerEXT debugMessenger = nullptr;
#ifdef NDEBUG
   static constexpr bool enableValidationLayers = false;
#else
   static constexpr bool enableValidationLayers = true;
#endif
   VkPhysicalDevice physicalDevice{};
   VkDevice device{};
   VkQueue graphicsQueue{};
   GLFWwindow* window{};
   VkSurfaceKHR surface{};
   VkQueue presentQueue{};
   VkSwapchainKHR swapChain{};
   std::vector<VkImage> swapChainImages;
   VkFormat swapChainImageFormat{};
   VkExtent2D swapChainExtent{};
   std::vector<VkImageView> swapChainImageViews;
   VkRenderPass renderPass{};
   VkPipelineLayout pipelineLayout = nullptr;
   VkPipeline graphicsPipeline = nullptr;
   std::vector<VkFramebuffer> swapChainFramebuffers;
   VkCommandPool commandPool = nullptr;
   std::vector<VkCommandBuffer> commandBuffers;
   const int MAX_FRAMES_IN_FLIGHT = 2;
   std::size_t currentFrame = 0;
   std::vector<VkSemaphore> imageAvailableSemaphores;
   std::vector<VkSemaphore> renderFinishedSemaphores;
   std::vector<VkFence> inFlightFences;
   std::vector<VkFence> imagesInFlight;

   bool checkValidationLayerSupport() {
      uint32_t layerCount;
      vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

      std::vector<VkLayerProperties> availableLayers(layerCount);
      vkEnumerateInstanceLayerProperties(&layerCount,
         availableLayers.data());

      for (const char* layerName : validationLayers) {
         bool layerFound = false;

         for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
               layerFound = true;
               break;
            }
         }
         if (!layerFound) {
            return false;
         }
      }
      return true;
   }

   static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
      VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
      VkDebugUtilsMessageTypeFlagsEXT messageType,
      const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
      void* pUserData) {
      switch (messageSeverity) {
      case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
         std::cerr << "Info: ";
         break;
      case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
         std::cerr << "Warning: ";
         break;
      case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
         std::cerr << "Error: ";
         break;
      default:
         std::cerr << "Unknown severity: ";
      }
      std::cerr << "validation layer: " << pCallbackData->pMessage <<
         std::endl;

      return VK_FALSE;
   }

   std::vector<const char*> getRequiredExtensions() {
      uint32_t glfwExtensionCount = 0;
      const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
      std::vector extensions(glfwExtensions,
         glfwExtensions + glfwExtensionCount);

      if constexpr (enableValidationLayers) {
         extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
      }

      return extensions;
   }

   void createInstance() {
      VkApplicationInfo appInfo{};
      appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
      appInfo.pApplicationName = "Hello Triangle";
      appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
      appInfo.pEngineName = "No Engine";
      appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
      appInfo.apiVersion = VK_API_VERSION_1_0;

      VkInstanceCreateInfo createInfo{};
      createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
      createInfo.pApplicationInfo = &appInfo;

      uint32_t glfwExtensionCount = 0;

      const auto extensions = getRequiredExtensions();
      createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
      createInfo.ppEnabledExtensionNames = extensions.data();

      if constexpr (enableValidationLayers) {
         if (!checkValidationLayerSupport()) {
            throw std::runtime_error("validation layers requested, but not available!");
         }
         createInfo.enabledLayerCount =
            static_cast<uint32_t>(validationLayers.size());
         createInfo.ppEnabledLayerNames = validationLayers.data();
      }
      else {
         createInfo.enabledLayerCount = 0;
      }

      VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
      if constexpr (enableValidationLayers) {
         createInfo.enabledLayerCount =
            static_cast<uint32_t>(validationLayers.size());
         createInfo.ppEnabledLayerNames = validationLayers.data();
         populateDebugMessengerCreateInfo(debugCreateInfo);
         createInfo.pNext = &debugCreateInfo;
      }
      else {
         createInfo.enabledLayerCount = 0;
         createInfo.pNext = nullptr;
      }

      VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
      if (result != VK_SUCCESS) {
         throw std::runtime_error("Create Instance Error");
      }
   }

   void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
      createInfo = {};
      createInfo.sType =
         VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
      createInfo.messageSeverity =
         VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
         VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
         VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
      createInfo.messageType =
         VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
         VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
         VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
      createInfo.pfnUserCallback = debugCallback;
   }

   VkResult CreateDebugUtilsMessengerEXT(
      const VkInstance inst
      , const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo
      , const VkAllocationCallbacks* pAllocator
      , VkDebugUtilsMessengerEXT* pDebugMessenger) {
      const auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
         vkGetInstanceProcAddr(inst, "vkCreateDebugUtilsMessengerEXT"));
      if (func != nullptr) {
         return func(inst, pCreateInfo, pAllocator,
            pDebugMessenger);
      }
      return VK_ERROR_EXTENSION_NOT_PRESENT;
   }

   void setupDebugMessenger() {
      if constexpr (!enableValidationLayers) return;
      else {
         VkDebugUtilsMessengerCreateInfoEXT createInfo{};
         populateDebugMessengerCreateInfo(createInfo);

         if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr,
            &debugMessenger) != VK_SUCCESS) {
            throw std::runtime_error("failed to set up debug messenger!");
         }
      }
   }

   void DestroyDebugUtilsMessengerEXT(VkInstance inst
      , const VkDebugUtilsMessengerEXT debugMessenger
      , const VkAllocationCallbacks* pAllocator) {
      const auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
         vkGetInstanceProcAddr(inst, "vkDestroyDebugUtilsMessengerEXT"));
      if (func != nullptr) {
         func(inst, debugMessenger, pAllocator);
      }
   }

   bool checkDeviceExtensionSupport(const VkPhysicalDevice device) const {
      uint32_t extensionCount;
      vkEnumerateDeviceExtensionProperties(device, nullptr,
         &extensionCount, nullptr);
      std::vector<VkExtensionProperties>
         availableExtensions(extensionCount);
      vkEnumerateDeviceExtensionProperties(device, nullptr,
         &extensionCount, availableExtensions.data());

      std::set<std::string>
         requiredExtensions(deviceExtensions.begin(),
            deviceExtensions.end());

      for (const auto& extension : availableExtensions) {
         requiredExtensions.erase(extension.extensionName);
      }

      return requiredExtensions.empty();
      return true;
   }

   bool isDeviceSuitable(VkPhysicalDevice device) {
      QueueFamilyIndices indices = findQueueFamilies(device);
      bool extensionsSupported = checkDeviceExtensionSupport(device);
      bool swapChainAdequate = false;
      if (extensionsSupported) {
         SwapChainSupportDetails swapChainSupport =
            querySwapChainSupport(device);
         swapChainAdequate = !swapChainSupport.formats.empty() &&
            !swapChainSupport.presentModes.empty();
      }
      return indices.isComplete() && extensionsSupported
         && swapChainAdequate;

      VkPhysicalDeviceProperties deviceProperties;
      vkGetPhysicalDeviceProperties(device, &deviceProperties);
      VkPhysicalDeviceFeatures deviceFeatures;
      vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
      return deviceProperties.deviceType ==
         VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
         deviceFeatures.geometryShader;
   }

   VkPhysicalDevice pickPhysicalDevice() {
      uint32_t deviceCount = 0;
      vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
      if (deviceCount == 0) {
         throw std::runtime_error("failed to find GPUs with Vulkan support!");
      }
      std::vector<VkPhysicalDevice> devices(deviceCount);
      vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
      for (const auto& device : devices) {
         if (isDeviceSuitable(device)) {
            physicalDevice = device;
            break;
         }
      }
      if (physicalDevice == nullptr) {
         throw std::runtime_error("failed to find a suitable GPU!");
      }
      return physicalDevice;
   }

   SwapChainSupportDetails querySwapChainSupport(
      VkPhysicalDevice  device) const {
      SwapChainSupportDetails details;
      vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface,
         &details.capabilities);

      uint32_t formatCount;
      vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount,
         nullptr);

      if (formatCount != 0) {
         details.formats.resize(formatCount);
         vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface,
            &formatCount, details.formats.data());
      }

      uint32_t presentModeCount;
      vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface,
         &presentModeCount, nullptr);
      if (presentModeCount != 0) {
         details.presentModes.resize(presentModeCount);
         vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface,
            &presentModeCount, details.presentModes.data());
      }

      return details;
   }

   VkSurfaceFormatKHR chooseSwapSurfaceFormat(const
      std::vector<VkSurfaceFormatKHR>& availableFormats) {
      for (const auto& availableFormat : availableFormats) {
         if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
            availableFormat.colorSpace ==
            VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
         }
      }
      return availableFormats[0];
   }

   VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
      for (const auto& availablePresentMode : availablePresentModes) {
         if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
         }
      }
      return VK_PRESENT_MODE_FIFO_KHR;
   }

   VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
      if (capabilities.currentExtent.width != UINT32_MAX) {
         return capabilities.currentExtent;
      }
      else {
         VkExtent2D actualExtent = { WIDTH, HEIGHT };

         actualExtent.width =
            std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
         actualExtent.height =
            std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

         return actualExtent;
      }
   }

   QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) const {
      QueueFamilyIndices indices;
      uint32_t queueFamilyCount = 0;
      vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount,
         nullptr);
      std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
      vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount,
         queueFamilies.data());

      int i = 0;
      for (const auto& queueFamily : queueFamilies) {
         // Graphics support
         if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
         }

         // Presentation support
         VkBool32 presentSupport = false;
         vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
         if (presentSupport) {
            indices.presentFamily = i;
         }

         if (indices.isComplete()) {
            break;
         }
         i++;
      }
      return indices;
   }

   void createLogicalDevice() {
      QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
      std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
      std::set<uint32_t> uniqueQueueFamilies =
      { indices.graphicsFamily.value(), indices.presentFamily.value() };
      float queuePriority = 1.0f;
      for (uint32_t queueFamily : uniqueQueueFamilies) {
         VkDeviceQueueCreateInfo queueCreateInfo{};
         queueCreateInfo.sType =
            VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
         queueCreateInfo.queueFamilyIndex = queueFamily;
         queueCreateInfo.queueCount = 1;
         queueCreateInfo.pQueuePriorities = &queuePriority;
         queueCreateInfos.push_back(queueCreateInfo);
      }

      VkPhysicalDeviceFeatures deviceFeatures{};
      VkDeviceCreateInfo createInfo{};
      createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
      createInfo.pQueueCreateInfos = queueCreateInfos.data();
      createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
      createInfo.pEnabledFeatures = &deviceFeatures;

      createInfo.enabledExtensionCount =
         static_cast<uint32_t>(deviceExtensions.size());
      createInfo.ppEnabledExtensionNames = deviceExtensions.data();

      if (enableValidationLayers) {
         createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
         createInfo.ppEnabledLayerNames = validationLayers.data();
      }
      else {
         createInfo.enabledLayerCount = 0;
      }
      if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) !=
         VK_SUCCESS) {
         throw std::runtime_error("failed to create logical device!");
      }
      vkGetDeviceQueue(device, indices.presentFamily.value(), 0,
         &presentQueue);
      vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0,
         &graphicsQueue);
   }

   void createSurfaceWin32() {
      VkWin32SurfaceCreateInfoKHR createInfo{};
      createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
      createInfo.hwnd = glfwGetWin32Window(window);
      createInfo.hinstance = GetModuleHandle(nullptr);
      if (vkCreateWin32SurfaceKHR(instance, &createInfo, nullptr, &surface) != VK_SUCCESS) {
         throw std::runtime_error("failed to create window surface!");
      }
   }

   void createSurfaceCommon() {
      VkResult res = glfwCreateWindowSurface(instance, window, nullptr, &surface);
      if (res
         != VK_SUCCESS) {
         throw std::runtime_error("failed to create window surface!");
      }
   }

   void createSurface() {
      createSurfaceCommon();
   }

   void createSwapChain() {
      const SwapChainSupportDetails swapChainSupport =
         querySwapChainSupport(physicalDevice);
      const VkSurfaceFormatKHR surfaceFormat =
         chooseSwapSurfaceFormat(swapChainSupport.formats);
      const VkPresentModeKHR presentMode =
         chooseSwapPresentMode(swapChainSupport.presentModes);
      const VkExtent2D extent =
         chooseSwapExtent(swapChainSupport.capabilities);
      uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
      if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount >
         swapChainSupport.capabilities.maxImageCount) {
         imageCount = swapChainSupport.capabilities.maxImageCount;
      }
      VkSwapchainCreateInfoKHR createInfo{};
      createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
      createInfo.surface = surface;
      createInfo.minImageCount = imageCount;
      createInfo.imageFormat = surfaceFormat.format;
      createInfo.imageColorSpace = surfaceFormat.colorSpace;
      createInfo.imageExtent = extent;
      createInfo.imageArrayLayers = 1;
      createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

      const QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
      const uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(),
      indices.presentFamily.value() };

      if (indices.graphicsFamily != indices.presentFamily) {
         createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
         createInfo.queueFamilyIndexCount = 2;
         createInfo.pQueueFamilyIndices = queueFamilyIndices;

      }
      else {
         createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
         createInfo.queueFamilyIndexCount = 0; // Optional
         createInfo.pQueueFamilyIndices = nullptr; // Optional

      }
      createInfo.preTransform =
         swapChainSupport.capabilities.currentTransform;
      createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
      createInfo.presentMode = presentMode;
      createInfo.clipped = VK_TRUE;
      createInfo.oldSwapchain = VK_NULL_HANDLE;
      if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain)
         != VK_SUCCESS) {
         throw std::runtime_error("failed to create swap chain!");
      }

      swapChainImageFormat = surfaceFormat.format;
      swapChainExtent = extent;
      vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
      swapChainImages.resize(imageCount);
      vkGetSwapchainImagesKHR(device, swapChain, &imageCount,
         swapChainImages.data());
   }

   void createImageView() {
      swapChainImageViews.resize(swapChainImages.size());
      for (std::size_t i = 0; i < swapChainImageViews.size(); ++i) {
         VkImageViewCreateInfo createInfo{};
         createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
         createInfo.image = swapChainImages[i];
         createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
         createInfo.format = swapChainImageFormat;
         createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
         createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
         createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
         createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
         createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
         createInfo.subresourceRange.baseMipLevel = 0;
         createInfo.subresourceRange.levelCount = 1;
         createInfo.subresourceRange.baseArrayLayer = 0;
         createInfo.subresourceRange.layerCount = 1;
         if (vkCreateImageView(device, &createInfo, nullptr,
            &swapChainImageViews[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create image views!");
         }
      }
   }
   void cleanUpImageViews() {
      for (auto imageView : swapChainImageViews) {
         vkDestroyImageView(device, imageView, nullptr);
      }
   }

   //
   // Graphic pipeline
   //

   std::vector<char> readFile(std::string const& fname) const {
      std::ifstream is(fname, std::ios::ate | std::ios::binary);
      if (!is.is_open())
         throw std::exception("Cannot open shader file");
      size_t fileSize = (size_t)is.tellg();
      std::vector<char> buffer(fileSize);
      is.seekg(0);
      is.read(buffer.data(), fileSize);
      is.close();
      return buffer;
      //return std::vector<char>(std::istream_iterator<char>(is), std::istream_iterator<char>());
   }

   void createGraphicPipeline() {
      const auto vertShaderCode = readFile("vert.spv");
      const auto fragShaderCode = readFile("frag.spv");
      VkShaderModule vertShaderModule =
         createShaderModule(vertShaderCode);
      VkShaderModule fragShaderModule =
         createShaderModule(fragShaderCode);

      VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
      vertShaderStageInfo.sType =
         VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
      vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
      vertShaderStageInfo.module = vertShaderModule;
      vertShaderStageInfo.pName = "main";

      VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
      fragShaderStageInfo.sType =
         VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
      fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
      fragShaderStageInfo.module = fragShaderModule;
      fragShaderStageInfo.pName = "main";

      VkPipelineShaderStageCreateInfo shaderStages[] =
      { vertShaderStageInfo, fragShaderStageInfo };

      VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
      vertexInputInfo.sType =
         VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
      vertexInputInfo.vertexBindingDescriptionCount = 0;
      vertexInputInfo.pVertexBindingDescriptions = nullptr; // Optional
      vertexInputInfo.vertexAttributeDescriptionCount = 0;
      vertexInputInfo.pVertexAttributeDescriptions = nullptr; // Optional

      VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
      inputAssembly.sType =
         VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
      inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
      inputAssembly.primitiveRestartEnable = VK_FALSE;

      VkViewport viewport{};
      viewport.x = 0.0f;
      viewport.y = 0.0f;
      viewport.width = (float)swapChainExtent.width;
      viewport.height = (float)swapChainExtent.height;
      viewport.minDepth = 0.0f;
      viewport.maxDepth = 1.0f;

      VkRect2D scissor{};
      scissor.offset = { 0, 0 };
      scissor.extent = swapChainExtent;

      VkPipelineViewportStateCreateInfo viewportState{};
      viewportState.sType =
         VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
      viewportState.viewportCount = 1;
      viewportState.pViewports = &viewport;
      viewportState.scissorCount = 1;
      viewportState.pScissors = &scissor;

      VkPipelineRasterizationStateCreateInfo rasterizer{};
      rasterizer.sType =
         VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
      rasterizer.depthClampEnable = VK_FALSE;
      rasterizer.rasterizerDiscardEnable = VK_FALSE;
      rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
      rasterizer.lineWidth = 1.0f;
      rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
      rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
      rasterizer.depthBiasEnable = VK_FALSE;
      rasterizer.depthBiasConstantFactor = 0.0f; // Optional
      rasterizer.depthBiasClamp = 0.0f; // Optional
      rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

      VkPipelineMultisampleStateCreateInfo multisampling{};
      multisampling.sType =
         VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
      multisampling.sampleShadingEnable = VK_FALSE;
      multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
      multisampling.minSampleShading = 1.0f; // Optional
      multisampling.pSampleMask = nullptr; // Optional
      multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
      multisampling.alphaToOneEnable = VK_FALSE; // Optional

      VkPipelineColorBlendAttachmentState colorBlendAttachment{};
      colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
         VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
         VK_COLOR_COMPONENT_A_BIT;
      colorBlendAttachment.blendEnable = VK_FALSE;
      colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; //  Optional
      colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; //Optional
      colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
      colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
      colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; //Optional
      colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

      //colorBlendAttachment.blendEnable = VK_TRUE;
      //colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
      //colorBlendAttachment.dstColorBlendFactor =
      // VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
      //colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
      //colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
      //colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
      //colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

      VkPipelineColorBlendStateCreateInfo colorBlending{};
      colorBlending.sType =
         VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
      colorBlending.logicOpEnable = VK_FALSE;
      colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
      colorBlending.attachmentCount = 1;
      colorBlending.pAttachments = &colorBlendAttachment;
      colorBlending.blendConstants[0] = 0.0f; // Optional
      colorBlending.blendConstants[1] = 0.0f; // Optional
      colorBlending.blendConstants[2] = 0.0f; // Optional
      colorBlending.blendConstants[3] = 0.0f; // Optional


      VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
      pipelineLayoutInfo.sType =
         VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
      pipelineLayoutInfo.setLayoutCount = 0; // Optional
      pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
      pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
      pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional
      if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr,
         &pipelineLayout) != VK_SUCCESS) {
         throw std::runtime_error("failed to create pipeline layout!");

      }




      VkGraphicsPipelineCreateInfo pipelineInfo{};
      pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
      pipelineInfo.stageCount = 2;
      pipelineInfo.pStages = shaderStages;
      pipelineInfo.pVertexInputState = &vertexInputInfo;
      pipelineInfo.pInputAssemblyState = &inputAssembly;
      pipelineInfo.pViewportState = &viewportState;
      pipelineInfo.pRasterizationState = &rasterizer;
      pipelineInfo.pMultisampleState = &multisampling;
      pipelineInfo.pDepthStencilState = nullptr; // Optional
      pipelineInfo.pColorBlendState = &colorBlending;
      pipelineInfo.pDynamicState = nullptr; // Optional
      pipelineInfo.layout = pipelineLayout;
      pipelineInfo.renderPass = renderPass;
      pipelineInfo.subpass = 0;
      pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
      pipelineInfo.basePipelineIndex = -1; // Optional


      if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1,
         &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
         throw std::runtime_error("failed to create graphics pipeline!");
      }

      vkDestroyShaderModule(device, fragShaderModule, nullptr);
      vkDestroyShaderModule(device, vertShaderModule, nullptr);
   }

   void createRenderPass() {
      VkAttachmentDescription colorAttachment{};
      colorAttachment.format = swapChainImageFormat;
      colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
      colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
      colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
      colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
      colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

      VkAttachmentReference colorAttachmentRef{};
      colorAttachmentRef.attachment = 0;
      colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

      VkSubpassDescription subpass{};
      subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
      subpass.colorAttachmentCount = 1;
      subpass.pColorAttachments = &colorAttachmentRef;


      VkSubpassDependency dependency{};
      dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
      dependency.dstSubpass = 0;
      dependency.srcStageMask =
         VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
      dependency.srcAccessMask = 0;
      dependency.dstStageMask =
         VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
      dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

      VkRenderPassCreateInfo renderPassInfo{};
      renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
      renderPassInfo.attachmentCount = 1;
      renderPassInfo.pAttachments = &colorAttachment;
      renderPassInfo.subpassCount = 1;
      renderPassInfo.pSubpasses = &subpass;
      renderPassInfo.dependencyCount = 1;
      renderPassInfo.pDependencies = &dependency;
      
      if (vkCreateRenderPass(device, &renderPassInfo, nullptr,
         &renderPass) != VK_SUCCESS) {
         throw std::runtime_error("failed to create render pass!");
      }
   }

   [[nodiscard]] VkShaderModule createShaderModule(const std::vector<char>& code) const {
      VkShaderModuleCreateInfo createInfo{};
      createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
      createInfo.codeSize = code.size();
      createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
      VkShaderModule shaderModule;
      if (vkCreateShaderModule(device, &createInfo, nullptr,
         &shaderModule) != VK_SUCCESS) {
         throw std::runtime_error("failed to create shader module!");
      }
      return shaderModule;
   }

   void createFramebuffers() {
      swapChainFramebuffers.resize(swapChainImageViews.size());
      for (size_t i = 0; i < swapChainImageViews.size(); i++) {
         const VkImageView attachments[] = {
         swapChainImageViews[i]
         };

         VkFramebufferCreateInfo framebufferInfo{};
         framebufferInfo.sType =
            VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
         framebufferInfo.renderPass = renderPass;
         framebufferInfo.attachmentCount = 1;
         framebufferInfo.pAttachments = attachments;
         framebufferInfo.width = swapChainExtent.width;
         framebufferInfo.height = swapChainExtent.height;
         framebufferInfo.layers = 1;

         if (vkCreateFramebuffer(device, &framebufferInfo, nullptr,
            &swapChainFramebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer!");
         }
      }
   }

   void createCommandPool() {
      QueueFamilyIndices queueFamilyIndices =
         findQueueFamilies(physicalDevice);
      VkCommandPoolCreateInfo poolInfo{};
      poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
      poolInfo.queueFamilyIndex =
         queueFamilyIndices.graphicsFamily.value();
      poolInfo.flags = 0; // Optional
      if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) !=
         VK_SUCCESS) {
         throw std::runtime_error("failed to create command pool!");
      }
   }

   void createCommandBuffers() {
      commandBuffers.resize(swapChainFramebuffers.size());
      VkCommandBufferAllocateInfo allocInfo{};
      allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
      allocInfo.commandPool = commandPool;
      allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
      allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

      if (vkAllocateCommandBuffers(device, &allocInfo,
         commandBuffers.data()) != VK_SUCCESS) {
         throw std::runtime_error("failed to allocate command buffers!");
      }
      for (size_t i = 0; i < commandBuffers.size(); i++) {
         VkCommandBufferBeginInfo beginInfo{};
         beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
         beginInfo.flags = 0; // Optional
         beginInfo.pInheritanceInfo = nullptr; // Optional

         if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) !=
            VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
         }

         VkRenderPassBeginInfo renderPassInfo{};
         renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
         renderPassInfo.renderPass = renderPass;
         renderPassInfo.framebuffer = swapChainFramebuffers[i];
         renderPassInfo.renderArea.offset = { 0, 0 };
         renderPassInfo.renderArea.extent = swapChainExtent;

         VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
         renderPassInfo.clearValueCount = 1;
         renderPassInfo.pClearValues = &clearColor;

         vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo,
            VK_SUBPASS_CONTENTS_INLINE);
         vkCmdBindPipeline(commandBuffers[i],
            VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
         vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);
         vkCmdEndRenderPass(commandBuffers[i]);
         if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
         }
      }
   }

   void createSyncObjects() {
      imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
      renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
      inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
      imagesInFlight.resize(swapChainImages.size(), VK_NULL_HANDLE);

      VkSemaphoreCreateInfo semaphoreInfo{};
      semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

      VkFenceCreateInfo fenceInfo{};
      fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
      fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

      for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
         if (vkCreateSemaphore(device, &semaphoreInfo, nullptr,
               &imageAvailableSemaphores[i]) != VK_SUCCESS 
            || vkCreateSemaphore(device, &semaphoreInfo, nullptr,
               &renderFinishedSemaphores[i]) != VK_SUCCESS
            || vkCreateFence(device, &fenceInfo, nullptr,
               &inFlightFences[i]) != VK_SUCCESS) {

            throw std::runtime_error("failed to create semaphores for a frame!");
         }
      }
   }

   void initVulkan() {
      glfwInit();
      createInstance();
      if constexpr (enableValidationLayers) setupDebugMessenger();
      initWindow();
      createSurface();
      pickPhysicalDevice();
      createLogicalDevice();
      createSwapChain();
      createImageView();
      createRenderPass();
      createGraphicPipeline();
      createFramebuffers();
      createCommandPool();
      createCommandBuffers();
      createSyncObjects();
   }

   void initWindow() {
      glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
      glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

      window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr,
         nullptr);
   }

   void drawFrame() {
      vkWaitForFences(device, 1, &inFlightFences[currentFrame],
         VK_TRUE, UINT64_MAX);
      
      uint32_t imageIndex;
      vkAcquireNextImageKHR(device, swapChain, UINT64_MAX,
         imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

      // Check if a previous frame is using this image (i.e. there is its fence to wait on)
      if (imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
         vkWaitForFences(device, 1, &imagesInFlight[imageIndex],
            VK_TRUE, UINT64_MAX);
      }
      // Mark the image as now being in use by this frame
      imagesInFlight[imageIndex] = inFlightFences[currentFrame];

      VkSubmitInfo submitInfo{};
      submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

      VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
      VkPipelineStageFlags waitStages[] =
      { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
      submitInfo.waitSemaphoreCount = 1;
      submitInfo.pWaitSemaphores = waitSemaphores;
      submitInfo.pWaitDstStageMask = waitStages;
      submitInfo.commandBufferCount = 1;
      submitInfo.pCommandBuffers = &commandBuffers[imageIndex];
      VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
      submitInfo.signalSemaphoreCount = 1;
      submitInfo.pSignalSemaphores = signalSemaphores;

      vkResetFences(device, 1, &inFlightFences[currentFrame]);

      if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) !=
         VK_SUCCESS) {
         throw std::runtime_error("failed to submit draw command buffer!");
      }

      VkPresentInfoKHR presentInfo{};
      presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
      presentInfo.waitSemaphoreCount = 1;
      presentInfo.pWaitSemaphores = signalSemaphores;
      VkSwapchainKHR swapChains[] = { swapChain };
      presentInfo.swapchainCount = 1;
      presentInfo.pSwapchains = swapChains;
      presentInfo.pImageIndices = &imageIndex;
      presentInfo.pResults = nullptr; // Optional
      vkQueuePresentKHR(presentQueue, &presentInfo);
      //vkQueueWaitIdle(presentQueue);
      currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
   }

   void mainLoop()  {
      while (!glfwWindowShouldClose(window)) {
         glfwPollEvents();
         drawFrame();
      }
      //vkDeviceWaitIdle(device);
   }

   void cleanup() {
      for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
         vkDestroySemaphore(device, renderFinishedSemaphores[i],
            nullptr);
         vkDestroySemaphore(device, imageAvailableSemaphores[i],
            nullptr);
         vkDestroyFence(device, inFlightFences[i], nullptr);
      }
      vkDestroyCommandPool(device, commandPool, nullptr);
      for (const auto framebuffer : swapChainFramebuffers) {
         vkDestroyFramebuffer(device, framebuffer, nullptr);
      }
      vkDestroyPipeline(device, graphicsPipeline, nullptr);
      vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
      vkDestroyRenderPass(device, renderPass, nullptr);
      cleanUpImageViews();
      vkDestroySwapchainKHR(device, swapChain, nullptr);
      vkDestroyDevice(device, nullptr);
      if constexpr (enableValidationLayers) {
         DestroyDebugUtilsMessengerEXT(instance, debugMessenger,
            nullptr);
      }
      vkDestroySurfaceKHR(instance, surface, nullptr);
      vkDestroyInstance(instance, nullptr);
      glfwDestroyWindow(window);
      glfwTerminate();
   }
};

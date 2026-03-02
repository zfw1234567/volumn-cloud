#include <vulkanContext/vulkanContext.h>

vulkanContext::vulkanContext(/* args */)
{
    initWindow();
    createInstance();
    createDebugMessenger();
    createWindowsSurface();
    selectPhysicalDevice();     
    createLogicalDevice();   
    createSwapChain();   
    createImageViews();
    this->depthImage.setContext(this->device,this->physicalDevice,this->swapChainExtent);
    this->depthImage.createDepthResources();
}

vulkanContext::~vulkanContext()
{
    std::cout<<"____"<<std::endl;
    std::cout<<"开始销毁：vulkanContext"<<std::endl;
    // commandBuffer不需要释放
    vkDeviceWaitIdle(device);
    cleanupSwapChain();
    //this->commandpool->~vulkanCommandPool();
    //this->backgroundPipeline->~vulkanGraphicsPipeline();
    //this->objectPipeline->~vulkanGraphicsPipeline();
    //this->renderPass->~vulkanRenderPass();
    //this->background->~vulkanRenderBackgroundObject();
    // for(int i=0;i<this->stageObject.size();i++)
    // {
    //     this->stageObject[i].~vulkanRenderObject();
    // }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(device, inFlightFences[i], nullptr);
    }

    if (enableValidationLayers)
    {
        destroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    }
    if(this->compositeDescriptorPool)
    {
        vkDestroyDescriptorPool(device, compositeDescriptorPool, nullptr);
    }
    vkDestroyDevice(device, nullptr);
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);
    std::cout<<"销毁结束"<<std::endl;
    std::cout<<"____"<<std::endl;
}

void vulkanContext::destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        func(instance, debugMessenger, pAllocator);
    }
}

/** 初始化GUI渲染窗口*/
void vulkanContext::initWindow()
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    window = glfwCreateWindow(WIDTH, HEIGHT, "volumn cloud", nullptr, nullptr);
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    
    // 隐藏光标并捕获鼠标输入
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    // 设置鼠标回调
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);

    glfwSetKeyCallback(window, keyCallback);

    camera.Position = glm::vec3(0.0f, 0.0f, 100.0f);
}

void vulkanContext::framebufferResizeCallback(GLFWwindow *window, int width, int height)
{
    auto app = reinterpret_cast<vulkanContext *>(glfwGetWindowUserPointer(window));
    app->framebufferResized = true;
}

/** 创建程序和Vulkan之间的连接，涉及程序和显卡驱动之间特殊细节*/
void vulkanContext::createInstance()
{
    if (enableValidationLayers && !checkValidationLayerSupport())
    {
        throw std::runtime_error("validation layers requested, but not available!");
    }

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Render the Scene";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "Vulkan Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
#ifdef __APPLE__
    createInfo.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif
    // 获取需要的glfw拓展名
    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (enableValidationLayers)
    {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
#ifdef __APPLE__
    // Fix issue on Mac(m2) "vkCreateInstance: Found no drivers!"
    extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    // Issue on Mac(m2), it's not a error, but a warning, ignore it by this time~
    // vkCreateDevice():  VK_KHR_portability_subset must be enabled because physical device VkPhysicalDevice 0x600003764f40[] supports it. The Vulkan spec states: If the VK_KHR_portability_subset extension is included in pProperties of vkEnumerateDeviceExtensionProperties, ppEnabledExtensionNames must include "VK_KHR_portability_subset"
    extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
#endif
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if (enableValidationLayers)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();

        populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo;
    }
    else
    {
        createInfo.enabledLayerCount = 0;

        createInfo.pNext = nullptr;
    }

    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create instance!");
    }
}

void vulkanContext::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
    {
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
    }

bool vulkanContext::checkValidationLayerSupport()
    {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const char* layerName : validationLayers)
        {
            bool layerFound = false;

            for (const auto& layerProperties : availableLayers)
            {
                if (strcmp(layerName, layerProperties.layerName) == 0)
                {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound)
            {
                return false;
            }
        }

        return true;
    }

/** 打印调试信息时的回调函数，可以用来处理调试信息*/
VKAPI_ATTR VkBool32 VKAPI_CALL vulkanContext::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData)
{
    std::cerr << "[LOG]: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}

/** 合法性监测层 Validation Layers
 *	- 检查参数规范，检测是否使用
 *	- 最终对象创建和销毁，找到资源泄漏
 *	- 通过追踪线程原始调用，检查线程安全性
 *	- 打印输出每次调用
 *	- 为优化和重现追踪Vulkan调用
 */
void vulkanContext::createDebugMessenger()
{
    if (!enableValidationLayers)
        return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    populateDebugMessengerCreateInfo(createInfo);

    if (createDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to set up debug messenger!");
    }
}

VkResult vulkanContext::createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void vulkanContext::createWindowsSurface()
{
    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create window surface!");
    }
}

/** 选择支持Vulkan的显卡硬件*/
void vulkanContext::selectPhysicalDevice()
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (deviceCount == 0)
    {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    for (const auto &device : devices)
    {
        if (isDeviceSuitable(device))
        {
            physicalDevice = device;
            break;
        }
    }

    if (physicalDevice == VK_NULL_HANDLE)
    {
        throw std::runtime_error("failed to find a suitable GPU!");
    }
}

/** 检测硬件是否合适*/
bool vulkanContext::isDeviceSuitable(VkPhysicalDevice device)
{
    QueueFamilyIndices queue_family_indices = findQueueFamilies(device);

    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for (const auto &extension : availableExtensions)
    {
        requiredExtensions.erase(extension.extensionName);
    }

    bool extensionsSupported = requiredExtensions.empty();

    bool swapChainAdequate = false;
    if (extensionsSupported)
    {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

    return queue_family_indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
}

/** 队列家族 Queue Family
 * 找到所有支持Vulkan的显卡硬件
 */
QueueFamilyIndices vulkanContext::findQueueFamilies(VkPhysicalDevice device)
{
    QueueFamilyIndices indices;
    
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
    
    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
        // 图形队列
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }
        
        // 计算队列
        if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT) {
            indices.computeFamily = i;
        }
        
        // 查找呈现队列
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
        if (presentSupport) {
            indices.presentFamily = i;
        }
        
        i++;
    }
    
    if (!indices.computeFamily.has_value() && indices.graphicsFamily.has_value()) {
        indices.computeFamily = indices.graphicsFamily;
    }
    
    return indices;
}

SwapChainSupportDetails vulkanContext::querySwapChainSupport(VkPhysicalDevice device)
{
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

    if (formatCount != 0)
    {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

    if (presentModeCount != 0)
    {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

void vulkanContext::createLogicalDevice()
{
    QueueFamilyIndices queue_family_indices = findQueueFamilies(physicalDevice);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {queue_family_indices.graphicsFamily.value(), queue_family_indices.presentFamily.value(),queue_family_indices.computeFamily.value()};

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    createInfo.pEnabledFeatures = &deviceFeatures;

    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    if (enableValidationLayers)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else
    {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create logical device!");
    }

    vkGetDeviceQueue(device, queue_family_indices.graphicsFamily.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(device, queue_family_indices.presentFamily.value(), 0, &presentQueue);
    vkGetDeviceQueue(device, queue_family_indices.computeFamily.value(), 0, &computeQueue);
}

/** 交换链 Swap Chain
 * Vulkan一种基础结构，持有帧缓存FrameBuffer
 * SwapChain持有显示到窗口的图像队列
 * 通常Vulkan获取图像，渲染到图像上，然后将图像推入SwapChain的图像队列
 * SwapChain显示图像，通常和屏幕刷新率保持同步
 */
void vulkanContext::createSwapChain()
{
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
    {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }
imageCount =2;
    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;

    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices queue_family_indices = findQueueFamilies(physicalDevice);
    uint32_t queueFamilyIndices[] = {queue_family_indices.graphicsFamily.value(), queue_family_indices.presentFamily.value()};

    if (queue_family_indices.graphicsFamily != queue_family_indices.presentFamily)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create swap chain!");
    }

    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
    swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;
}

/** 创建帧缓存，即每帧图像对应的渲染数据*/
void vulkanContext::createFramebuffers()
{
    swapChainFramebuffers.resize(swapChainImageViews.size());

    for (size_t i = 0; i < swapChainImageViews.size(); i++)
    {
        std::array<VkImageView, 2> attachments = {
            swapChainImageViews[i],
            this->depthImage.depthImageView};

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = this->renderPass->renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = swapChainExtent.width;
        framebufferInfo.height = swapChainExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
}

/** 选择SwapChain渲染到视图的图像的格式*/
VkSurfaceFormatKHR vulkanContext::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats)
{
    // 找到合适的图像格式
    // VK_FORMAT_B8G8R8A8_SRGB 将图像BGRA存储在 unsigned normalized format 中，使用SRGB 非线性编码，颜色空间为非线性空间，不用Gamma矫正最终结果
    // VK_FORMAT_B8G8R8A8_UNORM 将图像BGRA存储在 unsigned normalized format 中，颜色空间为线性空间，像素的最终输出颜色需要Gamma矫正
    for (const auto &availableFormat : availableFormats)
    {
        // 将 FrameBuffer Image 设置为线性空间，方便 PBR 的工作流以及颜色矫正（ColorCorrection）
        // if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM)
        {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

/** 选择SwapChain的显示方式
 * VK_PRESENT_MODE_IMMEDIATE_KHR 图形立即显示在屏幕上，会出现图像撕裂
 * VK_PRESENT_MODE_FIFO_KHR 图像会被推入一个队列，先入后出显示到屏幕，如果队列满了，程序会等待，和垂直同步相似
 * VK_PRESENT_MODE_FIFO_RELAXED_KHR 基于第二个Mode，当队列满了，程序不会等待，而是直接渲染到屏幕，会出现图像撕裂
 * VK_PRESENT_MODE_MAILBOX_KHR 基于第二个Mode，当队列满了，程序不会等待，而是直接替换队列中的图像，
 */
VkPresentModeKHR vulkanContext::chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes)
{
    for (const auto &availablePresentMode : availablePresentModes)
    {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D vulkanContext::chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities)
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        return capabilities.currentExtent;
    }
    else
    {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)};

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

/** 图像视图 Image View
 * 将视图显示为图像
 * ImageView定义了SwapChain里定义的图像是什么样的
 * 比如，带深度信息的RGB图像
 */
void vulkanContext::createImageViews()
{
    swapChainImageViews.resize(swapChainImages.size());

    for (size_t i = 0; i < swapChainImages.size(); i++)
    {
        createImageView(swapChainImageViews[i], swapChainImages[i], swapChainImageFormat);
    }
}

/** 创建图像视口*/
void vulkanContext::createImageView(VkImageView &outImageView, const VkImage &inImage, const VkFormat &inFormat, VkImageAspectFlags aspectFlags, uint32_t levelCount)
{
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = inImage;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = inFormat;
    viewInfo.subresourceRange.aspectMask = aspectFlags; // VK_IMAGE_ASPECT_COLOR_BIT 颜色 VK_IMAGE_ASPECT_DEPTH_BIT 深度
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = levelCount;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(device, &viewInfo, nullptr, &outImageView) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create texture image view!");
    }
}

/** 清理旧的SwapChain*/
void vulkanContext::cleanupSwapChain()
{
    this->depthImage.cleanup();

    for (auto framebuffer : swapChainFramebuffers)
    {
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    }

    vkDestroyFramebuffer(device, halfRescloudFramebufferA, nullptr);
    vkDestroyFramebuffer(device, halfRescloudFramebufferB, nullptr);
    for (auto imageView : swapChainImageViews)
    {
        vkDestroyImageView(device, imageView, nullptr);
    }

    vkDestroySwapchainKHR(device, swapChain, nullptr);
}

    void vulkanContext::recreateSwapChain()
    {
        // 当窗口长宽都是零时，说明窗口被最小化了，这时需要等待
        int width = 0, height = 0;
        glfwGetFramebufferSize(window, &width, &height);
        while (width == 0 || height == 0) {
            glfwGetFramebufferSize(window, &width, &height);
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(device);

        cleanupSwapChain();

        createSwapChain();
        createImageViews();
        this->depthImage.setContext(this->device, this->physicalDevice, this->swapChainExtent);
        this->depthImage.createDepthResources();
        createFramebuffers();
    }

/** 创建同步物体，同步显示当前渲染*/
void vulkanContext::createSyncObjects()
{
    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)
        {

            throw std::runtime_error("failed to create synchronization objects for a frame!");
        }
    }
}

void vulkanContext::mainTick(const vulkanUniformBuffer &meshUniformBuffer, const vulkanUniformBuffer &viewUniformBuffer, const vulkancubemaperesource &cubemap)
{
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        totalTime+=deltaTime;

        drawFrame(meshUniformBuffer,viewUniformBuffer,cubemap); // 绘制一帧
        this->framenumber=((this->framenumber+1)%this->MAXFRAMECOUNT);
    }

    vkDeviceWaitIdle(device);
}

void vulkanContext::drawFrame(const vulkanUniformBuffer &meshUniformBuffer, const vulkanUniformBuffer &viewUniformBuffer, const vulkancubemaperesource &cubemap)
    {
        // 等待上一帧绘制完成
        vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

        // 当窗口过期时（窗口尺寸改变或者窗口最小化后又重新显示），需要重新创建SwapChain并且停止这一帧的绘制
        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            recreateSwapChain();
            return;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        // 更新统一缓存区（UBO）
        updateUniformBuffer(currentFrame,meshUniformBuffer,viewUniformBuffer,cubemap);

        vkResetFences(device, 1, &inFlightFences[currentFrame]);
        // 清除渲染指令缓存
        vkResetCommandBuffer(this->commandpool->commandBuffers[currentFrame], /*VkCommandBufferResetFlagBits*/ 0);

        // 记录新的所有的渲染指令缓存
        recordCommandBuffer(this->renderPass->renderPass, this->commandpool->commandBuffers[currentFrame], imageIndex);
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &this->commandpool->commandBuffers[currentFrame];

        VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        // 提交渲染指令
        if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
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

        vkQueuePresentKHR(presentQueue, &presentInfo);

        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    /** 更新统一缓存区（UBO）*/
    void vulkanContext::updateUniformBuffer(uint32_t currentImage,const vulkanUniformBuffer &meshUniformBuffer, const vulkanUniformBuffer &viewUniformBuffer, const vulkancubemaperesource &cubemap)
    {
        this->updateCameraPosition();
        static auto startTime = std::chrono::high_resolution_clock::now();

        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
        global.time = time;

        glm::mat4 normalize = glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        UniformBufferObject ubo{};
        ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.proj = this->camera.getProjectionMatrix(swapChainExtent.width / (float)swapChainExtent.height);
        ubo.proj[1][1] *= -1;

        void* data_mesh;
        vkMapMemory(device, meshUniformBuffer.UniformBuffersMemory[currentImage], 0, sizeof(ubo), 0, &data_mesh);
        memcpy(data_mesh, &ubo, sizeof(ubo));
        vkUnmapMemory(device, meshUniformBuffer.UniformBuffersMemory[currentImage]);

        float rotationAngle = time * glm::radians(50.0f);

        // 创建绕Z轴的旋转矩阵
        glm::mat4 rotationMatrix = glm::rotate(glm::mat4(40.0f), rotationAngle, glm::vec3(0.0f, 1.0f, 0.0f));

        glm::vec4 initialLightPos = glm::vec4(500.0f, 500.0f, 0.0f, 0.0f);

        UniformBufferObjectView ubv{};
        Light light;
        light.position = rotationMatrix * initialLightPos;
        light.color = glm::vec4(1.0, 1.0, 1.0, 3.0);
        light.direction = glm::vec4(-2.0, 0.0, -2.0, 0.0);
        light.info = glm::vec4(0.0, 0.0, 0.0, 0.0);
        ubv.directional_lights[0] = light;
        ubv.lights_count = glm::ivec4(1, 0, 0, cubemap.cubemapMaxMips);
        ubv.camera_position =glm::vec4( this->camera.Position,1.0f);

        void* data_view;
        vkMapMemory(device, viewUniformBuffer.UniformBuffersMemory[currentImage], 0, sizeof(ubv), 0, &data_view);
        memcpy(data_view, &ubv, sizeof(ubv));
        vkUnmapMemory(device, viewUniformBuffer.UniformBuffersMemory[currentImage]);

        UniformBufferVolumeCloud volumecloudubo{};
        glm::mat4 S = glm::scale(glm::mat4(1.0f), this->volumecloud->bounding.size);                       // 缩放
        glm::mat4 R = glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f)); // 旋转
        glm::mat4 T = glm::translate(glm::mat4(1.0f), this->volumecloud->bounding.center);
        volumecloudubo.model=T * R * S;
        
        volumecloudubo.view = this->camera.getViewMatrix();
        volumecloudubo.proj = this->camera.getProjectionMatrix(swapChainExtent.width / (float)swapChainExtent.height);
        volumecloudubo.proj[1][1] *= -1;
        volumecloudubo.center=glm::vec4(this->volumecloud->bounding.center,0.0);
        volumecloudubo.size=glm::vec4(this->volumecloud->bounding.size,0.0);
        volumecloudubo.parameter=glm::vec4(1.0,1.0,1.0,this->totalTime*1.0);
        volumecloudubo.cameranear=camera.nearPlane;
        volumecloudubo.camerafar=camera.farPlane;
        void *data_vcloud;
        vkMapMemory(device, this->volumeclouduniform->UniformBuffersMemory[currentImage], 0, sizeof(volumecloudubo), 0, &data_vcloud);
        memcpy(data_vcloud, &volumecloudubo, sizeof(volumecloudubo));
        vkUnmapMemory(device, this->volumeclouduniform->UniformBuffersMemory[currentImage]);

        UniformBufferObject skyubo{};
        skyubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 1.0f, 0.0f));
        skyubo.view = this->camera.getSkyboxViewMatrix();
        skyubo.proj = this->camera.getProjectionMatrix(swapChainExtent.width / (float)swapChainExtent.height);
        skyubo.proj[1][1] *= -1;

        void* data_sky;
        vkMapMemory(device, this->skyUniform->UniformBuffersMemory[currentImage], 0, sizeof(skyubo), 0, &data_sky);
        memcpy(data_sky, &skyubo, sizeof(skyubo));
        vkUnmapMemory(device, this->skyUniform->UniformBuffersMemory[currentImage]);
    }
    
    void vulkanContext::recordCommandBuffer(VkRenderPass renderPass, VkCommandBuffer commandBuffer, uint32_t imageIndex)
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        // 开始记录指令
        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        //等待计算管线算完
        VkImageMemoryBarrier renderBarrier{};
        renderBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        renderBarrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        renderBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; // 布局不变
        renderBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        renderBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        renderBarrier.image = this->volumecloud->object.volumeImage;
        renderBarrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
        renderBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT; // 计算着色器的写入
        renderBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;  // 片段着色器的读取

        vkCmdPipelineBarrier(commandBuffer,
                             VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,  // 等待计算完成
                             VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, // 片段着色器可以读取
                             0, 0, nullptr, 0, nullptr, 1, &renderBarrier);
        

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPass;
        renderPassInfo.framebuffer = this->renderPass->Framebuffer;
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = swapChainExtent;

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
        clearValues[1].depthStencil = { 1.0f, 0 };

        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        // 开始RenderPass
        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        // 渲染视口信息
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float)swapChainExtent.width;
        viewport.height = (float)swapChainExtent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        // 视口剪切信息
        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = swapChainExtent;

        // 设置渲染视口
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        // 设置视口剪切，是否可以通过这个函数来实现 Tiled-Based Rendering？
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->skyboxPipeline->graphicsPipeline);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->skyboxPipeline->pipelineLayout, 0, 1, &this->skybox->descriptorSets[currentFrame], 0, nullptr);
        vkCmdDraw(commandBuffer, 36, 1, 0, 0);
        // if (this->background != nullptr)
        // {
        //     // 渲染背景面片
        //     vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->backgroundPipeline->graphicsPipeline);
        //     vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->backgroundPipeline->pipelineLayout, 0, 1, &this->background->object.descriptorSets[currentFrame], 0, nullptr);
        //     vkCmdPushConstants(commandBuffer, this->backgroundPipeline->pipelineLayout, VK_SHADER_STAGE_ALL, 0, sizeof(GlobalConstants), &global);
        //     vkCmdDraw(commandBuffer, 6, 1, 0, 0);

        // }

        // // // 渲染场景
        // for (size_t i = 0; i < stageObject.size(); i++)
        // {
        //     vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->objectPipeline->graphicsPipeline);
        //     VkBuffer objectVertexBuffers[] = { stageObject[i]->object.vertexBuffer};
        //     VkDeviceSize objectOffsets[] = { 0 };
        //     vkCmdBindVertexBuffers(commandBuffer, 0, 1, objectVertexBuffers, objectOffsets);
        //     vkCmdBindIndexBuffer(commandBuffer, stageObject[i]->object.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
        //     vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->objectPipeline->pipelineLayout, 0, 1, &stageObject[i]->object.descriptorSets[currentFrame], 0, nullptr);
        //     vkCmdPushConstants(commandBuffer, backgroundPipeline->pipelineLayout, VK_SHADER_STAGE_ALL, 0, sizeof(GlobalConstants), &global);
        //     vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(stageObject[i]->object.indices.size()), 1, 0, 0, 0);
        // }

        // 结束RenderPass
        vkCmdEndRenderPass(commandBuffer);

        VkImageMemoryBarrier skyboxBarrier{};
        skyboxBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        skyboxBarrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        skyboxBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        skyboxBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        skyboxBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        skyboxBarrier.image = this->renderPass->Image[0].Image; 
        skyboxBarrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
        skyboxBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        skyboxBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        
        vkCmdPipelineBarrier(commandBuffer,
                            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                            0, 0, nullptr, 0, nullptr, 1, &skyboxBarrier);

        VkViewport halfViewport{};
        halfViewport.x = 0.0f;
        halfViewport.y = 0.0f;
        halfViewport.width = (float)halfResPass->extent.width;
        halfViewport.height = (float)halfResPass->extent.height;
        halfViewport.minDepth = 0.0f;
        halfViewport.maxDepth = 1.0f;

        VkRect2D halfScissor{};
        halfScissor.offset = { 0, 0 };
        halfScissor.extent = halfResPass->extent;

        vkCmdSetViewport(commandBuffer, 0, 1, &halfViewport);
        vkCmdSetScissor(commandBuffer, 0, 1, &halfScissor);

        //体积云降采样pass
        VkRenderPassBeginInfo vcrenderPassInfo{};
        vcrenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        vcrenderPassInfo.renderPass = halfResPass->renderPass;
        if(this->framenumber%2==1) //乒乓交换
        {
            vcrenderPassInfo.framebuffer = halfRescloudFramebufferA;

        }
        else
        {
            vcrenderPassInfo.framebuffer = halfRescloudFramebufferB;
        }
        vcrenderPassInfo.renderArea.offset = { 0, 0 };
        vcrenderPassInfo.renderArea.extent = halfResPass->extent;

        std::array<VkClearValue, 3> vc_clearValues{};
        vc_clearValues[0].color = { {0.9f, 0.9f, 0.9f, 0.0f} };
        vc_clearValues[1].color = { {0.0f, 0.0f, 0.0f, 0.0f} };
        vc_clearValues[2].depthStencil = { 1.0f, 0 };

        vcrenderPassInfo.clearValueCount = static_cast<uint32_t>(vc_clearValues.size());
        vcrenderPassInfo.pClearValues = vc_clearValues.data();

        vkCmdBeginRenderPass(commandBuffer, &vcrenderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->volumeCloudPipeline->graphicsPipeline);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->volumeCloudPipeline->pipelineLayout, 0, 1, &volumecloud->object.descriptorSets[currentFrame], 0, nullptr);
        vkCmdDraw(commandBuffer, 36, 1, 0, 0);

        vkCmdEndRenderPass(commandBuffer);

        {
            VkImageMemoryBarrier cloudBarrier{};
            cloudBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            cloudBarrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            cloudBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            cloudBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            cloudBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            cloudBarrier.image = halfResPass->Image[0].Image; // 降采样后的云纹理
            cloudBarrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
            cloudBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            cloudBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            vkCmdPipelineBarrier(commandBuffer,
                                 VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                 VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                 0, 0, nullptr, 0, nullptr, 1, &cloudBarrier);
        }

        {
            VkImageMemoryBarrier cloudBarrier{};
            cloudBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            cloudBarrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            cloudBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            cloudBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            cloudBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            cloudBarrier.image = halfResPass->Image[1].Image; // 降采样后的云纹理
            cloudBarrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
            cloudBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            cloudBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            vkCmdPipelineBarrier(commandBuffer,
                                 VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                 VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                 0, 0, nullptr, 0, nullptr, 1, &cloudBarrier);
        }

        {
            VkImageMemoryBarrier cloudBarrier{};
            cloudBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            cloudBarrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            cloudBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            cloudBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            cloudBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            cloudBarrier.image = halfResPass->Image[2].Image; // 降采样后的云纹理
            cloudBarrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
            cloudBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            cloudBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            vkCmdPipelineBarrier(commandBuffer,
                                 VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                 VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                 0, 0, nullptr, 0, nullptr, 1, &cloudBarrier);
        }

        {
            VkImageMemoryBarrier cloudBarrier{};
            cloudBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            cloudBarrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            cloudBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            cloudBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            cloudBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            cloudBarrier.image = halfResPass->Image[3].Image; // 降采样后的云纹理
            cloudBarrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
            cloudBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            cloudBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            vkCmdPipelineBarrier(commandBuffer,
                                 VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                 VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                 0, 0, nullptr, 0, nullptr, 1, &cloudBarrier);
        }

        {
            VkRenderPassBeginInfo compositePassInfo{};
            compositePassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            compositePassInfo.renderPass = compositeRenderPass->renderPass;                // 需要创建的新渲染流程
            compositePassInfo.framebuffer = swapChainFramebuffers[imageIndex]; // 回到交换链
            compositePassInfo.renderArea.offset = {0, 0};
            compositePassInfo.renderArea.extent = swapChainExtent;

            // 注意：这里不清除，因为我们只是叠加云层
            std::array<VkClearValue, 2> clearValues{};
            clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
            clearValues[1].depthStencil = { 1.0f, 0 };
            compositePassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
            compositePassInfo.pClearValues = clearValues.data();

            vkCmdBeginRenderPass(commandBuffer, &compositePassInfo, VK_SUBPASS_CONTENTS_INLINE);

            // 恢复全分辨率视口
            VkViewport viewport{};
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = (float)swapChainExtent.width;
            viewport.height = (float)swapChainExtent.height;
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;

            VkRect2D scissor{};
            scissor.offset = {0, 0};
            scissor.extent = swapChainExtent;

            vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
            vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

            // 绑定合成管线
            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                              this->compositePipeline->graphicsPipeline);

            // 绑定描述符集（包含降采样云纹理）
            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    this->compositePipeline->pipelineLayout, 0, 1,
                                    &compositeDescriptorSets[currentFrame], 0, nullptr);

            CompositePushConstants pushConstants{};
            pushConstants.fullResolution = glm::vec2(swapChainExtent.width, swapChainExtent.height);
            pushConstants.halfResolution = glm::vec2(halfResPass->extent.width, halfResPass->extent.height);
            pushConstants.upsampleScale = (float)downscaleFactor; 
            pushConstants.time = totalTime;   // 当前时间
            pushConstants.framenumber = this->framenumber;

            vkCmdPushConstants(commandBuffer, compositePipeline->pipelineLayout,
                               VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                               sizeof(CompositePushConstants), &pushConstants);

            // 绘制全屏三角形进行合成
            vkCmdDraw(commandBuffer, 3, 1, 0, 0);

            vkCmdEndRenderPass(commandBuffer);
        }

        // 结束记录指令
        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to record command buffer!");
        }

        
    }

void vulkanContext::mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    auto* context = reinterpret_cast<vulkanContext*>(glfwGetWindowUserPointer(window));
    context->processMouseMovement(static_cast<float>(xpos), static_cast<float>(ypos));
}

void vulkanContext::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    // 可以在这里处理鼠标按键事件
}

void vulkanContext::scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    auto* context = reinterpret_cast<vulkanContext*>(glfwGetWindowUserPointer(window));
    context->processMouseScroll(static_cast<float>(yoffset));
}

void vulkanContext::processMouseMovement(float xpos, float ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // 反转Y轴，因为屏幕坐标从上到下递增
    
    lastX = xpos;
    lastY = ypos;
    
    camera.processMouseMovement(xoffset, yoffset);
}

void vulkanContext::processMouseScroll(float yoffset) {
    camera.processMouseScroll(yoffset);
}

void vulkanContext::keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    auto *app = reinterpret_cast<vulkanContext *>(glfwGetWindowUserPointer(window));
    app->processKeyInput(key, action);
}

void vulkanContext::processKeyInput(int key, int action)
{
    if (action == GLFW_PRESS)
    {
        keys[key] = true;
    }
    else if (action == GLFW_RELEASE)
    {
        keys[key] = false;
    }
}

void vulkanContext::updateCameraPosition()
{
    float velocity = 50.0 * this->deltaTime ;
    if (keys[GLFW_KEY_W])
    {
        camera.Position += camera.Front * velocity;
        
    }
    if (keys[GLFW_KEY_S])
    {
        camera.Position -= camera.Front * velocity;
        
    }
    if (keys[GLFW_KEY_A])
    {
        camera.Position -= glm::normalize(glm::cross(camera.Front, camera.Up)) * velocity;
        
    }
    if (keys[GLFW_KEY_D])
    {
        camera.Position += glm::normalize(glm::cross(camera.Front, camera.Up)) * velocity;
        
    }
    if (keys[GLFW_KEY_SPACE])
    { // 上升
        camera.Position += camera.Up * velocity;
        
    }
    if (keys[GLFW_KEY_LEFT_SHIFT] || keys[GLFW_KEY_RIGHT_SHIFT])
    { // 下降
        camera.Position -= camera.Up * velocity;
        
    }
}
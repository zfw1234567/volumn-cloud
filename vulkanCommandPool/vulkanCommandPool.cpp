#include <vulkanCommandPool/vulkanCommandPool.h>

vulkanCommandPool::vulkanCommandPool()
{

}

vulkanCommandPool::vulkanCommandPool(VkDevice device,VkPhysicalDevice physicalDevice,VkSurfaceKHR surface)
{
    this->device=device;
    this->physicalDevice=physicalDevice;
    this->surface=surface;
    queueFamilyIndices = findQueueFamilies(physicalDevice);
}

vulkanCommandPool::~vulkanCommandPool()
{
    vkDestroyCommandPool(device, commandPool, nullptr);
    vkDestroyCommandPool(device, computeCommandPool, nullptr);
}

void vulkanCommandPool::setContext(VkDevice device,VkPhysicalDevice physicalDevice,VkSurfaceKHR surface)
{
    this->device=device;
    this->physicalDevice=physicalDevice;
    this->surface=surface;
    queueFamilyIndices = findQueueFamilies(physicalDevice);
}

void vulkanCommandPool::createCommandPool()
{
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create command pool!");
    }
}

QueueFamilyIndices vulkanCommandPool::findQueueFamilies(VkPhysicalDevice device) {
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

void vulkanCommandPool::createCommandBuffer()
{
    commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

    if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}

void vulkanCommandPool::createComputeCommandPool() {
    if (!queueFamilyIndices.computeFamily.has_value()) {
        throw std::runtime_error("No compute queue family found!");
    }
    
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.computeFamily.value();
    
    if (vkCreateCommandPool(device, &poolInfo, nullptr, &computeCommandPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create compute command pool!");
    }
}

// 新增函数：创建计算命令缓冲区
void vulkanCommandPool::createComputeCommandBuffer() {
    if (computeCommandPool == VK_NULL_HANDLE) {
        createComputeCommandPool();
    }
    
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = computeCommandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;
    
    if (vkAllocateCommandBuffers(device, &allocInfo, &computeCommandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate compute command buffer!");
    }
}

// 新增函数：开始计算命令缓冲区的录制
VkCommandBuffer vulkanCommandPool::beginComputeCommandBuffer() {
    if (computeCommandBuffer == VK_NULL_HANDLE) {
        createComputeCommandBuffer();
    }
    
    // 重置命令缓冲区
    vkResetCommandBuffer(computeCommandBuffer, 0);
    
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0; // 或 VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    beginInfo.pInheritanceInfo = nullptr;
    
    if (vkBeginCommandBuffer(computeCommandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin compute command buffer!");
    }
    
    return computeCommandBuffer;
}

// 新增函数：结束计算命令缓冲区的录制
void vulkanCommandPool::endComputeCommandBuffer() {
    if (vkEndCommandBuffer(computeCommandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to end compute command buffer!");
    }

}

// 新增函数：提交计算命令缓冲区
void vulkanCommandPool::submitComputeCommandBuffer(VkQueue computeQueue, VkFence fence) {
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &computeCommandBuffer;
    
    if (vkQueueSubmit(computeQueue, 1, &submitInfo, fence) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit compute command buffer!");
    }
}

// 新增函数：重置计算命令缓冲区
void vulkanCommandPool::resetComputeCommandBuffer() {
    vkResetCommandBuffer(computeCommandBuffer, 0);
}

#include <vulkanRender/vulkanUniformBuffer.h>

vulkanUniformBuffer::vulkanUniformBuffer(VkDevice device,VkPhysicalDevice physicalDevice)
{
    this->device=device;
    this->physicalDevice=physicalDevice;
}

vulkanUniformBuffer::~vulkanUniformBuffer()
{
    std::cout<<"____"<<std::endl;
    std::cout<<"开始销毁：vulkanUniformBuffer"<<std::endl;
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        vkDestroyBuffer(device, UniformBuffers[i], nullptr);
        vkFreeMemory(device, UniformBuffersMemory[i], nullptr);
    }
    std::cout<<"销毁结束"<<std::endl;
    std::cout<<"____"<<std::endl;
}

/** 创建统一缓存区（UBO）*/
void vulkanUniformBuffer::createUniformBuffers(VkDeviceSize bufferSizeOfMesh)
{
    UniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    UniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        createBuffer(bufferSizeOfMesh, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, UniformBuffers[i], UniformBuffersMemory[i]);

        // 这里会导致 memory stack overflow ，不应该在这里 vkMapMemory
        // vkMapMemory(device, meshUniformBuffersMemory[i], 0, bufferSize, 0, &uniformBuffersMapped[i]);
    }
}

    void vulkanUniformBuffer::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to create buffer!");
        }

        // 为VertexBuffer创建内存，并赋予
        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        // 自动找到适合的内存类型
        allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);
        // 关联分配的内存地址
        if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate buffer memory!");
        }
        // 绑定VertexBuffer和它的内存地址
        vkBindBufferMemory(device, buffer, bufferMemory, 0);
    }

    uint32_t vulkanUniformBuffer::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
    {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

        // 自动寻找适合的内存类型
        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }

        throw std::runtime_error("failed to find suitable memory type!");
    }
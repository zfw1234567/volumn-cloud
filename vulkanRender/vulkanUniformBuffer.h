#ifndef VULKANUNIFORMBUFFER_H
#define VULKANUNIFORMBUFFER_H

#define GLFW_INCLUDE_VULKAN
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE // 深度缓存区，OpenGL默认是（-1.0， 1.0）Vulakn为（0.0， 1.0）
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <limits>
#include <optional>
#include <set>
#include <array>
#include <chrono>
#include <unordered_map>

#include <vulkanDataStructure/vulkanDataStructure.h>
class vulkanUniformBuffer
{
    

private:
    const int MAX_FRAMES_IN_FLIGHT = 2;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE; // 物理显卡硬件
    VkDevice device;                                  // 逻辑硬件，对接物理硬件
    /* data */

public:


    std::vector<VkBuffer> UniformBuffers;             // 统一缓存区
    std::vector<VkDeviceMemory> UniformBuffersMemory; // 统一缓存区内存地址

    vulkanUniformBuffer(VkDevice device,VkPhysicalDevice physicalDevice);
    ~vulkanUniformBuffer();

    void createUniformBuffers(VkDeviceSize bufferSizeOfMesh);
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &bufferMemory);
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
};

#endif
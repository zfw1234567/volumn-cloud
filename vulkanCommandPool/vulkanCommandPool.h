#ifndef VULKANCOMMANDPOOL_H
#define VULKANCOMMANDPOOL_H
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
class vulkanCommandPool
{
private:
const int MAX_FRAMES_IN_FLIGHT = 2;

    VkSurfaceKHR surface;								// 链接桌面和Vulkan的实例

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;	// 物理显卡硬件
    VkDevice device;									// 逻辑硬件，对接物理硬件
    /* data */
    

public:
    VkCommandPool commandPool;							// 指令池
    std::vector<VkCommandBuffer> commandBuffers;		// 指令缓存
    QueueFamilyIndices queueFamilyIndices;
    vulkanCommandPool();
    vulkanCommandPool(VkDevice device,VkPhysicalDevice physicalDevice,VkSurfaceKHR surface);
    ~vulkanCommandPool();

    void setContext(VkDevice device,VkPhysicalDevice physicalDevice,VkSurfaceKHR surface);
    //图形管线相关
    void createCommandPool();
    void createCommandBuffer();
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

    //计算管线相关
    VkCommandPool computeCommandPool = VK_NULL_HANDLE;
    VkCommandBuffer computeCommandBuffer = VK_NULL_HANDLE;

    void createComputeCommandPool();
    void createComputeCommandBuffer();
    VkCommandBuffer beginComputeCommandBuffer();
    void endComputeCommandBuffer();
    void submitComputeCommandBuffer(VkQueue computeQueue, VkFence fence = VK_NULL_HANDLE);
    void resetComputeCommandBuffer();

};



#endif
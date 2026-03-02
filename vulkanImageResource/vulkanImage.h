#ifndef VULKANIMAGE_H
#define VULKANIMAGE_H

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

class vulkanImage
{
private:
    /* data */
    
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkExtent2D swapChainExtent;	
    VkCommandPool commandPool;
    VkQueue graphicsQueue;

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
public:
    VkImage Image;              // 纹理资源
    VkDeviceMemory ImageMemory; // 纹理内存
    VkImageView ImageView;      // 纹理图像视口
    VkSampler ImageSampler;

    VkFormat ImageFormat;

    vulkanImage(VkDevice device, VkPhysicalDevice physicalDevice,VkExtent2D swapChainExtent, VkCommandPool commandPool,VkQueue graphicsQueue);
    ~vulkanImage();

    void createDownsampleImage(uint32_t downscaleFactor);
};

#endif
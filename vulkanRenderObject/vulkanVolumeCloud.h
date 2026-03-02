#ifndef VULKANVULEMECLOUD_H
#define VULKANVULEMECLOUD_H

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
#include <vulkanImageResource/vulkanResource.h>
#include <vulkanRender/vulkanUniformBuffer.h>
#include <vulkanDataStructure/tool/VolumeTextureGenerator.h>
#include <vulkanCommandPool/vulkanCommandPool.h>

class vulkanVolumeCloud
{
        /** 渲染一个模型需要的所有Vulkan资源*/
    struct cloud {
        VkImage volumeImage = VK_NULL_HANDLE;
        VkImageView volumeImageView = VK_NULL_HANDLE;
        VkDeviceMemory volumeImageMemory = VK_NULL_HANDLE;
        VkSampler volumeSampler;
        uint32_t width; 
        uint32_t height;
        uint32_t depth;


        VkDescriptorPool descriptorPool;				// 描述符池
        std::vector<VkDescriptorSet> descriptorSets;	// 描述符集合
    };

    struct BoundingBox
    {
        glm::vec3 min;    // 包围盒最小点（世界坐标）
        glm::vec3 max;    // 包围盒最大点（世界坐标）
        glm::vec3 size;   // 包围盒尺寸
        glm::vec3 center; // 包围盒中心
    } ;

    
private:
    const int MAX_FRAMES_IN_FLIGHT = 2;

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE; // 物理显卡硬件
    VkDevice device;                                  // 逻辑硬件，对接物理硬件
    VkCommandPool commandPool;
    VkExtent2D swapChainExtent;	
    VkQueue graphicsQueue;

    /* data */
    VkShaderModule createShaderModule(const std::vector<char> &code);

public:
vulkanCommandPool* commandpool;

    cloud object;
    BoundingBox bounding;
    std::vector<vulkanTextureResource> weatherMap;
    std::vector<vulkanTextureResource> Noise;
    
    vulkanVolumeCloud(VkDevice device, VkPhysicalDevice physicalDevice, VkExtent2D swapChainExtent, VkCommandPool commandPool,VkQueue graphicsQueue);
    ~vulkanVolumeCloud();
    void createStageRenderResource(const VkDescriptorSetLayout &descriptorSetLayout, const vulkanUniformBuffer &meshUniformBuffer,const vulkanUniformBuffer &viewUniformBuffer);
    bool createCloudVolumeTexture(uint32_t width, uint32_t height, uint32_t depth);
    void uploadVolumeData(const void* data, VkDeviceSize dataSize);
    void createDescriptorPool( uint32_t sampler_num = 1);
    void createDescriptorSets(const VkDescriptorSetLayout &inDescriptorSetLayout, const vulkanUniformBuffer &meshUniformBuffer,const vulkanUniformBuffer &viewUniformBuffer);
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
};

#endif
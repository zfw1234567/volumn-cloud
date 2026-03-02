#ifndef VULKANRENDERBACKGROUNDOBJECT_H
#define VULKANRENDERBACKGROUNDOBJECT_H

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

class vulkanRenderBackgroundObject
{
        /** 渲染一个模型需要的所有Vulkan资源*/
    struct StageObject {
        // std::vector<Vertex> vertices;		// 顶点
        // std::vector<uint32_t> indices;		// 点序
        // VkBuffer vertexBuffer;				// 顶点缓存
        // VkDeviceMemory vertexBufferMemory;	// 顶点缓存内存
        // VkBuffer indexBuffer;				// 点序缓存
        // VkDeviceMemory indexBufferMemory;	// 点序缓存内存

        std::vector<vulkanTextureResource> TextureImages;

        VkDescriptorPool descriptorPool;				// 描述符池
        std::vector<VkDescriptorSet> descriptorSets;	// 描述符集合
    };
    
private:
    const int MAX_FRAMES_IN_FLIGHT = 2;

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE; // 物理显卡硬件
    VkDevice device;                                  // 逻辑硬件，对接物理硬件
    VkCommandPool commandPool;
    VkExtent2D swapChainExtent;
    VkQueue graphicsQueue;
    /* data */
    

public:
    StageObject object;
    vulkanRenderBackgroundObject(VkDevice device, VkPhysicalDevice physicalDevice, VkExtent2D swapChainExtent, VkCommandPool commandPool,VkQueue graphicsQueue);
    ~vulkanRenderBackgroundObject();
    void createStageRenderResource (const std::vector<std::string> &pngfiles, const VkDescriptorSetLayout &descriptorSetLayout,const vulkanUniformBuffer &meshUniformBuffer, const vulkanUniformBuffer &viewUniformBuffer,const vulkancubemaperesource &cubemap);
    void createVertices(std::vector<Vertex>& outVertices, std::vector<uint32_t>& outIndices, const std::string& filename);
    static void readModelResource(const std::string& filename, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);
    void createDescriptorPool( uint32_t sampler_num = 1);
    void createDescriptorSets(const VkDescriptorSetLayout &inDescriptorSetLayout,const vulkanUniformBuffer &meshUniformBuffer, const vulkanUniformBuffer &viewUniformBuffer,const vulkancubemaperesource &cubemap);
};


#endif
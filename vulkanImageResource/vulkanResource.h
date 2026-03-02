#ifndef VULKANRECOURCE_H
#define VULKANRECOURCE_H

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

#include "stb_impl/stb_image_wrapper.h"

class vulkanDepthResource
{
private:
    /* data */
    VkFormat depthFormat;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkExtent2D swapChainExtent;	



public:
    VkImage depthImage;              // 深度纹理资源
    VkDeviceMemory depthImageMemory; // 深度纹理内存
    VkImageView depthImageView;      // 深度纹理图像视口
    vulkanDepthResource();
    vulkanDepthResource(VkDevice device, VkPhysicalDevice physicalDevice,VkExtent2D swapChainExtent);
    ~vulkanDepthResource();
    
    void setContext(VkDevice device, VkPhysicalDevice physicalDevice,VkExtent2D swapChainExtent);
    void createDepthResources();
    VkImage createImage(VkDeviceMemory& imageMemory, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, uint32_t miplevels = 1);
    void createImageView(VkImageView& outImageView, const VkImage& inImage, const VkFormat& inFormat, VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT, uint32_t levelCount = 1);
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    VkFormat findDepthFormat();
    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

    void cleanup();
};

class vulkancubemaperesource
{
    private:
    /* data */
    VkFormat depthFormat;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkExtent2D swapChainExtent;	
    VkCommandPool commandPool;
    VkQueue graphicsQueue;

    public:
    uint32_t cubemapMaxMips;  
    VkImage Image;              // 纹理资源
    VkDeviceMemory ImageMemory; // 纹理内存
    VkImageView ImageView;      // 纹理图像视口
    VkSampler Sampler;
    vulkancubemaperesource();
    vulkancubemaperesource(VkDevice device, VkPhysicalDevice physicalDevice,VkExtent2D swapChainExtent, VkCommandPool commandPool,VkQueue graphicsQueue);
    ~vulkancubemaperesource();

    void setcontext(VkDevice device, VkPhysicalDevice physicalDevice,VkExtent2D swapChainExtent, VkCommandPool commandPool,VkQueue graphicsQueue);
    void createImageHDRContext(  const std::vector<std::string>& filenames);
    static stbi_uc* readTextureResource(const std::string& filename, int& texWidth, int& texHeight, int& texChannels, int& mipLevels);
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    void createSampler(VkSampler& outSampler, uint32_t miplevels = 1);

    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);
};

class vulkanTextureResource
{
private:
    /* data */
    VkFormat TextureFormat;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkExtent2D swapChainExtent;	
    VkCommandPool commandPool;
    VkQueue graphicsQueue;



public:
    VkImage TextureImage;              // 纹理资源
    VkDeviceMemory TextureImageMemory; // 纹理内存
    VkImageView TextureImageView;      // 纹理图像视口
    VkSampler TextureSampler;
    
    vulkanTextureResource(VkDevice device, VkPhysicalDevice physicalDevice,VkExtent2D swapChainExtent, VkCommandPool commandPool,VkQueue graphicsQueue);
    ~vulkanTextureResource();

    void createImageContext( const std::string& filename, bool sRGB = true);
    VkImage createImage(VkDeviceMemory& imageMemory, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, uint32_t miplevels = 1);
    void createImageView(VkImageView& outImageView, const VkImage& inImage, const VkFormat& inFormat, VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT, uint32_t levelCount = 1);
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    VkFormat findDepthFormat();
    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
    static stbi_uc* readTextureResource(const std::string& filename, int& texWidth, int& texHeight, int& texChannels, int& mipLevels);
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    void createSampler(VkSampler& outSampler, uint32_t miplevels = 1);
    void transitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t miplevels = 1);
    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);
    void generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);
};

#endif
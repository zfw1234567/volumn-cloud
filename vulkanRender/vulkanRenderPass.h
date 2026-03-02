#ifndef VULKANRENDERPASS_H
#define VULKANRENDERPASS_H

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
#include <vulkanImageResource/vulkanImage.h>

class vulkanRenderPass
{
private:
    const size_t MaxAttachment = 5;
    int AttachmentCount = 0;
    

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;	// 物理显卡硬件
    VkDevice device;									// 逻辑硬件，对接物理硬件
    /* data */

    std::vector<VkAttachmentDescription> colorAttachmentDescriptions;  // 颜色组件描述队列
    VkAttachmentDescription depthAttachment;
    std::vector<VkAttachmentReference> colorAttachmentRefs;
    VkAttachmentReference depthAttachmentRef{};
    

public:
    VkRenderPass renderPass;							// 渲染层，保存Framebuffer和采样信息
    VkExtent2D extent;
    std::vector<vulkanImage> Image;
    VkFramebuffer Framebuffer;

    uint32_t colorAttachmentCount = 0;

    vulkanRenderPass(VkDevice device, VkPhysicalDevice physicalDevice);
    ~vulkanRenderPass();

    void createcolorAttachment(VkFormat swapChainImageFormat);
    void createdepthAttachment();
    void createAllAttachmentReference();
    
    void createCompositecolorAttachment(VkFormat swapChainImageFormat);
    void createMiddlecolorAttachment(VkFormat swapChainImageFormat);

    void setPass();


    VkFormat findDepthFormat();
    VkFormat findSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
};



#endif
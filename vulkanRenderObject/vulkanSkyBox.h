#ifndef VULKANSKYBOX_H
#define VULKANSKYBOX_H
#include "vulkanImageResource/vulkanResource.h"
#include <vulkanRender/vulkanUniformBuffer.h>
class vulkanSkyBox
{
private:
    /* data */
    vulkancubemaperesource cubemap;
    VkDevice device;
    VkPhysicalDevice physicalDevice;
    VkExtent2D swapChainExtent;
    VkCommandPool commandPool;
    VkQueue graphicsQueue;

const int MAX_FRAMES_IN_FLIGHT = 2;

public:
    VkDescriptorPool descriptorPool;             // 描述符池
    std::vector<VkDescriptorSet> descriptorSets; // 描述符集合

    vulkanSkyBox(VkDevice device, VkPhysicalDevice physicalDevice,VkExtent2D swapChainExtent, VkCommandPool commandPool,VkQueue graphicsQueue);
    ~vulkanSkyBox();
    void createStageRenderResource(const VkDescriptorSetLayout &descriptorSetLayout,const vulkanUniformBuffer &meshUniformBuffer, const vulkanUniformBuffer &viewUniformBuffer);
    void createCubeMap( const std::vector<std::string> &filenames);
    void createDescriptorPool( uint32_t sampler_num = 1);
    void createDescriptorSets(const VkDescriptorSetLayout &inDescriptorSetLayout, const vulkanUniformBuffer &meshUniformBuffer,const vulkanUniformBuffer &viewUniformBuffer);
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
};



#endif
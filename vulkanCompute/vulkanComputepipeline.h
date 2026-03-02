#ifndef VULKANCOMPUTEPIPELINE_H
#define VULKANCOMPUTEPIPELINE_H

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

#include <vulkanCommandPool/vulkanCommandPool.h>
class vulkanComputepipeline
{
private:
    /* data */
    VkDevice device;
    vulkanCommandPool* commandpool;
    VkPhysicalDevice physicalDevice;
    VkShaderModule computeShaderModule;

    std::vector<char> readShaderSource(const std::string &filename);
    VkShaderModule createShaderModule(const std::vector<char> &code);
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
public:
    VkDescriptorSetLayout descriptorSetLayout;  // 描述符集合布局
    VkPipelineLayout pipelineLayout;            // 渲染管线布局
    VkPipeline computePipeline;                // 渲染管线

    VkDescriptorPool descriptorPool;             // 描述符池
    VkDescriptorSet descriptorSet;               // 描述符集合

    vulkanComputepipeline(VkDevice device,VkPhysicalDevice physicalDevice,vulkanCommandPool* commandpool);
    ~vulkanComputepipeline();
    void createshader(const std::string& vert_filename);
    void createDescriptorSetLayout();
    void createPipeline();
    void createDescriptor();
    void createPipelineLayout();
    void updateDescriptorSet(VkImageView imageView);

    void calculate(VkImage& volumeImage,VkImageView& volumeImageView,VkDeviceMemory& volumeImageMemory,VkSampler& volumeSampler);
};



#endif
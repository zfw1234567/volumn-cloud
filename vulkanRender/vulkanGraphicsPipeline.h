#ifndef VULKANGRAPHICSPIPELINE_H
#define VULKANGRAPHICSPIPELINE_H

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

class vulkanGraphicsPipeline
{
private:
    /* data */
    VkDevice device;
    VkRenderPass renderPass;

    VkShaderModule vertShaderModule;
    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};

    VkShaderModule fragShaderModule;
    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};

    VkPipelineShaderStageCreateInfo shaderStages[2];



    VkBool32 bDepthTest = VK_TRUE;
    VkCullModeFlags CullMode = VK_CULL_MODE_BACK_BIT;

    bool blend=false;

    bool skybox=false;

public:
    VkDescriptorSetLayout descriptorSetLayout;  // 描述符集合布局
    VkPipelineLayout pipelineLayout;            // 渲染管线布局
    VkPipeline graphicsPipeline;                // 渲染管线
    vulkanGraphicsPipeline(VkDevice device,VkRenderPass renderPass,VkDescriptorSetLayout descriptorSetLayout);
    ~vulkanGraphicsPipeline();

    void createvertexshader(const std::string& vert_filename);
    void createfragmentshader(const std::string& frag_filename);

    void setshaderstage();

    VkShaderModule createShaderModule(const std::vector<char>& code);

    void isbackgroundrender();
    void isSkyBox();
    void setPipeline(int colorAttachment);
    void setcompositePipeline();

    void createGraphicsPipeline(VkPipelineLayout& outPipelineLayout, VkPipeline& outGraphicsPipeline, const VkDescriptorSetLayout& inDescriptorSetLayout, const std::string& vert_filename, const std::string& frag_filename,
        VkBool32 bDepthTest = VK_TRUE, VkCullModeFlags CullMode = VK_CULL_MODE_BACK_BIT);

    void blendEnable();
    static std::vector<char> readShaderSource(const std::string& filename);
};



#endif
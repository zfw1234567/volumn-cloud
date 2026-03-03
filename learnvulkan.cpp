// Copyright LearnVulkan-06: Draw with PBR, @xukai. All Rights Reserved.
#define GLFW_INCLUDE_VULKAN
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

// #define STB_IMAGE_IMPLEMENTATION
// #include <stb_image.h>
// #define TINYOBJLOADER_IMPLEMENTATION
// #include <tiny_obj_loader.h>


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

#include <vulkanContext/vulkanContext.h>
#include <vulkanRender/vulkanRenderPass.h>
#include <vulkanCommandPool/vulkanCommandPool.h>
#include <vulkanRender/vulkanDescriptorSetLayout.h>
#include <vulkanRender/vulkanGraphicsPipeline.h>
#include <vulkanRender/vulkanUniformBuffer.h>
#include <vulkanRenderObject/vulkanRenderBackgroundObject.h>
#include <vulkanRenderObject/vulkanRenderObject.h>
#include <vulkanRenderObject/vulkanVolumeCloud.h>
#include <vulkanRenderObject/vulkanSkyBox.h>
#include <vulkanCompute/vulkanComputepipeline.h>
#include <vulkanImageResource/vulkanImage.h>

const uint32_t WIDTH = 1080;
const uint32_t HEIGHT = 720;

/** 同时渲染多帧的最大帧数*/
const int MAX_FRAMES_IN_FLIGHT = 2;

const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" }; // VK_LAYER_KHRONOS_validation这个是固定的，不能重命名
const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

#ifdef NDEBUG
const bool enableValidationLayers = false;  // Build Configuration: Release
#else
const bool enableValidationLayers = true;   // Build Configuration: Debug
#endif

static VkResult createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

static void destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        func(instance, debugMessenger, pAllocator);
    }
}


int main()
{

    vulkanContext app;

    vulkanCommandPool commandPool(app.device,app.physicalDevice,app.surface);
    commandPool.createCommandPool();
    commandPool.createCommandBuffer();
    commandPool.createComputeCommandPool();
    commandPool.createComputeCommandBuffer();
    app.commandpool=&commandPool;

    std::cout<<"start backgroundPass"<<std::endl;
    vulkanRenderPass backgroundPass(app.device,app.physicalDevice);
    backgroundPass.createMiddlecolorAttachment(app.swapChainImageFormat);
    backgroundPass.createdepthAttachment();
    backgroundPass.createAllAttachmentReference();
    backgroundPass.setPass();

    backgroundPass.Image.emplace_back(app.device,app.physicalDevice,app.swapChainExtent,app.commandpool->commandPool,app.graphicsQueue);
    backgroundPass.Image[0].createDownsampleImage(1);
    std::array<VkImageView, 2> attachments = {
            backgroundPass.Image[0].ImageView,
            app.depthImage.depthImageView};
    
    VkFramebufferCreateInfo framebufferInfo = {};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = backgroundPass.renderPass;
    framebufferInfo.attachmentCount = 2;
    framebufferInfo.pAttachments = attachments.data();
    framebufferInfo.width = app.swapChainExtent.width;
    framebufferInfo.height = app.swapChainExtent.height;
    framebufferInfo.layers = 1;

    vkCreateFramebuffer(app.device, &framebufferInfo, nullptr, &backgroundPass.Framebuffer);


    std::cout<<"start backgroundDescriptorSetLayout"<<std::endl;
    vulkanDescriptorSetLayout backgroundDescriptorSetLayout(app.device);
    backgroundDescriptorSetLayout.createDescriptorSetLayout();

    std::cout<<"start backgroundPipeline"<<std::endl;
    vulkanGraphicsPipeline backgroundPipeline(app.device,backgroundPass.renderPass,backgroundDescriptorSetLayout.outDescriptorSetLayout);
    backgroundPipeline.isbackgroundrender();
    backgroundPipeline.createvertexshader("Resources/Shaders/draw_with_PBR_bg_vert.spv");
    backgroundPipeline.createfragmentshader("Resources/Shaders/draw_with_PBR_bg_frag.spv");
    backgroundPipeline.setshaderstage();
    backgroundPipeline.setPipeline(backgroundPass.colorAttachmentCount);

    app.backgroundPipeline=&backgroundPipeline;
    app.renderPass=&backgroundPass;

    std::cout<<"start cubemap"<<std::endl;
    vulkancubemaperesource cubemap(app.device,app.physicalDevice,app.swapChainExtent,app.commandpool->commandPool,app.graphicsQueue);
    cubemap.createImageHDRContext({
            "Resources/Textures/cubemap_X0.png",
            "Resources/Textures/cubemap_X1.png",
            "Resources/Textures/cubemap_Y2.png",
            "Resources/Textures/cubemap_Y3.png",
            "Resources/Textures/cubemap_Z4.png",
            "Resources/Textures/cubemap_Z5.png" });

    std::cout<<"start Framebuffers"<<std::endl;
    app.createFramebuffers();

    std::cout<<"start Uniformbuffer"<<std::endl;
    vulkanUniformBuffer meshUniformbuffer(app.device,app.physicalDevice);
    meshUniformbuffer.createUniformBuffers(sizeof(UniformBufferObject));

    vulkanUniformBuffer skyboxUniformbuffer(app.device,app.physicalDevice);
    skyboxUniformbuffer.createUniformBuffers(sizeof(UniformBufferObject));
    app.skyUniform=&skyboxUniformbuffer;

    vulkanUniformBuffer viewUniformbuffer(app.device,app.physicalDevice);
    viewUniformbuffer.createUniformBuffers(sizeof(UniformBufferObjectView));

    vulkanUniformBuffer VolumeCloudUniformbuffer(app.device,app.physicalDevice);
    VolumeCloudUniformbuffer.createUniformBuffers(sizeof(UniformBufferVolumeCloud));
    app.volumeclouduniform=&VolumeCloudUniformbuffer;


    //计算管线
    vulkanComputepipeline computePipeline(app.device,app.physicalDevice,app.commandpool);
    computePipeline.createshader("Resources/Shaders/volume_cloud_shader_comp.spv");
    computePipeline.createDescriptorSetLayout();
    computePipeline.createPipelineLayout();
    computePipeline.createPipeline();
    computePipeline.createDescriptor();

    std::cout<<"start volumelayout"<<std::endl;
    vulkanDescriptorSetLayout volumelayout(app.device);
    volumelayout.createVolumecloudLayout();

    
    
    std::cout<<"start volumecloud"<<std::endl;
    vulkanVolumeCloud volumecloud(app.device,app.physicalDevice,app.swapChainExtent, app.commandpool->commandPool,app.graphicsQueue);
    volumecloud.commandpool=&commandPool;
    computePipeline.calculate(volumecloud.object.volumeImage,volumecloud.object.volumeImageView,volumecloud.object.volumeImageMemory,volumecloud.object.volumeSampler);

    volumecloud.weatherMap.emplace_back(app.device,app.physicalDevice,app.swapChainExtent,app.commandpool->commandPool,app.graphicsQueue);
    volumecloud.weatherMap.back().createImageContext("Resources/Textures/wethermap.png");
    volumecloud.Noise.emplace_back(app.device,app.physicalDevice,app.swapChainExtent,app.commandpool->commandPool,app.graphicsQueue);
    volumecloud.Noise.back().createImageContext("Resources/Textures/LDR_LLL1_0.png");
    std::cout<<"end volumecreate"<<std::endl;

    volumecloud.createStageRenderResource(volumelayout.outDescriptorSetLayout,VolumeCloudUniformbuffer,viewUniformbuffer);
    volumecloud.bounding.center=glm::vec3(0.0,100.0,0.0);
    volumecloud.bounding.size=glm::vec3(1500,50,1500);
    app.volumecloud=&volumecloud;
    std::cout<<"end volumecloudcreate"<<std::endl;

    app.downscaleFactor = 1; // 分辨率
    const VkExtent2D downscaledExtent = {
        app.swapChainExtent.width / app.downscaleFactor,
        app.swapChainExtent.height / app.downscaleFactor};

    vulkanRenderPass downsamplePass(app.device, app.physicalDevice);
    downsamplePass.Image.reserve(4);
    downsamplePass.Image.emplace_back(app.device,app.physicalDevice,app.swapChainExtent,app.commandpool->commandPool,app.graphicsQueue);
    downsamplePass.Image[0].createDownsampleImage(app.downscaleFactor);
    downsamplePass.Image.emplace_back(app.device,app.physicalDevice,app.swapChainExtent,app.commandpool->commandPool,app.graphicsQueue);
    downsamplePass.Image[1].createDownsampleImage(app.downscaleFactor);
    downsamplePass.Image.emplace_back(app.device,app.physicalDevice,app.swapChainExtent,app.commandpool->commandPool,app.graphicsQueue);
    downsamplePass.Image[2].createDownsampleImage(app.downscaleFactor);
    downsamplePass.Image.emplace_back(app.device,app.physicalDevice,app.swapChainExtent,app.commandpool->commandPool,app.graphicsQueue);
    downsamplePass.Image[3].createDownsampleImage(app.downscaleFactor);
    downsamplePass.createMiddlecolorAttachment(downsamplePass.Image[0].ImageFormat);
    downsamplePass.createMiddlecolorAttachment(downsamplePass.Image[1].ImageFormat);
    downsamplePass.createdepthAttachment();
    downsamplePass.createAllAttachmentReference();
    downsamplePass.setPass();
    downsamplePass.extent=downscaledExtent;
    app.halfResPass=&downsamplePass;


    {
        std::array<VkImageView, 3> attachments = {
            downsamplePass.Image[0].ImageView,
            downsamplePass.Image[1].ImageView,
            app.depthImage.depthImageView};
        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = downsamplePass.renderPass;
        framebufferInfo.attachmentCount = 3;
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = downscaledExtent.width;
        framebufferInfo.height = downscaledExtent.height;
        framebufferInfo.layers = 1;

        vkCreateFramebuffer(app.device, &framebufferInfo, nullptr, &app.halfRescloudFramebufferA);
    }

    {
        std::array<VkImageView, 3> attachments = {
            downsamplePass.Image[2].ImageView,
            downsamplePass.Image[3].ImageView,
            app.depthImage.depthImageView};
        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = downsamplePass.renderPass;
        framebufferInfo.attachmentCount = 3;
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = downscaledExtent.width;
        framebufferInfo.height = downscaledExtent.height;
        framebufferInfo.layers = 1;

        vkCreateFramebuffer(app.device, &framebufferInfo, nullptr, &app.halfRescloudFramebufferB);
    }

    std::cout<<"end downsamplePasscreate"<<std::endl;

    std::cout<<"start volumecloudpipeline"<<std::endl;
    vulkanGraphicsPipeline volumecloudpipeline(app.device,downsamplePass.renderPass,volumelayout.outDescriptorSetLayout);
    volumecloudpipeline.createvertexshader("Resources/Shaders/volume_cloud_vert.spv");
    volumecloudpipeline.createfragmentshader("Resources/Shaders/volume_cloud_test_frag.spv");
    volumecloudpipeline.setshaderstage();
    volumecloudpipeline.blendEnable();
    volumecloudpipeline.setPipeline(downsamplePass.colorAttachmentCount);
    app.volumeCloudPipeline=&volumecloudpipeline;
    std::cout<<"end volumecloudpipeline"<<std::endl;

    vulkanRenderPass compositeRenderPass(app.device, app.physicalDevice);
    compositeRenderPass.createCompositecolorAttachment(app.swapChainImageFormat);
    compositeRenderPass.createdepthAttachment();
    compositeRenderPass.createAllAttachmentReference();
    compositeRenderPass.setPass();
    app.compositeRenderPass=&compositeRenderPass;

    vulkanDescriptorSetLayout compositelayout(app.device);
    compositelayout.createcompositeLayout();

    vulkanGraphicsPipeline compositepipeline(app.device,compositeRenderPass.renderPass,compositelayout.outDescriptorSetLayout);
    compositepipeline.createvertexshader("Resources/Shaders/composite_vert.spv");
    compositepipeline.createfragmentshader("Resources/Shaders/composite_frag.spv");
    compositepipeline.setshaderstage();
    compositepipeline.isSkyBox();
    compositepipeline.blendEnable();
    compositepipeline.setcompositePipeline();
    app.compositePipeline=&compositepipeline;



    std::array<VkDescriptorPoolSize, 1> poolSizes{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT) * 5;
    
    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    
    if (vkCreateDescriptorPool(app.device, &poolInfo, nullptr, 
                               &app.compositeDescriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create composite descriptor pool!");
    }

    std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT,
                                               compositelayout.outDescriptorSetLayout);

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = app.compositeDescriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    allocInfo.pSetLayouts = layouts.data();

    app.compositeDescriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
    if (vkAllocateDescriptorSets(app.device, &allocInfo,
                                 app.compositeDescriptorSets.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate composite descriptor sets!");
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {

        std::vector<VkWriteDescriptorSet> descriptorWrites{};
        descriptorWrites.resize(5);

        VkDescriptorImageInfo cloudimageAInfo{};
        cloudimageAInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        cloudimageAInfo.imageView = downsamplePass.Image[0].ImageView; 
        cloudimageAInfo.sampler = downsamplePass.Image[0].ImageSampler; 

        VkDescriptorImageInfo clouddepthAInfo{};
        clouddepthAInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        clouddepthAInfo.imageView = downsamplePass.Image[1].ImageView;
        clouddepthAInfo.sampler = downsamplePass.Image[1].ImageSampler; 

        VkDescriptorImageInfo cloudimageBInfo{};
        cloudimageBInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        cloudimageBInfo.imageView = downsamplePass.Image[2].ImageView; 
        cloudimageBInfo.sampler = downsamplePass.Image[2].ImageSampler; 

        VkDescriptorImageInfo clouddepthBInfo{};
        clouddepthBInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        clouddepthBInfo.imageView = downsamplePass.Image[3].ImageView; 
        clouddepthBInfo.sampler = downsamplePass.Image[3].ImageSampler; 

        VkDescriptorImageInfo skyboximageInfo{};
        skyboximageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        skyboximageInfo.imageView = backgroundPass.Image[0].ImageView; 
        skyboximageInfo.sampler = backgroundPass.Image[0].ImageSampler; 
        
        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = app.compositeDescriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pImageInfo = &cloudimageAInfo;

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = app.compositeDescriptorSets[i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pImageInfo = &clouddepthAInfo;

        descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[2].dstSet = app.compositeDescriptorSets[i];
        descriptorWrites[2].dstBinding = 2;
        descriptorWrites[2].dstArrayElement = 0;
        descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[2].descriptorCount = 1;
        descriptorWrites[2].pImageInfo = &cloudimageBInfo;

        descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[3].dstSet = app.compositeDescriptorSets[i];
        descriptorWrites[3].dstBinding = 3;
        descriptorWrites[3].dstArrayElement = 0;
        descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[3].descriptorCount = 1;
        descriptorWrites[3].pImageInfo = &clouddepthBInfo;

        descriptorWrites[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[4].dstSet = app.compositeDescriptorSets[i];
        descriptorWrites[4].dstBinding = 4;
        descriptorWrites[4].dstArrayElement = 0;
        descriptorWrites[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[4].descriptorCount = 1;
        descriptorWrites[4].pImageInfo = &skyboximageInfo;
        
        vkUpdateDescriptorSets(app.device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }

    

    vulkanDescriptorSetLayout skyboxlayout(app.device);
    skyboxlayout.createskyboxLayout();

    vulkanSkyBox SkyBox(app.device,app.physicalDevice,app.swapChainExtent,app.commandpool->commandPool,app.graphicsQueue);
    SkyBox.createCubeMap({
            "Resources/Textures/front.jpg",
            "Resources/Textures/back.jpg",
            "Resources/Textures/top.jpg",
            "Resources/Textures/bottom.jpg",
            "Resources/Textures/left.jpg",
            "Resources/Textures/right.jpg" });
    SkyBox.createStageRenderResource(skyboxlayout.outDescriptorSetLayout,skyboxUniformbuffer,viewUniformbuffer);
    app.skybox=&SkyBox;

    vulkanGraphicsPipeline skyboxpipeline(app.device, backgroundPass.renderPass, skyboxlayout.outDescriptorSetLayout);
    skyboxpipeline.createvertexshader("Resources/Shaders/skybox_vert.spv");
    skyboxpipeline.createfragmentshader("Resources/Shaders/skybox_frag.spv");
    skyboxpipeline.setshaderstage();
    skyboxpipeline.isSkyBox();
    skyboxpipeline.setPipeline(backgroundPass.colorAttachmentCount);
    app.skyboxPipeline=&skyboxpipeline;

    app.createSyncObjects();
    std::cout<<"start render"<<std::endl;
    app.mainTick(meshUniformbuffer,viewUniformbuffer,cubemap);

    vkDeviceWaitIdle(app.device);

    
    return EXIT_SUCCESS;
}

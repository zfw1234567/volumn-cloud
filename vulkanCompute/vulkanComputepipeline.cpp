#include <vulkanCompute/vulkanComputepipeline.h>

vulkanComputepipeline::vulkanComputepipeline(VkDevice device,VkPhysicalDevice physicalDevice,vulkanCommandPool* commandpool)
{
    this->device=device;
    this->physicalDevice=physicalDevice;
    this->commandpool=commandpool;
}

vulkanComputepipeline::~vulkanComputepipeline()
{
    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
    vkDestroyShaderModule(device, computeShaderModule, nullptr);
    vkDestroyDescriptorPool(device, descriptorPool, nullptr);
    vkDestroyPipeline(device, computePipeline, nullptr);
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
}

void vulkanComputepipeline::createshader(const std::string& filename)
{
    auto vertShaderCode = readShaderSource(filename);
    computeShaderModule = createShaderModule(vertShaderCode);
}

std::vector<char> vulkanComputepipeline::readShaderSource(const std::string &filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        std::cout << filename << std::endl;
        throw std::runtime_error("failed to open shader file!");
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}

VkShaderModule vulkanComputepipeline::createShaderModule(const std::vector<char> &code)
{
    VkShaderModuleCreateInfo shaderInfo{};
    shaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderInfo.codeSize = code.size();
    shaderInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &shaderInfo, nullptr, &shaderModule) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create compute shader module!");
    }

    return shaderModule;
}

void vulkanComputepipeline::createDescriptorSetLayout()
{
    VkDescriptorSetLayoutBinding layoutBinding{};
    layoutBinding.binding = 0;
    layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    layoutBinding.descriptorCount = 1;
    layoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &layoutBinding;
    vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout);
}

void vulkanComputepipeline::createPipeline()
{
    VkComputePipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineInfo.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    pipelineInfo.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    pipelineInfo.stage.module = computeShaderModule;
    pipelineInfo.stage.pName = "main";
    pipelineInfo.layout = pipelineLayout;
    vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &computePipeline);
}

void vulkanComputepipeline::createDescriptor()
{
    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    poolSize.descriptorCount = 1;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = 1;
    if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create descriptor pool!");
    }

    // 分配描述符集
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &descriptorSetLayout;

    if (vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate descriptor set!");
    }
}

void vulkanComputepipeline::createPipelineLayout() {
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
    
    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }
}

void vulkanComputepipeline::updateDescriptorSet(VkImageView imageView) {
    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageView = imageView;
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    
    VkWriteDescriptorSet descriptorWrite{};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = descriptorSet;
    descriptorWrite.dstBinding = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pImageInfo = &imageInfo;
    
    vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
}

void vulkanComputepipeline::calculate(VkImage& volumeImage,VkImageView& volumeImageView,VkDeviceMemory& volumeImageMemory,VkSampler& volumeSampler)
{
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_3D;
    imageInfo.extent = {128, 128, 128};  // 3D纹理尺寸
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;  // 适合计算的格式
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    
    vkCreateImage(device, &imageInfo, nullptr, &volumeImage);
    
    // 分配内存并绑定
    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, volumeImage, &memRequirements);
    
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, 
                                              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    
    vkAllocateMemory(device, &allocInfo, nullptr, &volumeImageMemory);
    vkBindImageMemory(device, volumeImage, volumeImageMemory, 0);
    
    // 创建图像视图
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = volumeImage;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_3D;
    viewInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.layerCount = 1;
    
    vkCreateImageView(device, &viewInfo, nullptr, &volumeImageView);
    

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxAnisotropy = 1.0f;
    samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    this->updateDescriptorSet(volumeImageView);

    if (vkCreateSampler(device, &samplerInfo, nullptr, &volumeSampler) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create texture sampler!");
    }
    VkCommandBuffer computeCmd = commandpool->beginComputeCommandBuffer();
    
    // 图像布局转换（UNDEFINED → GENERAL）
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = volumeImage;
    barrier.subresourceRange = viewInfo.subresourceRange;
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    
    vkCmdPipelineBarrier(computeCmd, 
                        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                        0, 0, nullptr, 0, nullptr, 1, &barrier);
    
    // 绑定计算管线
    vkCmdBindPipeline(computeCmd, VK_PIPELINE_BIND_POINT_COMPUTE, computePipeline);
    
    // 绑定描述符集
    vkCmdBindDescriptorSets(computeCmd, VK_PIPELINE_BIND_POINT_COMPUTE, 
                           pipelineLayout, 0, 1, 
                           &descriptorSet, 0, nullptr);
    
    // 分发计算着色器
    vkCmdDispatch(computeCmd, 16, 16, 16);  // 根据你的工作组大小调整
    
    commandpool->endComputeCommandBuffer();
    
    // 获取计算队列并提交
    VkQueue computeQueue;
    vkGetDeviceQueue(device, commandpool->queueFamilyIndices.computeFamily.value(), 0, &computeQueue);
    
    VkFence computeFence;
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    vkCreateFence(device, &fenceInfo, nullptr, &computeFence);
    
    commandpool->submitComputeCommandBuffer(computeQueue, computeFence);
    
    // 等待计算完成
    vkWaitForFences(device, 1, &computeFence, VK_TRUE, UINT64_MAX);
    
    // 7. 清理资源
    vkDestroyFence(device, computeFence, nullptr);

    VkCommandBuffer layoutCmd = commandpool->beginComputeCommandBuffer();
    
    // 第二个屏障：从 GENERAL 转换为 SHADER_READ_ONLY_OPTIMAL
    VkImageMemoryBarrier readBarrier{};
    readBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    readBarrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    readBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    readBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    readBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    readBarrier.image = volumeImage;
    readBarrier.subresourceRange = viewInfo.subresourceRange;
    readBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;  // 计算着色器写入完成
    readBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;   // 准备被着色器读取
    
    vkCmdPipelineBarrier(layoutCmd,
                        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,  // 计算阶段完成后
                        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 
                        0, 0, nullptr, 0, nullptr, 1, &readBarrier);
    
    commandpool->endComputeCommandBuffer();
    
    // 提交布局转换命令
    VkFence layoutFence;
    VkFenceCreateInfo layoutFenceInfo{};
    layoutFenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    vkCreateFence(device, &layoutFenceInfo, nullptr, &layoutFence);
    
    VkSubmitInfo layoutSubmitInfo{};
    layoutSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    layoutSubmitInfo.commandBufferCount = 1;
    layoutSubmitInfo.pCommandBuffers = &layoutCmd;
    
    vkQueueSubmit(computeQueue, 1, &layoutSubmitInfo, layoutFence);
    vkWaitForFences(device, 1, &layoutFence, VK_TRUE, UINT64_MAX);
    
    // 清理布局转换的资源
    vkDestroyFence(device, layoutFence, nullptr);

}

uint32_t vulkanComputepipeline::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    // 自动寻找适合的内存类型
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

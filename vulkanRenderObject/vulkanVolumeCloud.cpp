#include <vulkanRenderObject/vulkanVolumeCloud.h>

vulkanVolumeCloud::vulkanVolumeCloud(VkDevice device, VkPhysicalDevice physicalDevice, VkExtent2D swapChainExtent, VkCommandPool commandPool,VkQueue graphicsQueue)
{
    this->device = device;
    this->physicalDevice = physicalDevice;
    this->swapChainExtent=swapChainExtent;
    this->commandPool=commandPool;
    this->graphicsQueue=graphicsQueue;
}

vulkanVolumeCloud::~vulkanVolumeCloud()
{
     std::cout<<"____"<<std::endl;
    std::cout<<"开始销毁：vulkanRenderObject"<<std::endl;
    vkDestroyDescriptorPool(device, object.descriptorPool, nullptr);
    vkDestroyImageView(device, this->object.volumeImageView, nullptr);
    vkDestroySampler(device, this->object.volumeSampler, nullptr);
    vkDestroyImage(device, this->object.volumeImage, nullptr);
    vkFreeMemory(device, this->object.volumeImageMemory, nullptr);
    std::cout<<"销毁结束"<<std::endl;
    std::cout<<"____"<<std::endl;
}

bool vulkanVolumeCloud::createCloudVolumeTexture(uint32_t width, uint32_t height, uint32_t depth)
{
    // this->object.width=width;
    // this->object.height=height;
    // this->object.depth=depth;
    // VolumeTextureGenerator generator;
    // std::cout<<"start densityData"<<std::endl;
    // // 生成RGBA纹理（密度+梯度）
    // auto densityData = generator.generateCloudTexture3D(
    //     width, height, depth);
    // std::cout<<"end densityData"<<std::endl;
    // // 创建Vulkan 3D图像
    // VkImageCreateInfo imageInfo{};
    // imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    // imageInfo.imageType = VK_IMAGE_TYPE_3D;
    // imageInfo.extent.width = width;
    // imageInfo.extent.height = height;
    // imageInfo.extent.depth = depth;
    // imageInfo.mipLevels = 1;
    // imageInfo.arrayLayers = 1;
    // imageInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT; 
    // imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    // imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    // imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT |
    //                   VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    // imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    // imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    // imageInfo.flags = 0;

    // if (vkCreateImage(device, &imageInfo, nullptr, &this->object.volumeImage) != VK_SUCCESS)
    // {
    //     std::cerr << "Failed to create 3D image!" << std::endl;
    //     return false;
    // }

    // // 分配内存
    // VkMemoryRequirements memRequirements;
    // vkGetImageMemoryRequirements(device, this->object.volumeImage, &memRequirements);

    // VkMemoryAllocateInfo allocInfo{};
    // allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    // allocInfo.allocationSize = memRequirements.size;
    // allocInfo.memoryTypeIndex = findMemoryType(
    //     memRequirements.memoryTypeBits,
    //     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    // if (vkAllocateMemory(device, &allocInfo, nullptr, &this->object.volumeImageMemory) != VK_SUCCESS)
    // {
    //     std::cerr << "Failed to allocate image memory!" << std::endl;
    //     return false;
    // }

    // vkBindImageMemory(device, this->object.volumeImage, this->object.volumeImageMemory, 0);

    // // 创建暂存缓冲区并上传数据
    // uploadVolumeData(densityData.data(), width * height * depth * 4 * sizeof(float));

    // // 创建图像视图
    // VkImageViewCreateInfo viewInfo{};
    // viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    // viewInfo.image = this->object.volumeImage;
    // viewInfo.viewType = VK_IMAGE_VIEW_TYPE_3D;
    // viewInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;
    // viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    // viewInfo.subresourceRange.baseMipLevel = 0;
    // viewInfo.subresourceRange.levelCount = 1;
    // viewInfo.subresourceRange.baseArrayLayer = 0;
    // viewInfo.subresourceRange.layerCount = 1;

    // viewInfo.components.r = VK_COMPONENT_SWIZZLE_R;
    // viewInfo.components.g = VK_COMPONENT_SWIZZLE_G;
    // viewInfo.components.b = VK_COMPONENT_SWIZZLE_B;
    // viewInfo.components.a = VK_COMPONENT_SWIZZLE_A;

    // if (vkCreateImageView(device, &viewInfo, nullptr, &this->object.volumeImageView) != VK_SUCCESS)
    // {
    //     std::cerr << "Failed to create 3D image view!" << std::endl;
    //     return false;
    // }

    // VkSamplerCreateInfo samplerInfo{};
    // samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    // samplerInfo.magFilter = VK_FILTER_LINEAR;
    // samplerInfo.minFilter = VK_FILTER_LINEAR;
    // samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    // samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    // samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    // samplerInfo.anisotropyEnable = VK_FALSE;
    // samplerInfo.maxAnisotropy = 1.0f;
    // samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    // samplerInfo.unnormalizedCoordinates = VK_FALSE;
    // samplerInfo.compareEnable = VK_FALSE;
    // samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    // samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    // samplerInfo.mipLodBias = 0.0f;
    // samplerInfo.minLod = 0.0f;
    // samplerInfo.maxLod = 0.0f;

    // if (vkCreateSampler(device, &samplerInfo, nullptr, &this->object.volumeSampler) != VK_SUCCESS)
    // {
    //     throw std::runtime_error("Failed to create texture sampler!");
    // }

     return true;
 
}

void vulkanVolumeCloud::uploadVolumeData(const void* data, VkDeviceSize dataSize) {
        // 1. 创建暂存缓冲区（CPU可见的内存）
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = dataSize;
        bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;  // 用作传输源
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        
        if (vkCreateBuffer(device, &bufferInfo, nullptr, &stagingBuffer) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create staging buffer!");
        }
        
        // 获取内存需求并分配
        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(device, stagingBuffer, &memRequirements);
        
        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(
            memRequirements.memoryTypeBits,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |  // CPU可见
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT   // 自动保持内存一致性
        );
        
        if (vkAllocateMemory(device, &allocInfo, nullptr, &stagingBufferMemory) != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate staging buffer memory!");
        }
        
        vkBindBufferMemory(device, stagingBuffer, stagingBufferMemory, 0);
        
        // 2. 将数据复制到暂存缓冲区
        void* mappedData;
        vkMapMemory(device, stagingBufferMemory, 0, dataSize, 0, &mappedData);
        memcpy(mappedData, data, (size_t)dataSize);
        vkUnmapMemory(device, stagingBufferMemory);
        
        // 3. 创建命令缓冲区执行复制操作
        VkCommandBufferAllocateInfo allocInfoCmd{};
        allocInfoCmd.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfoCmd.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfoCmd.commandPool = commandPool;
        allocInfoCmd.commandBufferCount = 1;
        
        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(device, &allocInfoCmd, &commandBuffer);
        
        // 开始记录命令
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        
        vkBeginCommandBuffer(commandBuffer, &beginInfo);
        
        // 3.1 先将图像布局转换为适合传输的布局
        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = this->object.volumeImage;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        
        vkCmdPipelineBarrier(commandBuffer,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,  // 在传输前
            VK_PIPELINE_STAGE_TRANSFER_BIT,     // 传输阶段
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier);
        
        // 3.2 从缓冲区复制到图像
        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;     // 紧密排列
        region.bufferImageHeight = 0;   // 紧密排列
        
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        
        region.imageOffset = {0, 0, 0};
        region.imageExtent = {this->object.width, this->object.height, this->object.depth};
        
        vkCmdCopyBufferToImage(commandBuffer,
            stagingBuffer,                // 源缓冲区
            this->object.volumeImage,                  // 目标图像
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,  // 当前布局
            1, &region);
        
        // 3.3 将图像布局转换为着色器读取布局
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        
        vkCmdPipelineBarrier(commandBuffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT,        // 传输阶段
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, // 片段着色器阶段
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier);
        
        vkEndCommandBuffer(commandBuffer);
        
        // 4. 提交命令缓冲区并等待完成
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;
        
        vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(graphicsQueue);  // 等待传输完成
        
        // 5. 清理资源
        vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
        vkDestroyBuffer(device, stagingBuffer, nullptr);
        vkFreeMemory(device, stagingBufferMemory, nullptr);
        
        std::cout << "Volume texture uploaded: " 
                  << dataSize << " bytes (" 
                  << this->object.width << "x" << this->object.height << "x" << this->object.depth << ")" 
                  << std::endl;


        
    }
void vulkanVolumeCloud::createDescriptorPool(uint32_t sampler_num)
{
    std::vector<VkDescriptorPoolSize> poolSizes;
    poolSizes.resize(5); 
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    poolSizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[2].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    poolSizes[3].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[3].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    poolSizes[4].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[4].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &object.descriptorPool) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

void vulkanVolumeCloud::createDescriptorSets(const VkDescriptorSetLayout &inDescriptorSetLayout, const vulkanUniformBuffer &meshUniformBuffer,const vulkanUniformBuffer &viewUniformBuffer)
{
    std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, inDescriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = object.descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    allocInfo.pSetLayouts = layouts.data();

    object.descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
    if (vkAllocateDescriptorSets(device, &allocInfo, object.descriptorSets.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {

        uint32_t write_size = 5;
        std::vector<VkWriteDescriptorSet> descriptorWrites{};
        descriptorWrites.resize(write_size);


        // 绑定 UnifromBuffer
        VkDescriptorBufferInfo bufferInfoOfMesh{};
        bufferInfoOfMesh.buffer = meshUniformBuffer.UniformBuffers[i];
        bufferInfoOfMesh.offset = 0;
        bufferInfoOfMesh.range = sizeof(UniformBufferVolumeCloud);

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = object.descriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfoOfMesh;

        VkDescriptorBufferInfo bufferInfoOfview{};
        bufferInfoOfview.buffer = viewUniformBuffer.UniformBuffers[i];
        bufferInfoOfview.offset = 0;
        bufferInfoOfview.range = sizeof(UniformBufferObjectView);

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = object.descriptorSets[i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pBufferInfo = &bufferInfoOfview;

        // 绑定2: 3D纹理采样器
        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        imageInfo.imageView = object.volumeImageView;  // 3D纹理图像视图
        imageInfo.sampler = object.volumeSampler;      // 3D纹理采样器
        
        // 注意：3D纹理是只读的，所以所有帧都使用相同的纹理
        // 如果动态更新纹理，可能需要每个帧不同的纹理

        descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[2].dstSet = object.descriptorSets[i];
        descriptorWrites[2].dstBinding = 2;
        descriptorWrites[2].dstArrayElement = 0;
        descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[2].descriptorCount = 1;
        descriptorWrites[2].pImageInfo = &imageInfo;
        descriptorWrites[2].pBufferInfo = nullptr;  // 不使用
        descriptorWrites[2].pTexelBufferView = nullptr;  // 不使用

        std::vector<VkDescriptorImageInfo> imageInfos;
        imageInfos.resize(weatherMap.size()+Noise.size());
        for (size_t j = 0; j < weatherMap.size(); j++)
        {
            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = weatherMap[j].TextureImageView;
            imageInfo.sampler = weatherMap[j].TextureSampler;
            imageInfos[j] = imageInfo;

            descriptorWrites[j + 3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[j + 3].dstSet = object.descriptorSets[i];
            descriptorWrites[j + 3].dstBinding = static_cast<uint32_t>(j + 3);
            descriptorWrites[j + 3].dstArrayElement = 0;
            descriptorWrites[j + 3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[j + 3].descriptorCount = 1;
            descriptorWrites[j + 3].pImageInfo = &imageInfos[j]; // 注意，这里是引用了VkDescriptorImageInfo，所有需要创建imageInfos这个数组，存储所有的imageInfo而不是使用局部变量imageInfo
        }

            for (size_t j = 0; j < Noise.size(); j++)
        {
            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = Noise[j].TextureImageView;
            imageInfo.sampler = Noise[j].TextureSampler;
            imageInfos[j+weatherMap.size()] = imageInfo;

            descriptorWrites[j + 3+weatherMap.size()].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[j + 3+weatherMap.size()].dstSet = object.descriptorSets[i];
            descriptorWrites[j + 3+weatherMap.size()].dstBinding = static_cast<uint32_t>(j + 3+weatherMap.size());
            descriptorWrites[j + 3+weatherMap.size()].dstArrayElement = 0;
            descriptorWrites[j + 3+weatherMap.size()].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[j + 3+weatherMap.size()].descriptorCount = 1;
            descriptorWrites[j + 3+weatherMap.size()].pImageInfo = &imageInfos[j+weatherMap.size()]; // 注意，这里是引用了VkDescriptorImageInfo，所有需要创建imageInfos这个数组，存储所有的imageInfo而不是使用局部变量imageInfo
        }
std::cout<<"!"<<std::endl;

        vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }
}

void vulkanVolumeCloud::createStageRenderResource(const VkDescriptorSetLayout &descriptorSetLayout,const vulkanUniformBuffer &meshUniformBuffer, const vulkanUniformBuffer &viewUniformBuffer)
{

    createDescriptorPool();
    
    createDescriptorSets(descriptorSetLayout,meshUniformBuffer,viewUniformBuffer);

}

uint32_t vulkanVolumeCloud::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
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

VkShaderModule vulkanVolumeCloud::createShaderModule(const std::vector<char> &code)
{
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create shader module!");
    }

    return shaderModule;
}
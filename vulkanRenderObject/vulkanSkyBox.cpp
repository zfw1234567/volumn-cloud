#include <vulkanRenderObject/vulkanSkyBox.h>

vulkanSkyBox::vulkanSkyBox(VkDevice device, VkPhysicalDevice physicalDevice,VkExtent2D swapChainExtent, VkCommandPool commandPool,VkQueue graphicsQueue)
{
    this->device=device;
    this->physicalDevice=physicalDevice;
    this->swapChainExtent=swapChainExtent;
    this->commandPool=commandPool;
    this->graphicsQueue=graphicsQueue;
    this->cubemap.setcontext(device,physicalDevice,swapChainExtent,commandPool,graphicsQueue);
}

vulkanSkyBox::~vulkanSkyBox()
{
    vkDestroyDescriptorPool(device, descriptorPool, nullptr);
}

void vulkanSkyBox::createCubeMap(const std::vector<std::string> &filenames)
{
    this->cubemap.createImageHDRContext(filenames);
}

void vulkanSkyBox::createDescriptorPool(uint32_t sampler_num)
{
    std::vector<VkDescriptorPoolSize> poolSizes;
    poolSizes.resize(2); 
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);


    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

void vulkanSkyBox::createDescriptorSets(const VkDescriptorSetLayout &inDescriptorSetLayout, const vulkanUniformBuffer &meshUniformBuffer,const vulkanUniformBuffer &viewUniformBuffer)
{
    std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, inDescriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    allocInfo.pSetLayouts = layouts.data();

    descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
    if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {

        uint32_t write_size = 2;
        std::vector<VkWriteDescriptorSet> descriptorWrites{};
        descriptorWrites.resize(write_size);


        // 绑定 UnifromBuffer
        VkDescriptorBufferInfo bufferInfoOfMesh{};
        bufferInfoOfMesh.buffer = meshUniformBuffer.UniformBuffers[i];
        bufferInfoOfMesh.offset = 0;
        bufferInfoOfMesh.range = sizeof(UniformBufferObject);

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = descriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfoOfMesh;

        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = cubemap.ImageView;  
        imageInfo.sampler = cubemap.Sampler;      
        

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = descriptorSets[i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pImageInfo = &imageInfo;
        descriptorWrites[1].pBufferInfo = nullptr;  // 不使用
        descriptorWrites[1].pTexelBufferView = nullptr;  // 不使用

        vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }
}

void vulkanSkyBox::createStageRenderResource(const VkDescriptorSetLayout &descriptorSetLayout,const vulkanUniformBuffer &meshUniformBuffer, const vulkanUniformBuffer &viewUniformBuffer)
{

    createDescriptorPool();
    createDescriptorSets(descriptorSetLayout,meshUniformBuffer,viewUniformBuffer);

}

uint32_t vulkanSkyBox::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
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
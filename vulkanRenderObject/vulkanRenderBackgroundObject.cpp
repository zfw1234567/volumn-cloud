#include <vulkanRenderObject/vulkanRenderBackgroundObject.h>

vulkanRenderBackgroundObject::vulkanRenderBackgroundObject(VkDevice device, VkPhysicalDevice physicalDevice,VkExtent2D swapChainExtent, VkCommandPool commandPool, VkQueue graphicsQueue)
{
    this->device = device;
    this->physicalDevice = physicalDevice;
    this->swapChainExtent=swapChainExtent;
    this->commandPool=commandPool;
    this->graphicsQueue=graphicsQueue;
}

vulkanRenderBackgroundObject::~vulkanRenderBackgroundObject()
{
    std::cout<<"____"<<std::endl;
    std::cout<<"开始销毁：vulkanRenderBackgroundObject"<<std::endl;
    vkDestroyDescriptorPool(device, object.descriptorPool, nullptr);
    std::cout<<"销毁结束"<<std::endl;
    std::cout<<"____"<<std::endl;
}

void vulkanRenderBackgroundObject::createStageRenderResource( const std::vector<std::string> &pngfiles, const VkDescriptorSetLayout &descriptorSetLayout,const vulkanUniformBuffer &meshUniformBuffer, const vulkanUniformBuffer &viewUniformBuffer,const vulkancubemaperesource &cubemap)
{

    object.TextureImages.reserve(pngfiles.size());
    for (size_t i = 0; i < pngfiles.size(); i++)
    {
        // 一个便捷函数，创建图像，视口和采样器
        bool sRGB = (i == 0);
        object.TextureImages.emplace_back(
        device, physicalDevice, swapChainExtent, commandPool, graphicsQueue);
        object.TextureImages.back().createImageContext(pngfiles[i], sRGB);
    }

    createDescriptorPool(static_cast<uint32_t>(pngfiles.size() + 1));
    createDescriptorSets(descriptorSetLayout,meshUniformBuffer, viewUniformBuffer, cubemap);
}

/** 从文件中读取顶点和点序*/
void vulkanRenderBackgroundObject::createVertices(std::vector<Vertex> &outVertices, std::vector<uint32_t> &outIndices, const std::string &filename)
{
    readModelResource(filename, outVertices, outIndices);
}

void vulkanRenderBackgroundObject::readModelResource(const std::string &filename, std::vector<Vertex> &vertices, std::vector<uint32_t> &indices)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename.c_str()))
    {
        throw std::runtime_error(warn + err);
    }

    std::unordered_map<Vertex, uint32_t> uniqueVertices{};

    for (const auto &shape : shapes)
    {
        for (const auto &index : shape.mesh.indices)
        {
            Vertex vertex{};

            vertex.position = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]};

            vertex.normal = {
                attrib.normals[3 * index.vertex_index + 0],
                attrib.normals[3 * index.vertex_index + 1],
                attrib.normals[3 * index.vertex_index + 2]};

            vertex.color = {1.0f, 1.0f, 1.0f};

            vertex.texCoord = {
                attrib.texcoords[2 * index.texcoord_index + 0],
                1.0f - attrib.texcoords[2 * index.texcoord_index + 1]};

            if (uniqueVertices.count(vertex) == 0)
            {
                uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(vertex);
            }

            indices.push_back(uniqueVertices[vertex]);
        }
    }
}



void vulkanRenderBackgroundObject::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &bufferMemory)
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create buffer!");
    }

    // 为VertexBuffer创建内存，并赋予
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    // 自动找到适合的内存类型
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);
    // 关联分配的内存地址
    if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate buffer memory!");
    }
    // 绑定VertexBuffer和它的内存地址
    vkBindBufferMemory(device, buffer, bufferMemory, 0);
}

/** 查找内存类型*/
uint32_t vulkanRenderBackgroundObject::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
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

/** 通用函数用来拷贝Buffer*/
void vulkanRenderBackgroundObject::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    endSingleTimeCommands(commandBuffer);
}

VkCommandBuffer vulkanRenderBackgroundObject::beginSingleTimeCommands()
{
    // 和渲染一样，使用CommandBuffer拷贝缓存
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void vulkanRenderBackgroundObject::endSingleTimeCommands(VkCommandBuffer commandBuffer)
{
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphicsQueue);

    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

/** 通用函数用来创建DescriptorPool
 * outDescriptorPool ：输出的DescriptorPool
 * sampler_num ：贴图采样器的数量
 */
void vulkanRenderBackgroundObject::createDescriptorPool(uint32_t sampler_num)
{
    std::vector<VkDescriptorPoolSize> poolSizes;
    poolSizes.resize(sampler_num + 3); // 这里3是2个UniformBuffer和一个环境Cubemap贴图
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[2].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    for (size_t i = 0; i < sampler_num; i++)
    {
        poolSizes[i + 3].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSizes[i + 3].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    }

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

/** 通用函数用来创建DescriptorSets*/
void vulkanRenderBackgroundObject::createDescriptorSets(const VkDescriptorSetLayout &inDescriptorSetLayout,const vulkanUniformBuffer &meshUniformBuffer, const vulkanUniformBuffer &viewUniformBuffer,const vulkancubemaperesource &cubemap)
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

        uint32_t write_size = static_cast<uint32_t>(object.TextureImages.size()) + 3; // 这里加2为 UniformBuffer 的个数
        std::vector<VkWriteDescriptorSet> descriptorWrites{};
        descriptorWrites.resize(write_size);

        // 绑定 UnifromBuffer
        VkDescriptorBufferInfo bufferInfoOfMesh{};
        bufferInfoOfMesh.buffer = meshUniformBuffer.UniformBuffers[i];
        bufferInfoOfMesh.offset = 0;
        bufferInfoOfMesh.range = sizeof(UniformBufferObject);

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = object.descriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfoOfMesh;

        // 绑定 UnifromBuffer
        VkDescriptorBufferInfo bufferInfoOfView{};
        bufferInfoOfView.buffer = viewUniformBuffer.UniformBuffers[i];
        bufferInfoOfView.offset = 0;
        bufferInfoOfView.range = sizeof(UniformBufferObjectView);

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = object.descriptorSets[i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pBufferInfo = &bufferInfoOfView;

        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = cubemap.ImageView;
        imageInfo.sampler = cubemap.Sampler;

        descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[2].dstSet = object.descriptorSets[i];
        descriptorWrites[2].dstBinding = 2;
        descriptorWrites[2].dstArrayElement = 0;
        descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[2].descriptorCount = 1;
        descriptorWrites[2].pImageInfo = &imageInfo;

        // 绑定 Textures
        // descriptorWrites会引用每一个创建的VkDescriptorImageInfo，所以需要用一个数组把它们存储起来
        std::vector<VkDescriptorImageInfo> imageInfos;
        imageInfos.resize(object.TextureImages.size());
        for (size_t j = 0; j < object.TextureImages.size(); j++)
        {
            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = object.TextureImages[j].TextureImageView;
            imageInfo.sampler = object.TextureImages[j].TextureSampler;
            imageInfos[j] = imageInfo;

            descriptorWrites[j + 3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[j + 3].dstSet = object.descriptorSets[i];
            descriptorWrites[j + 3].dstBinding = static_cast<uint32_t>(j + 3);
            descriptorWrites[j + 3].dstArrayElement = 0;
            descriptorWrites[j + 3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[j + 3].descriptorCount = 1;
            descriptorWrites[j + 3].pImageInfo = &imageInfos[j]; // 注意，这里是引用了VkDescriptorImageInfo，所有需要创建imageInfos这个数组，存储所有的imageInfo而不是使用局部变量imageInfo
        }

        vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }
}

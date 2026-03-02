#include <vulkanRender/vulkanDescriptorSetLayout.h>

vulkanDescriptorSetLayout::vulkanDescriptorSetLayout(VkDevice device)
{
    this->device=device;
}

vulkanDescriptorSetLayout::~vulkanDescriptorSetLayout()
{
    vkDestroyDescriptorSetLayout(device, outDescriptorSetLayout, nullptr);
    
}

/** 通用函数用来创建DescriptorSetLayout*/
void vulkanDescriptorSetLayout::createDescriptorSetLayout(uint32_t sampler_number)
{
    // UnifromBufferObject（ubo）绑定
    VkDescriptorSetLayoutBinding meshUBOLayoutBinding{};
    meshUBOLayoutBinding.binding = 0;
    meshUBOLayoutBinding.descriptorCount = 1;
    meshUBOLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    meshUBOLayoutBinding.pImmutableSamplers = nullptr;
    meshUBOLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    // UnifromBufferObject（ubo）绑定
    VkDescriptorSetLayoutBinding viewUBOLayoutBinding{};
    viewUBOLayoutBinding.binding = 1;
    viewUBOLayoutBinding.descriptorCount = 1;
    viewUBOLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    viewUBOLayoutBinding.pImmutableSamplers = nullptr;
    viewUBOLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT; // view ubo 主要信息用于 fragment shader

    // 环境反射Cubemap贴图绑定
    VkDescriptorSetLayoutBinding cubemapLayoutBinding{};
    cubemapLayoutBinding.binding = 2;
    cubemapLayoutBinding.descriptorCount = 1;
    cubemapLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    cubemapLayoutBinding.pImmutableSamplers = nullptr;
    cubemapLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    // 将UnifromBufferObject和贴图采样器绑定到DescriptorSetLayout上
    std::vector<VkDescriptorSetLayoutBinding> bindings;
    bindings.resize(sampler_number + 3); // 这里3是2个UniformBuffer和一个环境Cubemap贴图
    bindings[0] = meshUBOLayoutBinding;
    bindings[1] = viewUBOLayoutBinding;
    bindings[2] = cubemapLayoutBinding;
    for (size_t i = 0; i < sampler_number; i++)
    {
        VkDescriptorSetLayoutBinding samplerLayoutBinding{};
        samplerLayoutBinding.binding = static_cast<uint32_t>(i + 3);
        samplerLayoutBinding.descriptorCount = 1;
        samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerLayoutBinding.pImmutableSamplers = nullptr;
        samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        bindings[i + 3] = samplerLayoutBinding;
    }
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();
    if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &outDescriptorSetLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

void vulkanDescriptorSetLayout::createVolumecloudLayout()
{
std::array<VkDescriptorSetLayoutBinding, 5> bindings{};
    
    // 绑定0: Uniform Buffer
    bindings[0].binding = 0;
    bindings[0].descriptorCount = 1;
    bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bindings[0].pImmutableSamplers = nullptr;
    bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | 
                             VK_SHADER_STAGE_FRAGMENT_BIT;
    
    bindings[1].binding = 1;
    bindings[1].descriptorCount = 1;
    bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bindings[1].pImmutableSamplers = nullptr;
    bindings[1].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | 
                             VK_SHADER_STAGE_FRAGMENT_BIT;
    
    // 绑定1: 3D纹理采样器
    bindings[2].binding = 2;
    bindings[2].descriptorCount = 1;
    bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindings[2].pImmutableSamplers = nullptr;
    bindings[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;  // 片段着色器使用3D纹理

    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.binding = 3;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    bindings[3] = samplerLayoutBinding;

    bindings[4].binding = 4;
    bindings[4].descriptorCount = 1;
    bindings[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindings[4].pImmutableSamplers = nullptr;
    bindings[4].stageFlags =  VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();
    
    if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &outDescriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create Volume descriptor set layout!");
    }
}

void vulkanDescriptorSetLayout::createskyboxLayout()
{
std::array<VkDescriptorSetLayoutBinding, 2> bindings{};
    
    // 绑定0: Uniform Buffer
    bindings[0].binding = 0;
    bindings[0].descriptorCount = 1;
    bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bindings[0].pImmutableSamplers = nullptr;
    bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | 
                             VK_SHADER_STAGE_FRAGMENT_BIT;
    
    bindings[1].binding = 1;
    bindings[1].descriptorCount = 1;
    bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindings[1].pImmutableSamplers = nullptr;
    bindings[1].stageFlags =VK_SHADER_STAGE_FRAGMENT_BIT;
    
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();
    
    if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &outDescriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create Volume descriptor set layout!");
    }
}

void vulkanDescriptorSetLayout::createcompositeLayout()
{
    std::array<VkDescriptorSetLayoutBinding, 5> bindings{};
    
    bindings[0].binding = 0;
    bindings[0].descriptorCount = 1;
    bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindings[0].pImmutableSamplers = nullptr;
    bindings[0].stageFlags =VK_SHADER_STAGE_FRAGMENT_BIT;

    bindings[1].binding = 1;
    bindings[1].descriptorCount = 1;
    bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindings[1].pImmutableSamplers = nullptr;
    bindings[1].stageFlags =VK_SHADER_STAGE_FRAGMENT_BIT;

    bindings[2].binding = 2;
    bindings[2].descriptorCount = 1;
    bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindings[2].pImmutableSamplers = nullptr;
    bindings[2].stageFlags =VK_SHADER_STAGE_FRAGMENT_BIT;

    bindings[3].binding = 3;
    bindings[3].descriptorCount = 1;
    bindings[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindings[3].pImmutableSamplers = nullptr;
    bindings[3].stageFlags =VK_SHADER_STAGE_FRAGMENT_BIT;

    bindings[4].binding = 4;
    bindings[4].descriptorCount = 1;
    bindings[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindings[4].pImmutableSamplers = nullptr;
    bindings[4].stageFlags =VK_SHADER_STAGE_FRAGMENT_BIT;
    
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();
    
    if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &outDescriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create Volume descriptor set layout!");
    }

}
//
//  texture.c
//  AgeOfAnarchyXCode
//
//  Created by Emmanuel Mera on 10/03/2023.
//

#include <string.h>
#include <stdlib.h>

#include "texture.h"
#include "init.h"
#include "renderer.h"
#include "descriptor_set.h"

#include "MEM_alloc.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

//-----------------------------------------------//
//-     Local variables                         -//
//-----------------------------------------------//

static VkCommandPool pool;

//-----------------------------------------------//
//-     Local functions                         -//
//-----------------------------------------------//

static void print_error(char str[]);
static void print_warning(char str[]);
static uint32_t *get_source_code(const char *file, size_t *size);

//-----------------------------------------------//
//-     Implementation                          -//
//-----------------------------------------------//

int createTexturedPipeline(Window *window) {
    VkShaderModuleCreateInfo vertShaderModuleInfo;
    memset(&vertShaderModuleInfo, 0, sizeof(VkShaderModuleCreateInfo));
    vertShaderModuleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    vertShaderModuleInfo.pCode = get_source_code("../Resources/texture_vert.spv", &vertShaderModuleInfo.codeSize);
    VkShaderModule vertShaderModule;
    if (vkCreateShaderModule(window->device, &vertShaderModuleInfo, NULL, &vertShaderModule) != VK_SUCCESS) {
        print_error("unabled to create a shader module");
        MEM_free((void*)vertShaderModuleInfo.pCode);
        return 0;
    }
    MEM_free((void*)vertShaderModuleInfo.pCode);
    
    VkPipelineShaderStageCreateInfo vertShaderInfo;
    memset(&vertShaderInfo, 0, sizeof(VkPipelineShaderStageCreateInfo));
    vertShaderInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderInfo.pName = "main";
    vertShaderInfo.module = vertShaderModule;
    
    VkShaderModuleCreateInfo fragShaderModuleInfo;
    memset(&fragShaderModuleInfo, 0, sizeof(VkShaderModuleCreateInfo));
    fragShaderModuleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    fragShaderModuleInfo.pCode = get_source_code("../Resources/texture_frag.spv", &fragShaderModuleInfo.codeSize);
    VkShaderModule fragShaderModule;
    if (vkCreateShaderModule(window->device, &fragShaderModuleInfo, NULL, &fragShaderModule) != VK_SUCCESS) {
        print_error("unabled to create a shader module");
        vkDestroyShaderModule(window->device, vertShaderModule, NULL);
        MEM_free((void*)fragShaderModuleInfo.pCode);
        return 0;
    }
    MEM_free((void*)fragShaderModuleInfo.pCode);
    
    VkPipelineShaderStageCreateInfo fragShaderInfo;
    memset(&fragShaderInfo, 0, sizeof(VkPipelineShaderStageCreateInfo));
    fragShaderInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderInfo.pName = "main";
    fragShaderInfo.module = fragShaderModule;
    
    VkPipelineShaderStageCreateInfo shaders[2] = {vertShaderInfo, fragShaderInfo};
    
    VkVertexInputBindingDescription bindingDescription;
    bindingDescription.binding = 0;
    bindingDescription.stride = 8*sizeof(float);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    
    VkVertexInputAttributeDescription posAttribute;
    posAttribute.binding = 0;
    posAttribute.location = 0;
    posAttribute.offset = 0;
    posAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
    
    VkVertexInputAttributeDescription normalAttribute;
    normalAttribute.binding = 0;
    normalAttribute.location = 1;
    normalAttribute.offset = 3*sizeof(float);
    normalAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
    
    VkVertexInputAttributeDescription texAttribute;
    texAttribute.binding = 0;
    texAttribute.location = 2;
    texAttribute.offset = 6*sizeof(float);
    texAttribute.format = VK_FORMAT_R32G32_SFLOAT;
    
    VkVertexInputAttributeDescription attributes[] = {posAttribute, normalAttribute, texAttribute};
    
    VkPipelineVertexInputStateCreateInfo vertexInfo;
    memset(&vertexInfo, 0, sizeof(VkPipelineVertexInputStateCreateInfo));
    vertexInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInfo.vertexBindingDescriptionCount = 1;
    vertexInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInfo.vertexAttributeDescriptionCount = 3;
    vertexInfo.pVertexAttributeDescriptions = attributes;
    
    VkPipelineInputAssemblyStateCreateInfo assemblyInfo;
    memset(&assemblyInfo, 0, sizeof(VkPipelineInputAssemblyStateCreateInfo));
    assemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    assemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    assemblyInfo.primitiveRestartEnable = VK_FALSE;
    
    VkPipelineViewportStateCreateInfo viewportInfo;
    memset(&viewportInfo, 0, sizeof(VkPipelineViewportStateCreateInfo));
    viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportInfo.viewportCount = 1;
    viewportInfo.scissorCount = 1;
    
    VkPipelineRasterizationStateCreateInfo rasterizationInfo;
    memset(&rasterizationInfo, 0, sizeof(rasterizationInfo));
    rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationInfo.depthBiasClamp = VK_FALSE;
    rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
    rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizationInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizationInfo.depthBiasEnable = VK_FALSE;
    rasterizationInfo.lineWidth = 1.f;
    
    VkPipelineMultisampleStateCreateInfo multisampleInfo;
    memset(&multisampleInfo, 0, sizeof(VkPipelineMultisampleStateCreateInfo));
    multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampleInfo.sampleShadingEnable = VK_FALSE;
    
    VkPipelineColorBlendAttachmentState colorBlendAttachment;
    memset(&colorBlendAttachment, 0, sizeof(VkPipelineColorBlendAttachmentState));
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_A_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_R_BIT;
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_SUBTRACT;
    
    VkPipelineColorBlendStateCreateInfo colorBlendInfo;
    memset(&colorBlendInfo, 0, sizeof(VkPipelineColorBlendStateCreateInfo));
    colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendInfo.logicOpEnable = VK_FALSE;
    colorBlendInfo.attachmentCount = 1;
    colorBlendInfo.pAttachments = &colorBlendAttachment;
    
    VkPipelineDynamicStateCreateInfo dynamicStateInfo;
    memset(&dynamicStateInfo, 0, sizeof(VkPipelineDynamicStateCreateInfo));
    dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicStateInfo.dynamicStateCount = 4;
    VkDynamicState dynamicStates[] = {VK_DYNAMIC_STATE_SCISSOR, VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_STENCIL_REFERENCE, VK_DYNAMIC_STATE_STENCIL_WRITE_MASK};
    dynamicStateInfo.pDynamicStates = dynamicStates;
    
    VkPushConstantRange constantRange;
    constantRange.size = 21*sizeof(float);
    constantRange.offset = 0;
    constantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    
    VkDescriptorSetLayoutBinding textureBinding;
    textureBinding.binding = 0;
    textureBinding.descriptorCount = 1;
    textureBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    textureBinding.pImmutableSamplers = NULL;
    textureBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    
    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo;
    memset(&descriptorSetLayoutInfo, 0, sizeof(VkDescriptorSetLayoutCreateInfo));
    descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutInfo.bindingCount = 1;
    descriptorSetLayoutInfo.pBindings = &textureBinding;
    
    VkDescriptorSetLayout descriptorSetLayout;
    if (vkCreateDescriptorSetLayout(window->device, &descriptorSetLayoutInfo, NULL, &descriptorSetLayout) != VK_SUCCESS) {
        return 0;
    }
    
    window->texDescriptorSetLayout = descriptorSetLayout;
    
    VkPipelineLayoutCreateInfo layoutInfo;
    memset(&layoutInfo, 0, sizeof(VkPipelineLayoutCreateInfo));
    layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutInfo.setLayoutCount = 1;
    layoutInfo.pSetLayouts = &descriptorSetLayout;
    layoutInfo.pushConstantRangeCount = 1;
    layoutInfo.pPushConstantRanges = &constantRange;
    VkPipelineLayout layout;
    if (vkCreatePipelineLayout(window->device, &layoutInfo, NULL, &layout) != VK_SUCCESS) {
        print_error("unabled to create the pipeline layout");
        return 0;
    }
    window->texturePipelineLayout = layout;
    
    VkPipelineDepthStencilStateCreateInfo depthStencil;
    memset(&depthStencil, 0, sizeof(VkPipelineDepthStencilStateCreateInfo));
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    
    VkStencilOpState stencilState;
    memset(&stencilState, 0, sizeof(VkStencilOpState));
    stencilState.failOp = VK_STENCIL_OP_KEEP;
    stencilState.passOp = VK_STENCIL_OP_INCREMENT_AND_CLAMP;
    stencilState.depthFailOp = VK_STENCIL_OP_KEEP;
    stencilState.compareOp = VK_COMPARE_OP_GREATER_OR_EQUAL;
    stencilState.compareMask = 1;
    stencilState.writeMask = 1;
    stencilState.reference = 1;
    
    depthStencil.front = stencilState;
    
    VkGraphicsPipelineCreateInfo pipelineInfo;
    memset(&pipelineInfo, 0, sizeof(VkGraphicsPipelineCreateInfo));
    pipelineInfo.sType =                VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount =           2;
    pipelineInfo.pStages =              shaders;
    pipelineInfo.pVertexInputState =    &vertexInfo;
    pipelineInfo.pInputAssemblyState =  &assemblyInfo;
    pipelineInfo.pViewportState =       &viewportInfo;
    pipelineInfo.pRasterizationState =  &rasterizationInfo;
    pipelineInfo.pMultisampleState =    &multisampleInfo;
    pipelineInfo.pColorBlendState =     &colorBlendInfo;
    pipelineInfo.pDynamicState =        &dynamicStateInfo;
    pipelineInfo.pDepthStencilState =   &depthStencil;
    pipelineInfo.layout =               layout;
    pipelineInfo.renderPass =           window->renderPass;
    pipelineInfo.basePipelineHandle =   VK_NULL_HANDLE;
    
    if (vkCreateGraphicsPipelines(window->device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &window->texturePipeline) != VK_SUCCESS) {
        print_error("unabled to create the graphic pipeline");
        vkDestroyShaderModule(window->device, vertShaderModule, NULL);
        vkDestroyShaderModule(window->device, fragShaderModule, NULL);
        return 0;
    }
    
    vkDestroyShaderModule(window->device, vertShaderModule, NULL);
    vkDestroyShaderModule(window->device, fragShaderModule, NULL);
    
    window->descriptorPool = createDescriptorPool(window);
    window->sampler = createImageSampler(window);
    
    // creation of the pool for transfert operation
    // WILL move later on but fine for now
    VkCommandPoolCreateInfo poolInfo;
    memset(&poolInfo, 0, sizeof(VkCommandPoolCreateInfo));
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = window->transferQueueFamily.family;
    vkCreateCommandPool(window->device, &poolInfo, NULL, &pool);
    
    return 1;
}

void destroyTexturePipeline(Window *window) {
    vkDestroyCommandPool(window->device, pool, NULL);
    vkDestroyDescriptorSetLayout(window->device, window->texDescriptorSetLayout, NULL);
    vkDestroySampler(window->device, window->sampler, NULL);
    vkDestroyPipelineLayout(window->device, window->texturePipelineLayout, NULL);
    destroyDescriptorPool(window, window->descriptorPool);
    vkDestroyPipeline(window->device, window->texturePipeline, NULL);
}

// TODO: refactor the code please !!!!
Texture *createTexture(Window *window, unsigned char *image, int x, int y, int channel) {
    VkDeviceSize texSize = x*y*channel;
    
    Buffer stagingBuffer;
    createBuffer(window, texSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, &stagingBuffer);
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(window->device, stagingBuffer.buffer, &memRequirements);
    MemBlock *stagingBufferMem = allocVulkanMemory(window, &memRequirements, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, BUFFER_MEMORY);
    bindBuffer(window, stagingBuffer, stagingBufferMem);
    
    void *data;
    vkMapMemory(window->device, getBindingPoint(stagingBufferMem), getOffset(stagingBufferMem), texSize, 0, &data);
        memcpy(data, image, texSize);
    vkUnmapMemory(window->device, getBindingPoint(stagingBufferMem));
    
    Texture *tex = MEM_malloc(sizeof(Texture), __func__);
    VkImageCreateInfo imageInfo;
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.pNext = NULL;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = x;
    imageInfo.extent.height = y;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.flags = 0;
    
    if (vkCreateImage(window->device, &imageInfo, NULL, &tex->image) != VK_SUCCESS) {
        MEM_free(tex);
        return NULL;
    }
    
    vkGetImageMemoryRequirements(window->device, tex->image, &memRequirements);
    tex->mem = allocVulkanMemory(window, &memRequirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, IMAGE_MEMORY);
    
    vkBindImageMemory(window->device, tex->image, getBindingPoint(tex->mem), getOffset(tex->mem));
    
    VkCommandBufferAllocateInfo allocInfo;
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.pNext = NULL;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = pool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(window->device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo;
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    beginInfo.pNext = NULL;
    
    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    
    VkImageMemoryBarrier barrier;
    memset(&barrier, 0, sizeof(VkImageMemoryBarrier));
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = tex->image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = 0;
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    
    vkCmdPipelineBarrier(commandBuffer,
                         VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                         0,
                         0, NULL,
                         0, NULL,
                         1, &barrier
    );
    
    vkEndCommandBuffer(commandBuffer);
    
    VkSubmitInfo submitInfo;
    memset(&submitInfo, 0, sizeof(VkSubmitInfo));
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    
    vkQueueSubmit(window->transferQueueFamily.queues[0], 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(window->transferQueueFamily.queues[0]);

    vkFreeCommandBuffers(window->device, pool, 1, &commandBuffer);
    
    vkAllocateCommandBuffers(window->device, &allocInfo, &commandBuffer);
    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    
    VkBufferImageCopy region;
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = (VkOffset3D){0, 0, 0};
    region.imageExtent = (VkExtent3D){x, y, 1};
    
    vkCmdCopyBufferToImage(commandBuffer, stagingBuffer.buffer, tex->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    
    vkEndCommandBuffer(commandBuffer);
    
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(window->transferQueueFamily.queues[0], 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(window->transferQueueFamily.queues[0]);

    vkFreeCommandBuffers(window->device, pool, 1, &commandBuffer);
    
    destroyBuffer(window, stagingBuffer);
    freeVulkanMemory(stagingBufferMem);
    
    vkAllocateCommandBuffers(window->device, &allocInfo, &commandBuffer);
    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = tex->image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    
    vkCmdPipelineBarrier(commandBuffer,
                         VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                         0,
                         0, NULL,
                         0, NULL,
                         1, &barrier
    );
    
    vkEndCommandBuffer(commandBuffer);
    
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    
    vkQueueSubmit(window->transferQueueFamily.queues[0], 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(window->transferQueueFamily.queues[0]);
    
    vkFreeCommandBuffers(window->device, pool, 1, &commandBuffer);
    
    VkImageViewCreateInfo viewInfo;
    memset(&viewInfo, 0, sizeof(VkImageViewCreateInfo));
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = tex->image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;
    
    if (vkCreateImageView(window->device, &viewInfo, NULL, &tex->view) != VK_SUCCESS) {
        vkDestroyImage(window->device, tex->image, NULL);
        freeVulkanMemory(tex->mem);
        MEM_free(tex);
        return NULL;
    }
    
    createDescriptorSet(window, window->descriptorPool, (VkDescriptorSetLayout[2]){window->texDescriptorSetLayout, window->texDescriptorSetLayout}, 2, tex->descriptorSets);
    
    for (int i=0; i<MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorImageInfo imageInfo;
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = tex->view;
        imageInfo.sampler = window->sampler;
        
        VkWriteDescriptorSet descriptorWrite;
        memset(&descriptorWrite, 0, sizeof(descriptorWrite));
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = tex->descriptorSets[i];
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pImageInfo = &imageInfo;
        
        vkUpdateDescriptorSets(window->device, 1, &descriptorWrite, 0, NULL);
    }
    
    return tex;
}

Texture *loadTexture(Window *window, char filename[]) {
    int x, y, n;
    unsigned char *texImage = stbi_load(filename, &x, &y, &n, STBI_rgb_alpha);
    Texture *tex = createTexture(window, texImage, x, y, 4);
    stbi_image_free(texImage);
    return tex;
}

void destroyTexture(Window *window, Texture *tex) {
    vkDestroyImageView(window->device, tex->view, NULL);
    vkDestroyImage(window->device, tex->image, NULL);
    freeVulkanMemory(tex->mem);
    MEM_free(tex);
}

VkSampler createImageSampler(Window *window) {
    VkSamplerCreateInfo samplerInfo;
    memset(&samplerInfo, 0, sizeof(VkSamplerCreateInfo));
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.anisotropyEnable = VK_FALSE; // TODO: change this to true when anisotropy feature enabled
    
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(window->physicalDevice, &properties);
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;
    
    VkSampler sampler;
    if (vkCreateSampler(window->device, &samplerInfo, NULL, &sampler) != VK_SUCCESS) {
        return VK_NULL_HANDLE;
    }
    
    return sampler;
}

//-----------------------------------------------//
//-     Local functions                         -//
//-----------------------------------------------//

uint32_t *get_source_code(const char *file, size_t *size) {
    FILE *f = fopen(file, "rb");
    fseek(f, 0, SEEK_END);
    *size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    uint32_t *content = MEM_calloc_array(*size, sizeof(char), __func__);
    fread(content, 1, *size, f);
    
    fclose(f);
    return content;
}

void print_error(char str[]) {
    fprintf(stderr, "[ \033[31mERROR\033[29m ] : initialization : %s\n", str);
}

void print_warning(char str[]) {
#ifdef DEBUG
    fprintf(stderr, "[ \033[32mWARNING\033[29m ] : initialization : %s\n", str);
#endif
}

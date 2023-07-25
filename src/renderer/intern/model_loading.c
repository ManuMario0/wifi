//
//  model_loading.c
//  AgeOfAnarchy
//
//  Created by Emmanuel Mera on 19/02/2023.
//

#include "model_loading.h"
#include "MEM_alloc.h"

#include <stdlib.h>
#include <string.h>

ModelBuffer *createModelBuffer(Window *window, size_t vertexSize, void *vertices, size_t indexSize, void *indices) {
    ModelBuffer *model = MEM_malloc(sizeof(ModelBuffer), __func__);
    
    VkCommandPoolCreateInfo poolInfo;
    memset(&poolInfo, 0, sizeof(VkCommandPoolCreateInfo));
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = window->transferQueueFamily.family;
    VkCommandPool pool;
    vkCreateCommandPool(window->device, &poolInfo, NULL, &pool);
    
    Buffer stagingBuffer;
    createBuffer(window, vertexSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, &stagingBuffer);
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(window->device, stagingBuffer.buffer, &memRequirements);
    MemBlock *stagingBufferMem = allocVulkanMemory(window, &memRequirements, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, BUFFER_MEMORY);
    bindBuffer(window, stagingBuffer, stagingBufferMem);
    
    void* data;
    vkMapMemory(window->device, getBindingPoint(stagingBufferMem), getOffset(stagingBufferMem), vertexSize, 0, &data);
        memcpy(data, vertices, vertexSize);
    vkUnmapMemory(window->device, getBindingPoint(stagingBufferMem));
    
    createBuffer(window, vertexSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, &model->vertexBuffer);
    vkGetBufferMemoryRequirements(window->device, model->vertexBuffer.buffer, &memRequirements);
    model->vertMemory = allocVulkanMemory(window, &memRequirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, BUFFER_MEMORY);
    bindBuffer(window, model->vertexBuffer, model->vertMemory);
    
    copyBuffer(window, &window->transferQueueFamily, pool, stagingBuffer.buffer, model->vertexBuffer.buffer, vertexSize);
    
    destroyBuffer(window, stagingBuffer);
    freeVulkanMemory(stagingBufferMem);
    
    createBuffer(window, indexSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, &stagingBuffer);
    vkGetBufferMemoryRequirements(window->device, stagingBuffer.buffer, &memRequirements);
    stagingBufferMem = allocVulkanMemory(window, &memRequirements, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, BUFFER_MEMORY);
    bindBuffer(window, stagingBuffer, stagingBufferMem);
    
    vkMapMemory(window->device, getBindingPoint(stagingBufferMem), getOffset(stagingBufferMem), indexSize, 0, &data);
        memcpy(data, indices, indexSize);
    vkUnmapMemory(window->device, getBindingPoint(stagingBufferMem));
    
    createBuffer(window, indexSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, &model->indexBuffer);
    vkGetBufferMemoryRequirements(window->device, model->vertexBuffer.buffer, &memRequirements);
    model->indexMemory = allocVulkanMemory(window, &memRequirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, BUFFER_MEMORY);
    bindBuffer(window, model->indexBuffer, model->indexMemory);
    
    copyBuffer(window, &window->transferQueueFamily, pool, stagingBuffer.buffer, model->indexBuffer.buffer, indexSize);
    
    destroyBuffer(window, stagingBuffer);
    freeVulkanMemory(stagingBufferMem);
    
    vkDestroyCommandPool(window->device, pool, NULL);
    
    return model;
}

void destroyModelBuffer(Window *window, ModelBuffer *model) {
    destroyBuffer(window, model->vertexBuffer);
    destroyBuffer(window, model->indexBuffer);
    freeVulkanMemory(model->vertMemory);
    freeVulkanMemory(model->indexMemory);
    MEM_free(model);
}

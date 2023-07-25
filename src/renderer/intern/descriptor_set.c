//
//  descriptor_set.c
//  AgeOfAnarchy
//
//  Created by Emmanuel Mera on 12/03/2023.
//

#include <string.h>

#include "descriptor_set.h"

#include "MEM_alloc.h"

/*
 * TODO: list of things to do :
 *      - abstract descriptor pool allocation (just set the layout of the pools)
 *      - I don't know yet but will be updated
 */

VkDescriptorPool createDescriptorPool(Window *window) {
    VkDescriptorPoolSize poolSize;
    poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSize.descriptorCount = 1024; // should be enough for now, will change later
    
    VkDescriptorPoolCreateInfo poolInfo;
    memset(&poolInfo, 0, sizeof(VkDescriptorPoolCreateInfo));
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = 1024; // should be enough for now, will change later
    
    VkDescriptorPool pool;
    if (vkCreateDescriptorPool(window->device, &poolInfo, NULL, &pool) != VK_SUCCESS) {
        return VK_NULL_HANDLE;
    }
    
    return pool;
}

void destroyDescriptorPool(Window *window, VkDescriptorPool pool) {
    vkDestroyDescriptorPool(window->device, pool, NULL);
}

void createDescriptorSet(Window *window, VkDescriptorPool pool, VkDescriptorSetLayout *layout, uint32_t setCount, VkDescriptorSet *sets) {
    VkDescriptorSetAllocateInfo allocInfo;
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.pNext = NULL;
    allocInfo.descriptorPool = pool;
    allocInfo.descriptorSetCount = setCount;
    allocInfo.pSetLayouts = layout;
    
    if (vkAllocateDescriptorSets(window->device, &allocInfo, sets) != VK_SUCCESS) {
        printf("Unable to allocate descritpor set");
        return;
    }
}

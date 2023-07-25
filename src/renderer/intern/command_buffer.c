//
//  command_buffer.c
//  AgeOfAnarchy
//
//  Created by Emmanuel Mera on 05/02/2023.
//

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "command_buffer.h"
#include "init.h"

//-----------------------------------------------//
//-     Local functions                         -//
//-----------------------------------------------//

static void print_error(char str[]);
static void print_warning(char str[]);

//-----------------------------------------------//
//-     Implementation                          -//
//-----------------------------------------------//

int createCommandPool(Window *window, QueueFamily family, VkCommandPool *pool) {
    VkCommandPoolCreateInfo commandPoolInfo;
    memset(&commandPoolInfo, 0, sizeof(VkCommandPoolCreateInfo));
    commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    commandPoolInfo.queueFamilyIndex = family.family;
    if (vkCreateCommandPool(window->device, &commandPoolInfo, NULL, pool) != VK_SUCCESS) {
        print_error("unabled to create a command pool");
        return 0;
    }
    
    return 1;
}

int destroyCommandPool(Window *window, VkCommandPool pool) {
    vkDeviceWaitIdle(window->device);
    vkDestroyCommandPool(window->device, pool, NULL);
    return 1;
}

int createCommand(Window *window, VkCommandPool pool, VkCommandBuffer *buffer) {
    VkCommandBufferAllocateInfo commandBufferInfo;
    memset(&commandBufferInfo, 0, sizeof(VkCommandBufferAllocateInfo));
    commandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferInfo.commandPool = pool;
    commandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferInfo.commandBufferCount = 1;
    if (vkAllocateCommandBuffers(window->device, &commandBufferInfo, buffer) != VK_SUCCESS) {
        print_error("unabled to create a command buffer");
        return 0;
    }
    
    return 1;
}

/*
int recordCommand(Window *window, Commands *commands, uint32_t index, void (*command)(Commands *)) {
    VkCommandBufferBeginInfo commandBufferBeginInfo;
    memset(&commandBufferBeginInfo, 0, sizeof(VkCommandBufferBeginInfo));
    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    if (vkBeginCommandBuffer(commands->buffers[index], &commandBufferBeginInfo) != VK_SUCCESS) {
        print_error("unabled to record a command buffer");
        return 0;
    }
    
    command(commands);
    
    if (vkEndCommandBuffer(commands->buffers[index]) != VK_SUCCESS) {
        print_error("unabled to record a command buffer");
        return 0;
    }
    
    return 1;
}
*/
//-----------------------------------------------//
//-     Local functions                         -//
//-----------------------------------------------//

void print_error(char str[]) {
    fprintf(stderr, "[ \033[31mERROR\033[29m ] : initialization : %s\n", str);
}

void print_warning(char str[]) {
#ifdef DEBUG
    fprintf(stderr, "[ \033[32mWARNING\033[29m ] : initialization : %s\n", str);
#endif
}

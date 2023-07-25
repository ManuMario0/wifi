//
//  vertex.c
//  AgeOfAnarchy
//
//  Created by Emmanuel Mera on 09/02/2023.
//

#include "vertex.h"
#include "headers/common.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <pthread.h>

//-----------------------------------------------//
//-     Local structures                        -//
//-----------------------------------------------//

typedef struct {
    Window *    window;
    VertexBuffer * vertBuffer;
    Buffer      buffer;
    VkCommandBuffer commandBuffer;
    
    char        patch;
    size_t      size;
    size_t      srcMemOffset;
    size_t      dstMemOffset;
    char *      data;
} ThreadLoad;

//-----------------------------------------------//
//-     Local functions                         -//
//-----------------------------------------------//

static void print_error(char str[]);
static void print_warning(char str[]);
static void *record_command_buffer(void *load);

//-----------------------------------------------//
//-     Implementation                          -//
//-----------------------------------------------//


VertexBuffer *createVertexBuffer(Window *window, VkCommandPool pool, size_t size) {
    VertexBuffer *vertBuffer = malloc(sizeof(VertexBuffer));
    if (createBuffer(window, size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, &vertBuffer->vertexBuffer) != AOA_TRUE) {
        free(vertBuffer);
        return NULL;
    }
    
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(window->device, vertBuffer->vertexBuffer.buffer, &memRequirements);
    vertBuffer->overallMemRequiredSize = memRequirements.size;
    vertBuffer->dataSize = size;
    vertBuffer->tranferPool = pool;
    
    vertBuffer->targetMem = allocateDeviceMemory(window, memRequirements.size, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
}

int destroyVertexBuffer(Window *window, VertexBuffer *vertBuffer) {
    for (int i=0; i<vertBuffer->stagingBufferCount; i++) {
        vkDestroyBuffer(window->device, vertBuffer->stagingBuffers[i].buffer, NULL);
    }
    vkDestroyBuffer(window->device, vertBuffer->vertexBuffer.buffer, NULL);
    vkDestroyCommandPool(window->device, vertBuffer->tranferPool, NULL);
    freeDeviceMemory(window, vertBuffer->stagingMem);
    freeDeviceMemory(window, vertBuffer->targetMem);
    free(vertBuffer->stagingBuffers);
    free(vertBuffer->commandBuffers);
    free(vertBuffer);
}

int createWorkers(Window *window, VertexBuffer *vertBuffer, uint8_t count, size_t size) {
    uint32_t requiredSize = 0;
    vertBuffer->stagingBufferCount = count;
    vertBuffer->stagingBuffers = calloc(count, sizeof(Buffer));
    vertBuffer->stagingBufferSize = size;
    
    for (int i=0; i<count; i++) {
        if (createBuffer(window, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, &vertBuffer->stagingBuffers[i]) != AOA_TRUE) {
            free(vertBuffer->stagingBuffers); // I might regret this later ...
            return AOA_FALSE;
        }
        
        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(window->device, vertBuffer->stagingBuffers[i].buffer, &memRequirements);
        requiredSize += memRequirements.size;
    }
    
    vertBuffer->stagingMemSize = requiredSize;
    vertBuffer->stagingMem = allocateDeviceMemory(window, requiredSize, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    
    for (int i=0; i<count; i++) {
        bindBuffer(window, vertBuffer->stagingBuffers[i], vertBuffer->stagingMem);
    }
    
    vertBuffer->commandBuffers = calloc(count, sizeof(VkCommandBuffer));
    for (int i=0; i<count; i++) {
        createCommand(window, vertBuffer->tranferPool, &vertBuffer->commandBuffers[i]);
    }
    
    return AOA_TRUE;
}

int recordData(Window *window, VertexBuffer *vertbuffer, uint32_t dataCount, char *data, size_t size, uint8_t _jobs) {
    uint8_t jobs;
    if (_jobs > vertbuffer->stagingBufferCount) {
        jobs = vertbuffer->stagingBufferCount;
    } else {
        jobs = _jobs;
    }
    
    jobs /= 2;
    
    size_t  currentOffset = 0;
    char    workPatch = 0;
    
    VkFenceCreateInfo fenceInfo;
    memset(&fenceInfo, 0, sizeof(VkFenceCreateInfo));
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    VkFence fences[2];
    vkCreateFence(window->device, &fenceInfo, NULL, &fences[0]);
    vkCreateFence(window->device, &fenceInfo, NULL, &fences[1]);
    
    VkSubmitInfo submitInfo;
    memset(&submitInfo, 0, sizeof(VkSubmitInfo));
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = jobs;
    VkCommandBuffer *buffers = calloc(jobs, sizeof(VkBuffer));
    submitInfo.pCommandBuffers = buffers;
    
    pthread_t *threads = calloc(jobs, sizeof(pthread_t));
    ThreadLoad *load = calloc(jobs, sizeof(ThreadLoad));
    
    while (currentOffset < size) {
        for (int i=0; i<jobs; i++) {
            load[i].dstMemOffset = currentOffset;
            if (size - currentOffset < vertbuffer->stagingBufferSize) {
                load[i].size = size - currentOffset;
            } else {
                load[i].size = vertbuffer->stagingBufferSize;
            }
            load[i].data = &data[currentOffset];
            currentOffset += load[i].size;
            if (workPatch == 0) {
                load[i].buffer = vertbuffer->stagingBuffers[i];
                load[i].commandBuffer = vertbuffer->commandBuffers[i];
                buffers[i] = vertbuffer->commandBuffers[i];
            } else {
                load[i].buffer = vertbuffer->stagingBuffers[i+jobs];
                load[i].commandBuffer = vertbuffer->commandBuffers[i+jobs];
                buffers[i] = vertbuffer->commandBuffers[i+jobs];
            }
            load[i].patch = workPatch;
            load[i].window = window;
            load[i].srcMemOffset = i*vertbuffer->stagingBufferSize;
            load[i].vertBuffer = vertbuffer;
            
            pthread_create(&threads[i], NULL, record_command_buffer, (void*)&load[i]);
        }
        
        for (int i=0; i<jobs; i++) {
            pthread_join(threads[i], NULL);
        }
        
        vkWaitForFences(window->device, 1, &fences[workPatch], VK_TRUE, UINT64_MAX);
        
        vkQueueSubmit(window->transferQueueFamily.queues[0], 1, &submitInfo, fences[workPatch]);
        
        workPatch = 1 - workPatch;
    }
    
    vkDestroyFence(window->device, fences[0], NULL);
    vkDestroyFence(window->device, fences[1], NULL);
    free(buffers);
    
    return AOA_TRUE;
}

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

void *record_command_buffer(void *_load) {
    ThreadLoad *load = (ThreadLoad*) _load;
    
    if (load->size == 0) {
        return NULL;
    }
    
    void* mem;
    vkMapMemory(load->window->device, load->vertBuffer->stagingMem->memory, load->srcMemOffset, load->size, 0, &mem);
    memcpy(mem, load->data, load->size);
    vkUnmapMemory(load->window->device, load->vertBuffer->stagingMem->memory);
    
    VkCommandBufferBeginInfo beginInfo;
    memset(&beginInfo, 0, sizeof(VkCommandBufferBeginInfo));
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    
    if (vkBeginCommandBuffer(load->commandBuffer, &beginInfo) != VK_SUCCESS) {
        print_error("unabled to record buffer");
        return NULL;
    }
    
    VkBufferCopy copyRegion;
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = load->dstMemOffset;
    copyRegion.size = load->size;
    vkCmdCopyBuffer(load->commandBuffer, load->buffer.buffer, load->vertBuffer->vertexBuffer.buffer, 1, &copyRegion);
    
    if (vkEndCommandBuffer(load->commandBuffer) != VK_SUCCESS) {
        print_error("unabled to record buffer");
        return NULL;
    }
    
    return NULL;
}

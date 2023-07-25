//
//  buffer.h
//  AgeOfAnarchy
//
//  Created by Emmanuel Mera on 16/02/2023.
//

#ifndef buffer_h
#define buffer_h

#include "vulkan/vulkan.h"
#include "init.h"
#include "device_alloc.h"
/*
typedef struct {
    size_t                  size;
    size_t                  offset;
    VkMemoryPropertyFlags   properties;
    VkDeviceMemory          memory;
} BufferMemory;*/

typedef struct {
    VkBuffer    buffer;
    size_t      size;
} Buffer;

//BufferMemory *allocateDeviceMemory(Window *window, size_t size, uint32_t typeFilter, VkMemoryPropertyFlags properties);
//int freeDeviceMemory(Window *window, BufferMemory *mem);
int createBuffer(Window *window, size_t size, VkBufferUsageFlags usage, Buffer *buffer);
int destroyBuffer(Window *window, Buffer buffer);
int bindBuffer(Window *window, Buffer buffer, MemBlock *b);
int copyBuffer(Window *window, QueueFamily *family, VkCommandPool pool, VkBuffer srcBuffer, VkBuffer dstBuffer, size_t size);

#endif /* buffer_h */

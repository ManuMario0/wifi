//
//  device_alloc.h
//  AgeOfAnarchy
//
//  Created by Emmanuel Mera on 31/03/2023.
//

#ifndef device_alloc_h
#define device_alloc_h

#include <stdio.h>

#define DEFAULT_SEGMENT_SIZE 0x1000000

enum {
    BUFFER_MEMORY,
    IMAGE_MEMORY
} MemType;

typedef struct MemBlock MemBlock;

#include "init.h"

void initVulkanAllocator(Window *window);
void destroyVulkanAllocators(Window *window);
void freeVulkanMemory(MemBlock *block);
MemBlock *allocVulkanMemory(Window *window, VkMemoryRequirements *memRequirements, VkMemoryPropertyFlags properties, int type);
VkDeviceMemory getBindingPoint(MemBlock *b);
size_t getOffset(MemBlock *b);

#endif /* device_alloc_h */

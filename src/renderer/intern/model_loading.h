//
//  model_loading.h
//  AgeOfAnarchy
//
//  Created by Emmanuel Mera on 19/02/2023.
//

#ifndef model_loading_h
#define model_loading_h

#include "init.h"
#include "buffer.h"
#include "vulkan/vulkan.h"
#include "device_alloc.h"


typedef struct {
    Buffer          vertexBuffer;
    Buffer          indexBuffer;
    
    MemBlock *      vertMemory;
    MemBlock *      indexMemory;
} ModelBuffer;

ModelBuffer *createModelBuffer(Window *window, size_t vertexSize, void *vertices, size_t indexSize, void *indices);
void destroyModelBuffer(Window *window, ModelBuffer *model);

#endif /* model_loading_h */

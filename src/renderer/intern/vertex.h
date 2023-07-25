//
//  vertex.h
//  AgeOfAnarchy
//
//  Created by Emmanuel Mera on 09/02/2023.
//

#ifndef vertex_h
#define vertex_h

#include "init.h"
#include "renderer.h"
#include "buffer.h"

typedef struct {
    size_t          overallMemRequiredSize;
    BufferMemory *  targetMem;
    Buffer          vertexBuffer;
    
    size_t          stagingMemSize;
    BufferMemory *  stagingMem;
    size_t          stagingBufferSize;
    uint32_t        stagingBufferCount;
    Buffer *        stagingBuffers;
    
    VkCommandPool   tranferPool;
    VkCommandBuffer *commandBuffers;
    
    size_t          dataSize;
} VertexBuffer;

VertexBuffer *createVertexBuffer(Window *window, VkCommandPool pool, size_t size);
int destroyVertexBuffer(Window *window, VertexBuffer *vertBuffer);
int createWorkers(Window *window, VertexBuffer *vertBuffer, uint8_t count, size_t size);
int recordData(Window *window, VertexBuffer *vertbuffer, uint32_t dataCount, char *data, size_t size, uint8_t _jobs);

#endif /* vertex_h */

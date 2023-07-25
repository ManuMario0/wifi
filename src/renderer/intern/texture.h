//
//  texture.h
//  AgeOfAnarchyXCode
//
//  Created by Emmanuel Mera on 10/03/2023.
//

#ifndef texture_h
#define texture_h

#include <stdio.h>

#include "buffer.h"
#include "init.h"
#include "device_alloc.h"

typedef struct {
    VkDescriptorSet descriptorSets[MAX_FRAMES_IN_FLIGHT];
    
    VkImage         image;
    VkImageView     view;
    MemBlock *      mem;
} Texture;

int createTexturedPipeline(Window *window);
void destroyTexturePipeline(Window *window);
Texture *createTexture(Window *window, unsigned char *image, int x, int y, int channel);
Texture *loadTexture(Window *window, char filename[]);
void destroyTexture(Window *window, Texture *tex);
VkSampler createImageSampler(Window *window);

#endif /* texture_h */

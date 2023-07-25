//
//  renderer.h
//  AgeOfAnarchy
//
//  Created by Emmanuel Mera on 06/02/2023.
//

#ifndef renderer_h
#define renderer_h

#include <stdio.h>

#include "init.h"
#include "model_loading.h"
#include "command_buffer.h"
#include "model.h"
#include "object.h"
#include "camera.h"
#include "ui.h"
#include "device_alloc.h"

typedef struct {
    Window *    window;
    
    VkCommandPool   renderPool;
    VkCommandBuffer commands[2];
    
    int             modelCount;
    Model **        models;
    
    int             objectCount;
    Object **       objects;
    
    mat4            proj;
    Camera *        cam;
    
    void *            ui;
} Renderer;

int recordCommandBuffer(Renderer *renderer, uint32_t index);
int drawFrame(Renderer *renderer);
void getObjectMatrix(Renderer *r, Object *obj, mat4 dest);

#endif /* renderer_h */

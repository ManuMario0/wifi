//
//  model.h
//  AgeOfAnarchyXCode
//
//  Created by Emmanuel Mera on 22/02/2023.
//

#ifndef model_h
#define model_h

#include "model_loading.h"
#include "cglm/cglm.h"

typedef struct {
    ModelBuffer *   buffer;
    uint32_t        indexCount;
} Model;

Model *createModel(Window *window, size_t vertexSize, void *vertices, size_t indexSize, void *indices);
void destroyModel(Window *window, Model *model);
void setModelColor(Model *model, vec4 color);

#endif /* model_h */

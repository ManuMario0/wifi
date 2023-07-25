//
//  model.c
//  AgeOfAnarchyXCode
//
//  Created by Emmanuel Mera on 22/02/2023.
//

#include <string.h>

#include "model.h"
#include "MEM_alloc.h"

Model *createModel(Window *window, size_t vertexSize, void *vertices, size_t indexSize, void *indices) {
    Model *model = MEM_malloc(sizeof(Model), __func__);
    model->buffer = createModelBuffer(window, vertexSize, vertices, indexSize, indices);
    model->indexCount = (uint32_t) (indexSize / sizeof(uint16_t));
    return model;
}

void destroyModel(Window *window, Model *model) {
    destroyModelBuffer(window, model->buffer);
    MEM_free(model);
}

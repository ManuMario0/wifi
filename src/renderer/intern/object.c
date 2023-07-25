//
//  object.c
//  AgeOfAnarchy
//
//  Created by Emmanuel Mera on 25/02/2023.
//

#include <stdlib.h>
#include <string.h>

#include "object.h"
#include "camera.h"
#include "MEM_alloc.h"

Object *createObject(int model, vec3 pos, vec3 orientation, float scale, vec4 color) {
    Object *obj = MEM_malloc_aligned(sizeof(Object), 16, __func__);
    obj->model = model;
    obj->scale = scale;
    obj->should_draw = 1;
    memcpy(obj->pos, pos, sizeof(vec3));
    memcpy(obj->orientation, orientation, sizeof(vec3));
    memcpy(obj->color, color, sizeof(vec4));
    return obj;
}

void destroyObject(Object *obj) {
    MEM_free(obj);
}

float *getObjectPos(Object *obj) {
    return obj->pos;
}

void placeObject(Object *obj, vec3 pos) {
    memcpy(obj->pos, pos, sizeof(vec3));
}

void moveObject(Object *obj, vec3 displacment) {
    glm_vec3_add(obj->pos, displacment, obj->pos);
}

void rotateObject(Object *obj, vec3 rotation) {
    
}

void outlineObject(Object *obj, uint8_t b) {
    obj->outline = b;
}

void shouldDrawObject(Object *obj, uint8_t flag) {
    obj->should_draw = flag;
}

void setColor(Object *obj, vec4 color) {
    memcpy(obj->color, color, sizeof(vec4));
}

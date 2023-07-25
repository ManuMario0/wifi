//
//  camera.c
//  AgeOfAnarchyXCode
//
//  Created by Emmanuel Mera on 25/02/2023.
//

#include <string.h>

#include "camera.h"
#include "MEM_alloc.h"

Camera *createCamera(vec3 pos) {
    Camera *cam = MEM_malloc(sizeof(Camera), __func__);
    memcpy(cam->pos, pos, sizeof(vec3));
    memcpy(cam->up, (vec3){0.f, 1.f, 0.f}, sizeof(vec3));
    return cam;
}

void destroyCamera(Camera *cam) {
    MEM_free(cam);
}

void getCameraMatrix(Camera *cam, mat4 dest) {
    glm_lookat(cam->pos, (vec3){0.f, 0.f, 0.f}, cam->up, dest);
}

void moveCamera(Camera *cam, vec3 displacment) {
    glm_vec3_scale(cam->pos, displacment[0], cam->pos);
    
    vec3 temp;
    glm_vec3_cross(cam->pos, cam->up, temp);
    glm_vec3_rotate(cam->pos, displacment[1], temp);
    glm_vec3_rotate(cam->pos, displacment[2], cam->up);
    glm_vec3_rotate(cam->up, displacment[1], temp);
}

void setCamera(Camera *cam, vec3 pos) {
    memcpy(cam->pos, pos, sizeof(vec3));
}

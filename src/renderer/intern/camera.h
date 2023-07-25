//
//  camera.h
//  AgeOfAnarchyXCode
//
//  Created by Emmanuel Mera on 25/02/2023.
//

#ifndef camera_h
#define camera_h

#include "cglm/cglm.h"

typedef struct {
    vec3        pos;
    vec3        up;
} Camera;

Camera *createCamera(vec3 pos);
void destroyCamera(Camera *cam);
void getCameraMatrix(Camera *cam, mat4 dest);
void moveCamera(Camera *cam, vec3 displacment);
void setCamera(Camera *cam, vec3 pos);

#endif /* camera_h */

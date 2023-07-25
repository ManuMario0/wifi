//
//  animation.c
//  empty
//
//  Created by Emmanuel Mera on 23/07/2023.
//

#include "animation.h"
#include "main_loop.h"

#include "USR_common.h"
#include "MEM_alloc.h"

void VSL_launch_animation(USR_user_list *ul, DEV_ap_list *apl, DEV_device_list *dl) {
    Renderer *r = createRenderer();
    
    Model s = addModel(r, "../Resources/sphere.3d");
    Model c = addModel(r, "../Resources/cube.3d");
    
    APNode *ap_node = load_ap_nodes(r, s, apl, dl);
    DeviceNode *dev_node = load_device_nodes(r, c, ul, dl, apl);
    
    setGlobalCamera(r, (vec3){5000.f, 0.f, 0.f});
    
    long iterations = 0;
    
    while (!windowShouldClose(r) && iterations < 600) {
        update_ap_pos(ap_node, apl, dl, .02);
        
        if (iterations < 81) {
            float speed = 1./sqrtf((float)1+iterations);
            moveGlobalCamera(r, (vec3){1.-speed/10., 0., -speed/10.});
        } else if (iterations < 121) {
            float speed = 1./sqrtf((float)1+iterations);
            moveGlobalCamera(r, (vec3){1.f, 0., -speed/10.});
        } else {
            float speed = 1./expf(logf(121.f)+iterations-121.f);
            moveGlobalCamera(r, (vec3){1.f, 0., -speed});
        }
        
        drawFrame(r);
        glfwPollEvents();
        
        iterations ++;
    }
}

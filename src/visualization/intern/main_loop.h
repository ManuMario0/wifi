//
//  main_loop.h
//  empty
//
//  Created by Emmanuel Mera on 19/07/2023.
//

#ifndef main_loop_h
#define main_loop_h

#include "cglm/common.h"

#include "DEVICE_common.h"
#include "RND_window.h"
#include "USR_common.h"

/* ---------------------------- */
/*  local structures            */
/* ---------------------------- */

typedef struct APNode {
    Object *object;
    vec3            gravity;
    vec3            speed;
    vec3            tmp_pos;
    vec3            tmp_speed;
    unsigned long   id;
} APNode;

typedef struct DeviceNode {
    Object *        object;
    DEV_device *    device;
} DeviceNode;

/* ---------------------------- */
/*  local functions             */
/* ---------------------------- */

APNode *load_ap_nodes(Renderer *r, Model sphere, DEV_ap_list *apl, DEV_device_list *dl);
DeviceNode *load_device_nodes(Renderer *r, Model sphere, USR_user_list *ul, DEV_device_list *dl, DEV_ap_list *apl);

void update_ap_pos(APNode *n, DEV_ap_list *apl, DEV_device_list *dl, float delta);
void update_dev_pos(DeviceNode *n, APNode *ap_node, DEV_ap_list *apl, DEV_device_list *dl, time_t current);

#endif /* main_loop_h */

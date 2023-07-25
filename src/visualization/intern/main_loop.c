//
//  main_loop.c
//  empty
//
//  Created by Emmanuel Mera on 19/07/2023.
//

#include <string.h>
#include <stdlib.h>

#include "main_loop.h"

#include "MEM_alloc.h"

/* ---------------------------- */
/*  local variables             */
/* ---------------------------- */

double xpos, ypos, rxpos, rypos;

/* ---------------------------- */
/*  local functions             */
/* ---------------------------- */

static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
static void cursor_position_callback(GLFWwindow* window, double nxpos, double nypos);

/* ---------------------------- */
/*  implementation              */
/* ---------------------------- */

void VSL_launchVisualization(USR_user_list *ul, DEV_ap_list *apl, DEV_device_list *dl) {
    Renderer *r = createRenderer();
    
    Model s = addModel(r, "../Resources/sphere.3d");
    Model c = addModel(r, "../Resources/cube.3d");
    
    APNode *ap_node = load_ap_nodes(r, s, apl, dl);
    DeviceNode *dev_node = load_device_nodes(r, c, ul, dl, apl);
    
    GLFWwindow *window = getGLFWWindow(r);
    glfwSetScrollCallback(window, scroll_callback);
    glfwGetCursorPos(window, &xpos, &ypos);
    glfwSetCursorPosCallback(window, cursor_position_callback);
//    glfwSetMouseButtonCallback(window, mouse_button_callback);
    
    long iterations = 0;
    
    while (!windowShouldClose(r) && iterations < 300) {
        update_ap_pos(ap_node, apl, dl, .02);
        
        drawFrame(r);
        glfwPollEvents();
        
        iterations ++;
    }
    
    struct tm start_tm = {0};
    start_tm.tm_year = 2022 - 1900;
    start_tm.tm_mon = 9 - 1;
    start_tm.tm_mday = 8;
    start_tm.tm_hour = 7;
    time_t time = mktime(&start_tm);
    char buffer[2048];
    while (!windowShouldClose(r)) {
        update_dev_pos(dev_node, ap_node, apl, dl, time);
        time += 1;
        
        strftime(buffer, 2048, "%T", localtime(&time));
        printf("%s\n", buffer);
        
        drawFrame(r);
        glfwPollEvents();
    }
    
    destroyRenderer(r);
}

/* ---------------------------- */
/*  local functions             */
/* ---------------------------- */

APNode *load_ap_nodes(Renderer *r, Model sphere, DEV_ap_list *apl, DEV_device_list *dl) {
    APNode *n = MEM_calloc_array(sizeof(APNode), dl->AP_count+1, __func__);
    
    
    for (unsigned long i=1; i<=dl->AP_count; i++) {
        vec3 pos;
        pos[0] = (float)(rand()%1000)-500.;
        pos[1] = (float)(rand()%1000)-500.;
        pos[2] = (float)(rand()%1000)-500.;
        glm_vec3_normalize(pos);
//        glm_vec3_scale(pos, 100., pos);
        
        char *key = CSV_reverse_id(dl->csv, APMAC, i);
        char *name = (char*)KER_hash_find(apl->translate, key, (int)strlen(key));
        
        if (name) {
            switch (name[0]) {
                case 'A':
                    n[i].gravity[0] = 400.;
                    n[i].object = addObject(r, sphere, pos, (vec3)GLM_VEC3_ZERO_INIT, 1.0f, (vec4){1.f, 1.f, 6.f, .7f});
                    break;
                    
                case 'C':
                    n[i].gravity[1] = 400.;
                    n[i].object = addObject(r, sphere, pos, (vec3)GLM_VEC3_ZERO_INIT, 1.0f, (vec4){1.f, 6.f, 1.f, .7f});
                    break;
                    
                case 'M':
                    n[i].gravity[2] = 400.;
                    n[i].object = addObject(r, sphere, pos, (vec3)GLM_VEC3_ZERO_INIT, 1.0f, (vec4){6.f, 1.f, 1.f, .7f});
                    break;
                    
                default:
                    n[i].object = addObject(r, sphere, pos, (vec3)GLM_VEC3_ZERO_INIT, 1.0f, (vec4){(name[2] - '0'), (name[2] - '0'), (name[2] - '0'), .7f});
                    break;
            }
        } else {
            n[i].object = addObject(r, sphere, pos, (vec3)GLM_VEC3_ZERO_INIT, 1.0f, (vec4)GLM_VEC4_ZERO_INIT);
            shouldDrawObject(n[i].object, 0);
        }
        n[i].id = i;
        
        float sum = 0.;
        for (unsigned long j=1; j<=dl->AP_count; j++) {
            sum += 1000. - DEV_get_AP_distance(dl, i, j);
        }
        if (sum == 1000.) {
            shouldDrawObject(n[i].object, 0);
        }
    }
    
    return n;
}

DeviceNode *load_device_nodes(Renderer *r, Model cube, USR_user_list *ul, DEV_device_list *dl, DEV_ap_list *apl) {
    DeviceNode *n = MEM_calloc_array(sizeof(DeviceNode), dl->effective_device_count, __func__);
    
    long index = 0;
    for (int i=0; i<ul->user_count; i++) {
        USR_user u = ul->users[i];
        for (int j=0; j<u.device_count; j++) {
            DEV_device *d = u.devices[j];
            n[index].object = addObject(r, cube, (vec3)GLM_VEC3_ZERO_INIT, (vec3)GLM_VEC3_ZERO_INIT, 1.0f, (vec4){10.f, 10.f, 10.f, 1.0f});
            shouldDrawObject(n[index].object, 0);
            n[index].device = d;
            
            index++;
        }
    }
    
    return n;
}

void update_ap_pos(APNode *n, DEV_ap_list *apl, DEV_device_list *dl, float delta) {
    for (unsigned long i=1; i<=dl->AP_count; i++) {
        APNode *cn = &n[i];
        
        // compute pos
        vec3 tmp;
        glm_vec3_scale(cn->speed, delta, tmp);
        glm_vec3_add(tmp, getObjectPos(cn->object), cn->tmp_pos);
        
        memset(tmp, 0, sizeof(vec3));
        // compute speed
        for (unsigned long j=1; j<=dl->AP_count; j++) {
            if (/*DEV_get_AP_distance(dl, i, j) < 1000. &&*/ i!=j) {
                APNode *on = &n[j];
                
                vec3 dir;
                float *tt = getObjectPos(cn->object);
                float *ttt = getObjectPos(on->object);
                glm_vec3_sub(getObjectPos(cn->object), getObjectPos(on->object), dir);
                
                float length = glm_vec3_norm(dir);
                
                if (length == 0.) {
                    dir[0] = (float)(rand()%1000);
                    dir[1] = (float)(rand()%1000);
                    dir[2] = (float)(rand()%1000);
                }
                
                glm_vec3_normalize(dir);
                vec3 dirtmp;
                if (DEV_get_AP_distance(dl, i, j) < 1000.) {
                    glm_vec3_scale(dir, (DEV_get_AP_distance(dl, i, j) - length), dirtmp);
                } else if (length > 0) {
                    glm_vec3_scale(dir, 10./length, dirtmp);
                }
                glm_vec3_add(dirtmp, tmp, tmp);
            }
        }
        
        vec3 fluid;
        glm_vec3_scale(cn->speed, -40., fluid);
        glm_vec3_add(tmp, fluid, tmp);
        
        vec3 gravity;
        glm_vec3_sub(cn->gravity, getObjectPos(cn->object), gravity);
        glm_vec3_scale(gravity, 30., gravity);
        glm_vec3_add(tmp, gravity, tmp);
        
        glm_vec3_scale(tmp, delta, tmp);
        glm_vec3_add(tmp, cn->speed, cn->tmp_speed);
    }
    
    for (unsigned long i=1; i<=dl->AP_count; i++) {
        APNode *cn = &n[i];
        memcpy(cn->speed, cn->tmp_speed, sizeof(vec3));
        placeObject(cn->object, cn->tmp_pos);
    }
}

static long find_floor_index(DEV_device *d, time_t ts) {
    long left = 0;
    long right = d->local_csv->row_count-1;
    CSV_cell *row;
    
    while (left < right) {
        long m = (right+left+1)/2;
        row =  DEV_get_row(d, m);
        if (row[TIMESTAMP].l < ts)
            left = m;
        else
            right = m-1;
    }
    
    row = DEV_get_row(d, left);
    if (row[TIMESTAMP].l > ts)
        return --left;
    return left;
}

void update_dev_pos(DeviceNode *n, APNode *ap_node, DEV_ap_list *apl, DEV_device_list *dl, time_t current) {
    static long stop = 0;
    if (stop == 0)
     stop = (long)KER_hash_find(dl->devices->local_csv->types[STATUS_TYPE].tbl, "Stop", 4);
    
    for (unsigned long i=0; i<dl->effective_device_count; i++) {
        DeviceNode *cn = &n[i];
        DEV_device *d = cn->device;
        
        long index = find_floor_index(d, current);
        
        if (index+1 < d->logs_count && index > -1) {
            CSV_cell *prev = CSV_get_row(d->local_csv, index);
            CSV_cell *next = CSV_get_row(d->local_csv, index+1);
            
            if (next[TIMESTAMP].l >= current  && prev[STATUS_TYPE].l != stop) {
                shouldDrawObject(cn->object, 1);
                
                float prop =
                    (float)(current-prev[TIMESTAMP].l) / (float)(next[TIMESTAMP].l-prev[TIMESTAMP].l);
                
                vec3 dir;
                glm_vec3_sub(getObjectPos(ap_node[next[APMAC].l].object), getObjectPos(ap_node[prev[APMAC].l].object), dir);
                glm_vec3_normalize(dir);
                glm_vec3_scale(dir, prop, dir);
                glm_vec3_add(dir, getObjectPos(ap_node[prev[APMAC].l].object), dir);
                placeObject(cn->object, dir);
            }
        } else {
            shouldDrawObject(cn->object, 0);
        }
    }
}

static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    Renderer *renderer = glfwGetWindowUserPointer(window);
    float shift = tanh(yoffset)/10.f+1.f;
    moveGlobalCamera(renderer, (vec3){shift, 0.f, 0.f});
}

static void cursor_position_callback(GLFWwindow* window, double nxpos, double nypos) {
    Renderer *renderer = glfwGetWindowUserPointer(window);
    int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
    if (state == GLFW_PRESS | state == GLFW_REPEAT)
        moveGlobalCamera(renderer, (vec3){1.f, (nypos-ypos)/50.f, (nxpos-xpos)/50.f}); // some mistakes here
//    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
//        resizeRectangle(rect, (int)(xpos-rxpos), (int)(ypos-rypos));
//
//        vec2 offset, extent;
//        offset[0] = rxpos; offset[1] = rypos;
//        extent[0] = xpos-rxpos; extent[1] = ypos-rypos;
//
//        detectSelectedElements(context, offset, extent, context->units, (vec4){0.f, 0.f, 0.f, 1.f});
//    }
    xpos = nxpos;
    ypos = nypos;
}

//static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
//    Renderer *renderer = glfwGetWindowUserPointer(window);
//
//    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
//        rxpos = xpos;
//        rypos = ypos;
//        placeRectangle(rect, (int)xpos, (int)ypos);
//        resizeRectangle(rect, 0, 0);
//
//        vec2 offset, extent;
//        offset[0] = rxpos-10.; offset[1] = rypos-10.;
//        extent[0] = 20.; extent[1] = 20.;
//        detectSelectedElements(context, offset, extent, context->units, (vec4){0.f, 0.f, 0.f, 1.f});
//        detectSelectedElements(context, offset, extent, context->buildings, (vec4){0.f, 0.f, 0.f, 1.f});
//    }
//    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
//        placeRectangle(rect, -5, -5);
//        resizeRectangle(rect, 1, 1);
//    }
//
//    vec3 pos;
//    memcpy(pos, getGlobalCameraPosition(renderer), sizeof(vec3));
//    vec3 up;
//    memcpy(up, getGlobalCameraUp(renderer), sizeof(vec3));
//    mat4 temp = GLM_MAT4_IDENTITY_INIT;
//    vec3 right;
//    glm_vec3_cross(pos, up, right);
//
//    double _xpos = xpos;
//    double _ypos = ypos;
//    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
//        List *l = context->units;
//        vec4 dest;
//
//        int width, height;
//        glfwGetWindowSize(window, &width, &height);
//        double yh = (_xpos - (double)width/2.)/(double)height * 2. * tan(M_PI/8.);
//        double xh = (_ypos - (double)height/2.)/(double)height * 2. * tan(M_PI/8.);
//
//        vec3 relObjPos;
//        double n = sqrt(pow(yh, 2.) + pow(xh, 2.));
//
//        double d = glm_vec3_norm(pos);
//        double angle = M_PI - atan(n) + asin(d*sin(atan(n))/10.01);
//
//        vec4 axe;
//        glm_normalize(right);
//        glm_vec3_scale(right, yh, right);
//        glm_vec3_scale(up, -xh, up);
//        glm_vec3_add(right, up, relObjPos);
//        glm_normalize(pos);
//        glm_vec3_scale(pos, -1., pos);
//        glm_vec3_add(relObjPos, pos, relObjPos);
//        glm_vec3_cross(relObjPos, pos, axe);
//        glm_normalize(axe);
//        glm_rotate_at(temp, (vec3){0.f, 0.f, 0.f}, -angle, axe);
//        glm_mat4_mulv(temp, (vec4){pos[0], pos[1], pos[2], 1.}, dest);
//        glm_vec3_scale(dest, 10.01/glm_vec3_norm(dest), dest);
//
//        while (l) {
//            Unit *u = listHead(l);
//            if (u->isSelected) {
//                memcpy(u->destination, dest, sizeof(vec3));
//            }
//            l = listTail(l);
//        }
//    }
//}

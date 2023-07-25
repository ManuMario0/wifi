//
//  renderer.c
//  AgeOfAnarchy
//
//  Created by Emmanuel Mera on 06/02/2023.
//

#include "renderer.h"
#include "wavefront.h"
#include "texture.h"
#include "MEM_alloc.h"

#define CGLM_FORCE_LEFT_HANDED
#define CGLM_FORCE_DEPTH_ZERO_TO_ONE
#include "cglm/cglm.h"

#include <stdlib.h>
#include <string.h>

//-----------------------------------------------//
//-     Local functions                         -//
//-----------------------------------------------//

static void print_error(char str[]);
static void print_warning(char str[]);

//-----------------------------------------------//
//-     Implementation                          -//
//-----------------------------------------------//

Renderer *createRenderer(void) {
    Renderer *renderer = MEM_malloc_aligned(sizeof(Renderer), 32, __func__);
    memset(renderer, 0, sizeof(Renderer));
    renderer->window = createWindow();
    if (!renderer->window) {
        print_error("unabled to create the renderer");
        MEM_free(renderer);
        return NULL;
    }
    glfwSetWindowUserPointer(renderer->window->glfwWindow, renderer);
    selectPhysicalDevice(renderer->window);
    createLogicalDevice(renderer->window);
    setupSwapchain(renderer->window);
    createImageViews(renderer->window);
    createDepthResources(renderer->window);
    createRenderpass(renderer->window);
    setupPipeline(renderer->window);
    createTexturedPipeline(renderer->window);
    createFramebuffers(renderer->window);
    initVulkanAllocator(renderer->window);
    
    glm_perspective(GLM_PI_4f, (float)renderer->window->swapchainSize.width/(float)renderer->window->swapchainSize.height, 0.01f, 2000.0f, renderer->proj);
//    glm_perspective_default((float)renderer->window->swapchainSize.width/(float)renderer->window->swapchainSize.height, renderer->proj);
    
    renderer->cam = createCamera((vec3){200.f, 0.f, 0.f});
    
    createCommandPool(renderer->window, renderer->window->graphicQueueFamily, &renderer->renderPool);
    
    createCommand(renderer->window, renderer->renderPool, &renderer->commands[0]);
    createCommand(renderer->window, renderer->renderPool, &renderer->commands[1]);
    createSync(renderer->window);
    
    renderer->ui = createUI(renderer->window);
    
    return renderer;
}

void destroyRenderer(Renderer *renderer) {
    if (!renderer) {
        return;
    }
    vkDeviceWaitIdle(renderer->window->device);
    destroyUI(renderer->window, renderer->ui);
    destroyTexturePipeline(renderer->window);
    destroyCommandPool(renderer->window, renderer->renderPool);
    destroyCamera(renderer->cam);
    for (int i=0; i<renderer->objectCount; i++) {
        destroyObject(renderer->objects[i]);
    }
    for (int i=0; i<renderer->modelCount; i++) {
        destroyModel(renderer->window, renderer->models[i]);
    }
    destroyVulkanAllocators(renderer->window);
    destroyWindow(renderer->window);
    MEM_free(renderer);
}

void clearRenderer(Renderer *renderer) {
    vkDeviceWaitIdle(renderer->window->device);
    clearUI(renderer->ui);
    if (renderer->objects) {
        MEM_free(renderer->objects);
    }
    renderer->objects = NULL;
    renderer->objectCount = 0;
}

GLFWwindow *getGLFWWindow(Renderer *renderer) {
    return renderer->window->glfwWindow;
}

int windowShouldClose(Renderer *renderer) {
    return glfwWindowShouldClose(renderer->window->glfwWindow);
}

// TODO: move this to vertex.c and improve vert structure
int recordCommandBuffer(Renderer *renderer, uint32_t imageIndex) {
    VkCommandBufferBeginInfo beginInfo;
    memset(&beginInfo, 0, sizeof(VkCommandBufferBeginInfo));
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    if (vkBeginCommandBuffer(renderer->commands[renderer->window->currentFrame], &beginInfo) != VK_SUCCESS) {
        print_error("unabled to begin command recording");
        return 0;
    }
    
    VkRenderPassBeginInfo renderPassInfo;
    memset(&renderPassInfo, 0, sizeof(VkRenderPassBeginInfo));
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderer->window->renderPass;
    renderPassInfo.framebuffer = renderer->window->framebuffers[renderer->window->currentFrame];
    VkOffset2D offset = {0, 0};
    VkExtent2D extent = renderer->window->swapchainSize;
    renderPassInfo.renderArea.offset = offset;
    renderPassInfo.renderArea.extent = extent;
    
    VkClearValue clearVal = {0.0f, 0.0f, 0.0f, 1.f};
    VkClearValue depthVal = {1.0f, 0};
    renderPassInfo.clearValueCount = 2;
    renderPassInfo.pClearValues = (VkClearValue[2]){clearVal, depthVal};
    
    vkCmdBeginRenderPass(renderer->commands[renderer->window->currentFrame], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    
    vkCmdBindPipeline(renderer->commands[renderer->window->currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->window->colorPipeline);
    
    VkViewport viewport;
    viewport.x = 0.f;
    viewport.y = 0.f;
    viewport.width = (float)renderer->window->swapchainSize.width;
    viewport.height = (float)renderer->window->swapchainSize.height;
    viewport.minDepth = 0.f;
    viewport.maxDepth = 1.f;
    vkCmdSetViewport(renderer->commands[renderer->window->currentFrame], 0, 1, &viewport);
    
    VkRect2D scissor;
    scissor.offset = offset;
    scissor.extent = extent;
    vkCmdSetScissor(renderer->commands[renderer->window->currentFrame], 0, 1, &scissor);
    
    float lightOn;
    
    for (int i=0; i<renderer->objectCount; i++) {
        Object  *obj = renderer->objects[i];
        Model *model = renderer->models[obj->model];
        
        if (obj->should_draw) {
            vkCmdSetStencilReference(renderer->commands[renderer->window->currentFrame], VK_STENCIL_FACE_FRONT_BIT, 1);
            if (obj->outline) {
                vkCmdSetStencilWriteMask(renderer->commands[renderer->window->currentFrame], VK_STENCIL_FACE_FRONT_BIT, 1);
            } else {
                vkCmdSetStencilWriteMask(renderer->commands[renderer->window->currentFrame], VK_STENCIL_FACE_FRONT_BIT, 0);
            }
            
            VkDeviceSize bufferOffset = 0;
            vkCmdBindVertexBuffers(renderer->commands[renderer->window->currentFrame], 0, 1, &model->buffer->vertexBuffer.buffer, &bufferOffset);
            vkCmdBindIndexBuffer(renderer->commands[renderer->window->currentFrame], model->buffer->indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT16);
            
            mat4 transform;
            getObjectMatrix(renderer, obj, transform);
            
            vkCmdPushConstants(renderer->commands[renderer->window->currentFrame], renderer->window->colorPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(mat4), transform);
            vkCmdPushConstants(renderer->commands[renderer->window->currentFrame], renderer->window->colorPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, sizeof(mat4), sizeof(vec4), obj->color);
            
            lightOn = 1.;
            vkCmdPushConstants(renderer->commands[renderer->window->currentFrame], renderer->window->colorPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, sizeof(mat4)+sizeof(vec4), sizeof(float), &lightOn);
            
            vkCmdDrawIndexed(renderer->commands[renderer->window->currentFrame], model->indexCount, 1, 0, 0, 0);
            
            if (obj->outline) {
                vkCmdSetStencilReference(renderer->commands[renderer->window->currentFrame], VK_STENCIL_FACE_FRONT_BIT, 0);
                
                vkCmdPushConstants(renderer->commands[renderer->window->currentFrame], renderer->window->colorPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, sizeof(mat4), sizeof(vec4), (vec4){1.f, 1.f, 1.f, 1.f});
                glm_scale(transform, (vec3){1.02f, 1.02f, 1.02f});
                vkCmdPushConstants(renderer->commands[renderer->window->currentFrame], renderer->window->colorPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(mat4), transform);
                lightOn = 0.;
                vkCmdPushConstants(renderer->commands[renderer->window->currentFrame], renderer->window->colorPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, sizeof(mat4)+sizeof(vec4), sizeof(float), &lightOn);
                vkCmdDrawIndexed(renderer->commands[renderer->window->currentFrame], model->indexCount, 1, 0, 0, 0);
            }
        }
    }
    
    recordUICommandBuffer(renderer->ui, renderer->window, renderer->commands[renderer->window->currentFrame]);
    
    vkCmdEndRenderPass(renderer->commands[renderer->window->currentFrame]);
    
    if (vkEndCommandBuffer(renderer->commands[renderer->window->currentFrame]) != VK_SUCCESS) {
        print_error("unabled to end command recording");
        return 0;
    }
    
    return 1;
}

int drawFrame(Renderer *renderer) {
    vkWaitForFences(renderer->window->device, 1, &renderer->window->inFlightFences[renderer->window->currentFrame], VK_TRUE, UINT64_MAX);
    
    uint32_t imageIndex;
    if (vkAcquireNextImageKHR(renderer->window->device, renderer->window->swapchain, UINT64_MAX, renderer->window->imageAvailableSemaphores[renderer->window->currentFrame], VK_NULL_HANDLE, &imageIndex) != VK_SUCCESS) {
        print_error("unabled to acquire next image");
        return 0;
    }
    
    vkResetFences(renderer->window->device, 1, &renderer->window->inFlightFences[renderer->window->currentFrame]);
    
    vkResetCommandBuffer(renderer->commands[renderer->window->currentFrame], 0);
    recordCommandBuffer(renderer, imageIndex);
    
    VkSubmitInfo submitInfo;
    memset(&submitInfo, 0, sizeof(VkSubmitInfo));
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &renderer->window->imageAvailableSemaphores[renderer->window->currentFrame];
    VkPipelineStageFlags waitFlag = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    submitInfo.pWaitDstStageMask = &waitFlag;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &renderer->commands[renderer->window->currentFrame];
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &renderer->window->renderFinishedSemaphores[renderer->window->currentFrame];
    
    if (vkQueueSubmit(renderer->window->graphicQueueFamily.queues[0], 1, &submitInfo, renderer->window->inFlightFences[renderer->window->currentFrame]) != VK_SUCCESS) {
        print_error("unabled to run the commands");
        return 0;
    }
    
    VkPresentInfoKHR presentInfo;
    memset(&presentInfo, 0, sizeof(VkPresentInfoKHR));
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &renderer->window->renderFinishedSemaphores[renderer->window->currentFrame];
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &renderer->window->swapchain;
    presentInfo.pImageIndices = &imageIndex;
    
    if (vkQueuePresentKHR(renderer->window->graphicQueueFamily.queues[0], &presentInfo) != VK_SUCCESS) {
        print_error("unabled to present to the window");
        return 0;
    }
    
    renderer->window->currentFrame = (renderer->window->currentFrame+1) % MAX_FRAMES_IN_FLIGHT;
    
    return 1;
}

int addModel(Renderer *renderer, char filename[], vec4 color) {
    float *vertices;
    uint16_t *indices;
    int indicesSize;
    int verticesSize;
    acquireData(&vertices, &indices, &verticesSize, &indicesSize, filename);
    Model *model = createModel(renderer->window, verticesSize, vertices, indicesSize*sizeof(uint16_t), indices);
    MEM_free(vertices);
    MEM_free(indices);
    
    renderer->models = MEM_realloc(renderer->models, (renderer->modelCount+1)*sizeof(Model*));
    renderer->models[renderer->modelCount] = model;
    renderer->modelCount += 1;
    return renderer->modelCount - 1;
}

Object *addObject(Renderer *renderer, int model, vec3 pos, vec3 orientation, float scale, vec4 color) {
    Object *obj = createObject(model, pos, orientation, scale, color);
    
    renderer->objects = MEM_realloc(renderer->objects, (renderer->objectCount+1)*sizeof(Object*));
    renderer->objects[renderer->objectCount] = obj;
    renderer->objectCount++;
    return obj;
}

void getObjectMatrix(Renderer *r, Object *obj, mat4 dest) {
    mat4 _dest =  GLM_MAT4_IDENTITY_INIT;
    glm_translate(_dest, obj->pos);
    vec3 scale = {obj->scale, obj->scale, obj->scale};
    glm_scale(_dest, scale);
    glm_rotate(_dest, obj->orientation[0], (vec3){1.f, 0.f, 0.f});
    glm_rotate(_dest, obj->orientation[1], (vec3){0.f, 1.f, 0.f});
    glm_rotate(_dest, obj->orientation[2], (vec3){0.f, 0.f, 1.f});
    
    mat4 view;
    getCameraMatrix(r->cam, view);
    
    mat4 temp;
    glm_mat4_mul(view, _dest, temp);
    glm_mat4_mul(r->proj, temp, dest);
}

void moveGlobalCamera(Renderer *renderer, vec3 displacment) {
    moveCamera(renderer->cam, displacment);
}

void setGlobalCamera(Renderer *renderer, vec3 pos) {
    setCamera(renderer->cam, pos);
}

float *getGlobalCameraPosition(Renderer *renderer) {
    return renderer->cam->pos;
}

float *getGlobalCameraUp(Renderer *renderer) {
    return renderer->cam->up;
}

UI *getUI(Renderer *renderer) {
    return renderer->ui;
}

//-----------------------------------------------//
//-     Local functions                         -//
//-----------------------------------------------//

void print_error(char str[]) {
    fprintf(stderr, "[ \033[31mERROR\033[29m ] : initialization : %s\n", str);
}

void print_warning(char str[]) {
#ifdef DEBUG
    fprintf(stderr, "[ \033[32mWARNING\033[29m ] : initialization : %s\n", str);
#endif
}

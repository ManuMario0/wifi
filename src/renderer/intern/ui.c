//
//  ui.c
//  AgeOfAnarchyXCode
//
//  Created by Emmanuel Mera on 04/03/2023.
//

#include <string.h>

#include "ui.h"
#include "KER_list.h"
#include "MEM_alloc.h"

/*
 NOTE: I can have a bigger stride length than what I put in it : don't have to use two dinstinct buffers for textured or not textured box
 */

//-----------------------------------------------//
//-     Local functions                         -//
//-----------------------------------------------//

static uint32_t *get_source_code(const char *file, size_t *size);
static void textRenderingAuxiliaryProcedure(UI *ui, VkCommandBuffer cBuffer, List *l);

//-----------------------------------------------//
//-     Implementation                          -//
//-----------------------------------------------//

UI *createUI(Window *window) {
    UI *ui = MEM_calloc(sizeof(UI), __func__);
    
    FT_Init_FreeType(&ui->ftLibHandle);
    
    float vert[] = {
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        1.0f, -1.0, 0.0f, 0.0f, 0.0f, 0.0f,
        -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f
    };
    uint16_t indices[] = {
        0, 1, 2, 1, 3, 2
    };
    
    float textureVert[] = {
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.f, 0.f,
        1.0f, -1.0, 0.0f, 0.0f, 0.0f, 0.0f, 1.f, 0.f,
        -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.f, 1.f,
        1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.f, 1.f
    };
    ui->colorModel = createModel(window, sizeof(vert), vert, sizeof(indices), indices);
    ui->textureModel = createModel(window, sizeof(textureVert), textureVert, sizeof(indices), indices);
    ui->window = window;
    
    return ui;
}

void destroyUI(Window *window, UI *ui) {
    clearUI(ui);
    destroyModel(window, ui->textureModel);
    destroyModel(window, ui->colorModel);
    MEM_free(ui);
}

void clearUI(UI *ui) {
    if (ui->rect) {
        for (int i=0; i<ui->rectCount; i++) {
            if (ui->rect[i].isTextured)
                destroyTexture(ui->window, ui->rect[i].tex);
        }
        MEM_free(ui->rect);
    }
    listDestroyList(ui->text, (void (*)(void*))destroyText);
    ui->text = NULL;
    ui->rect = NULL;
    ui->rectCount = 0;
}

Rectangle *addRectangle(UI *ui, int x, int y, int width, int height, vec4 color, vec4 borderColor) {
    ui->rect = MEM_realloc(ui->rect, (ui->rectCount+1)*sizeof(Rectangle));
#ifdef __APPLE__
    ui->rect[ui->rectCount].rect.offset.x = 4*x;
    ui->rect[ui->rectCount].rect.offset.y = 4*y;
    ui->rect[ui->rectCount].rect.extent.width = 2*width;
    ui->rect[ui->rectCount].rect.extent.height = 2*height;
#else
    ui->rect[ui->rectCount].rect.offset.x = x;
    ui->rect[ui->rectCount].rect.offset.y = y;
    ui->rect[ui->rectCount].rect.extent.width = width;
    ui->rect[ui->rectCount].rect.extent.height = height;
#endif
    memcpy(ui->rect[ui->rectCount].color, color, sizeof(vec4));
    memcpy(ui->rect[ui->rectCount].borderColor, borderColor, sizeof(vec4));
    ui->rectCount++;
    return &ui->rect[ui->rectCount-1];
}

void addTexture(UI *ui, Rectangle *rect, char filename[]) {
    rect->isTextured = 1;
    rect->tex = loadTexture(ui->window, filename);
}

void recordUICommandBuffer(UI *ui, Window *window, VkCommandBuffer buffer) {
    VkOffset2D offset = {0, 0};
    VkExtent2D extent = window->swapchainSize;
    
    VkDeviceSize bufferOffset = 0;
    vkCmdBindIndexBuffer(buffer, ui->colorModel->buffer->indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT16);
    
    vkCmdSetStencilReference(buffer, VK_STENCIL_FACE_FRONT_BIT, 1);
    vkCmdSetStencilWriteMask(buffer, VK_STENCIL_FACE_FRONT_BIT, 0);
    for (int i=0; i<ui->rectCount; i++) {
        if (ui->rect[i].isTextured) {
            vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, window->texturePipeline);
            
            VkViewport viewport;
            viewport.x = 0.f;
            viewport.y = 0.f;
            viewport.width = (float)window->swapchainSize.width;
            viewport.height = (float)window->swapchainSize.height;
            viewport.minDepth = 0.f;
            viewport.maxDepth = 1.f;
            vkCmdSetViewport(buffer, 0, 1, &viewport);
            
            VkRect2D scissor;
            scissor.offset = offset;
            scissor.extent = extent;
            vkCmdSetScissor(buffer, 0, 1, &scissor);
            
            vkCmdBindDescriptorSets(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, window->texturePipelineLayout, 0, 1, &ui->rect[i].tex->descriptorSets[window->currentFrame], 0, NULL);
        } else {
            vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, window->colorPipeline);
            
            VkViewport viewport;
            viewport.x = 0.f;
            viewport.y = 0.f;
            viewport.width = (float)window->swapchainSize.width;
            viewport.height = (float)window->swapchainSize.height;
            viewport.minDepth = 0.f;
            viewport.maxDepth = 1.f;
            vkCmdSetViewport(buffer, 0, 1, &viewport);
            
            VkRect2D scissor;
            scissor.offset = offset;
            scissor.extent = extent;
            vkCmdSetScissor(buffer, 0, 1, &scissor);
        }
        
        mat4 transform = GLM_MAT4_IDENTITY_INIT;
        
        if ((int)ui->rect[i].rect.extent.width >= 0) {
            glm_scale(transform, (vec3){(float)ui->rect[i].rect.extent.width/(float)window->swapchainSize.width, 1.0f, 1.0f});
            transform[3][0] = (float)ui->rect[i].rect.offset.x/(float)window->swapchainSize.width - 1. + (float)ui->rect[i].rect.extent.width/(float)window->swapchainSize.width;
        } else {
            glm_scale(transform, (vec3){-(float)(int)ui->rect[i].rect.extent.width/(float)window->swapchainSize.width, 1.0f, 1.0f});
            transform[3][0] = (float)ui->rect[i].rect.offset.x/(float)window->swapchainSize.width - 1. + (float)(int)ui->rect[i].rect.extent.width/(float)window->swapchainSize.width;
        }
        if ((int)ui->rect[i].rect.extent.height >= 0) {
            glm_scale(transform, (vec3){1.0f, (float)ui->rect[i].rect.extent.height/(float)window->swapchainSize.height, 1.0f});
            transform[3][1] = (float)ui->rect[i].rect.offset.y/(float)window->swapchainSize.height - 1. + (float)ui->rect[i].rect.extent.height/(float)window->swapchainSize.height;
        } else {
            glm_scale(transform, (vec3){1.0f, -(float)(int)ui->rect[i].rect.extent.height/(float)window->swapchainSize.height, 1.0f});
            transform[3][1] = (float)ui->rect[i].rect.offset.y/(float)window->swapchainSize.height - 1. + (float)(int)ui->rect[i].rect.extent.height/(float)window->swapchainSize.height;
        }
        
        vkCmdPushConstants(buffer, window->colorPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(mat4), transform);
        vkCmdPushConstants(buffer, window->colorPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, sizeof(mat4), sizeof(vec4), ui->rect[i].color);
        float light = 1.;
        vkCmdPushConstants(buffer, window->colorPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, sizeof(mat4)+sizeof(vec4), sizeof(float), &light);
        
        if (ui->rect[i].isTextured) {
            vkCmdBindVertexBuffers(buffer, 0, 1, &ui->textureModel->buffer->vertexBuffer.buffer, &bufferOffset);
            vkCmdDrawIndexed(buffer, ui->textureModel->indexCount, 1, 0, 0, 0);
        } else {
            vkCmdBindVertexBuffers(buffer, 0, 1, &ui->colorModel->buffer->vertexBuffer.buffer, &bufferOffset);
            vkCmdDrawIndexed(buffer, ui->colorModel->indexCount, 1, 0, 0, 0);
        }
    }
    textRenderingAuxiliaryProcedure(ui, buffer, ui->text);
}

void resetRectangle(Rectangle *rect, int x, int y, int width, int height, vec4 color, vec4 borderColor) {
#ifdef __APPLE__
    rect->rect.offset.x = 4*x;
    rect->rect.offset.y = 4*y;
    rect->rect.extent.width = 2*width;
    rect->rect.extent.height = 2*height;
#else
    rect->rect.offset.x = x;
    rect->rect.offset.y = y;
    rect->rect.extent.width = width;
    rect->rect.extent.height = height;
#endif
    memcpy(rect->color, color, sizeof(vec4));
    memcpy(rect->borderColor, borderColor, sizeof(vec4));
}

void resizeRectangle(Rectangle *rect, int width, int height) {
#ifdef __APPLE__
    rect->rect.extent.width = 2*width;
    rect->rect.extent.height = 2*height;
#else
    rect->rect.extent.width = width;
    rect->rect.extent.height = height;
#endif
}

void placeRectangle(Rectangle *rect, int x, int y) {
#ifdef __APPLE__
    rect->rect.offset.x = 4*x;
    rect->rect.offset.y = 4*y;
#else
    rect->rect.offset.x = x;
    rect->rect.offset.y = y;
#endif
}

// TODO: make this efficient (after remaking the texture module)
Font *loadTextFont(UI *ui, char filename[]) {
    Font *font = MEM_malloc(sizeof(Font), __func__);
    FT_Face face;
    int error = FT_New_Face(ui->ftLibHandle, filename, 0, &face);
    FT_Set_Pixel_Sizes(face, 0, 255);
    unsigned char *tex = MEM_calloc_array(256*256*4, sizeof(unsigned char), __func__);
    for (int i=33; i<=127; i++) {
        FT_Load_Char(face, (unsigned char)i, FT_LOAD_RENDER);
        for (int j=0; j<face->glyph->bitmap.width*face->glyph->bitmap.rows; j++) {
            tex[j*4] = tex[j*4+1] = tex[j*4+2] = 255;
            tex[j*4+3] =  face->glyph->bitmap.buffer[j];
        }
        font->characters[i-33].tex = createTexture(ui->window, tex, face->glyph->bitmap.width, face->glyph->bitmap.rows, 4);
        font->characters[i-33].width = face->glyph->bitmap.width;
        font->characters[i-33].rows = face->glyph->bitmap.rows;
        font->characters[i-33].left = face->glyph->bitmap_left;
        font->characters[i-33].top = face->glyph->bitmap_top;
        font->characters[i-33].advance = face->glyph->advance.x >> 6;
    }
    MEM_free(tex);
    return font;
}

void destroyFont(UI *ui, Font *font) {
    vkDeviceWaitIdle(ui->window->device);
    for (int i=33; i<=127; i++) {
        destroyTexture(ui->window, font->characters[i-33].tex);
    }
    MEM_free(font);
}

Text *addText(UI *ui, Font *font, char text[], int x, int y, vec3 color, float scale) {
    Text *t = MEM_malloc(sizeof(Text), __func__);
    t->text = text;
    t->pos.x = x;
    t->pos.y = y;
    t->font = font;
    t->scale = scale;
    memcpy(t->color, color, sizeof(vec3));
    List *cell = listCreateCell(t);
    ui->text = listCons(cell, ui->text);
    
    return t;
}

void destroyText(Text *text) {
    MEM_free(text);
}

void modifyText(Text *t, char content[]) {
    t->text = content;
}

void addDynamicText(void) {
    
}

void renderText(UI *ui, VkCommandBuffer cBuffer, Text *text) {
    Window *window = ui->window;
    
    VkOffset2D offset = {0, 0};
    VkExtent2D extent = window->swapchainSize;
    
    VkDeviceSize bufferOffset = 0;
    vkCmdBindVertexBuffers(cBuffer, 0, 1, &ui->textureModel->buffer->vertexBuffer.buffer, &bufferOffset);
    vkCmdBindIndexBuffer(cBuffer, ui->colorModel->buffer->indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT16);
    
    vkCmdSetStencilReference(cBuffer, VK_STENCIL_FACE_FRONT_BIT, 1);
    vkCmdSetStencilWriteMask(cBuffer, VK_STENCIL_FACE_FRONT_BIT, 0);
    
    vkCmdBindPipeline(cBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, window->texturePipeline);
    
    VkViewport viewport;
    viewport.x = 0.f;
    viewport.y = 0.f;
    viewport.width = (float)window->swapchainSize.width;
    viewport.height = (float)window->swapchainSize.height;
    viewport.minDepth = 0.f;
    viewport.maxDepth = 1.f;
    vkCmdSetViewport(cBuffer, 0, 1, &viewport);
    
    VkRect2D scissor;
    scissor.offset = offset;
    scissor.extent = extent;
    vkCmdSetScissor(cBuffer, 0, 1, &scissor);
    
    int x = text->pos.x, y = text->pos.y;
    float width = 0., height = 0.;
    char *t = text->text;
    while (*t != '\0') {
        char c = *t;
        
        Character currentChar = text->font->characters[c-33];
        
        vkCmdBindDescriptorSets(cBuffer,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                window->texturePipelineLayout,
                                0, 1,
                                &currentChar.tex->descriptorSets[window->currentFrame],
                                0, NULL);
        
        float xpos = x + currentChar.left * text->scale;
        float ypos = y - currentChar.top * text->scale;
        
        width = currentChar.width * text->scale;
        height = currentChar.rows * text->scale;
        
#ifdef __APPLE__
        xpos*=4;
        ypos*=4;
        width*=2;
        height*=2;
#endif
        
        mat4 transform = GLM_MAT4_IDENTITY_INIT;
        
        glm_scale(transform, (vec3){width/(float)window->swapchainSize.width, 1.0f, 1.0f});
        transform[3][0] = xpos/(float)window->swapchainSize.width - 1. + width/(float)window->swapchainSize.width;
        
        glm_scale(transform, (vec3){1.0f, height/(float)window->swapchainSize.height, 1.0f});
        transform[3][1] = ypos/(float)window->swapchainSize.height - 1. + height/(float)window->swapchainSize.height;
        
        vkCmdPushConstants(cBuffer, window->colorPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(mat4), transform);
        vkCmdPushConstants(cBuffer, window->colorPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, sizeof(mat4), sizeof(vec4), text->color);
        float light = 1.;
        vkCmdPushConstants(cBuffer, window->colorPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, sizeof(mat4)+sizeof(vec4), sizeof(float), &light);
        vkCmdDrawIndexed(cBuffer, ui->textureModel->indexCount, 1, 0, 0, 0);
        
        x += (int)((float)text->font->characters[c-33].advance * text->scale);
        t++;
    }
}

Button *addButton(UI *ui, int x, int y, int width, int height, char texture[]) {
    Button *button = MEM_malloc(sizeof(Button), __func__);
    button->rect = addRectangle(ui, x, y, width, height, (vec4){0}, (vec4){0});
    addTexture(ui, button->rect, texture);
    return button;
}

// TODO: make it plateform independent !!!!
int isButtonPress(UI *ui, Button *button) {
    double x, y;
    
    int state = glfwGetMouseButton(ui->window->glfwWindow, GLFW_MOUSE_BUTTON_LEFT);
    if (state == GLFW_PRESS || state == GLFW_REPEAT) {
        glfwGetCursorPos(ui->window->glfwWindow, &x, &y);
        return ( 4.*x > button->rect->rect.offset.x
                    && 4.*x < button->rect->rect.offset.x + button->rect->rect.extent.width*2
                    && 4.*y > button->rect->rect.offset.y
                    && 4.*y < button->rect->rect.offset.y + button->rect->rect.extent.height*2 );
    }
    
    return 0;
}

/*
 I want to make a in-game dynamic UI editor for simplicity, I don't know how easy or difficult it'll be but I'll try and find out myself hehe
 */
void launchUIEditor(Renderer *renderer) {
    UI *ui = renderer->ui;
    
    
    while (!glfwWindowShouldClose(ui->window->glfwWindow)) {
        glfwPollEvents();
        drawFrame(renderer);
    }
}

//-----------------------------------------------//
//-     Local functions                         -//
//-----------------------------------------------//

uint32_t *get_source_code(const char *file, size_t *size) {
    FILE *f = fopen(file, "rb");
    fseek(f, 0, SEEK_END);
    *size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    uint32_t *content = calloc(*size, sizeof(char));
    fread(content, 1, *size, f);
    
    fclose(f);
    return content;
}

void textRenderingAuxiliaryProcedure(UI *ui, VkCommandBuffer cBuffer, List *l) {
    if (l) {
        renderText(ui, cBuffer, listHead(l));
        textRenderingAuxiliaryProcedure(ui, cBuffer, listTail(l));
    }
}

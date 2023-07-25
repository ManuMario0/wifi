//
//  ui.h
//  AgeOfAnarchyXCode
//
//  Created by Emmanuel Mera on 04/03/2023.
//

#ifndef ui_h
#define ui_h

#include <stdio.h>

#include "KER_list.h"
#include "texture.h"
#include "renderer.h"

#include "cglm/cglm.h"
#include <ft2build.h>
#include FT_FREETYPE_H

typedef struct {
    Texture *   tex;
    int         width, rows, left, top;
    long        advance;
} Character;

typedef struct {
    Character   characters[130];
} Font;

typedef struct {
    Font *      font;
    char *      text;
    
    VkOffset2D  pos;
    vec3        color;
    float       scale;
} Text;

typedef struct {
    VkRect2D    rect;
    vec4        color;
    vec4        borderColor;
    
    int         isTextured;
    Texture     *tex;
} Rectangle;

typedef struct {
    Rectangle   *rect;
} Button;

typedef struct {
    FT_Library  ftLibHandle;
    
    Model       *colorModel;
    Model       *textureModel;
    Window      *window;
    
    int         rectCount;
    Rectangle   *rect;
    
    List        *text;
    List        *fonts;
} UI;

UI *createUI(Window *window);
void destroyUI(Window *window, UI *ui);
void clearUI(UI *ui);
Rectangle *addRectangle(UI *ui, int x, int y, int width, int height, vec4 color, vec4 borderColor);
void recordUICommandBuffer(UI *ui, Window *window, VkCommandBuffer buffer);
void renderText(UI *ui, VkCommandBuffer cBuffer, Text *text);
void destroyText(Text *text);

#endif /* ui_h */

//
//  command_buffer.h
//  AgeOfAnarchy
//
//  Created by Emmanuel Mera on 05/02/2023.
//

#ifndef command_buffer_h
#define command_buffer_h

#include "vulkan/vulkan.h"
#include "init.h"

int createCommandPool(Window *window, QueueFamily family, VkCommandPool *pool);
int destroyCommandPool(Window *window, VkCommandPool pool);
int createCommand(Window *window, VkCommandPool pool, VkCommandBuffer *buffer);

#endif /* command_buffer_h */

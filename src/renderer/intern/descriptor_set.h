//
//  descriptor_set.h
//  AgeOfAnarchy
//
//  Created by Emmanuel Mera on 12/03/2023.
//

#ifndef descriptor_set_h
#define descriptor_set_h

#include <stdio.h>

#include "init.h"

VkDescriptorPool createDescriptorPool(Window *window);
void destroyDescriptorPool(Window *window, VkDescriptorPool pool);
void createDescriptorSet(Window *window, VkDescriptorPool pool, VkDescriptorSetLayout *layout, uint32_t setCount, VkDescriptorSet *sets);

#endif /* descriptor_set_h */

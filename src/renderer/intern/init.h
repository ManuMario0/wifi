//
//  init.h
//  AgeOfAnarchy
//
//  Created by Emmanuel Mera on 03/02/2023.
//

#ifndef init_h
#define init_h

#include <vulkan/vulkan.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define MAX_FRAMES_IN_FLIGHT 2

typedef struct {
    uint32_t    family;
    uint32_t    queueCount;
    VkQueue *   queues;
} QueueFamily;

typedef struct {
    // GLFW stuff
    GLFWwindow *        glfwWindow;
    
    // Vulkan stuff
    VkInstance          instance;
    VkPhysicalDevice    physicalDevice;
    VkDevice            device;
    QueueFamily         graphicQueueFamily;
    QueueFamily         transferQueueFamily;
    
    VkSurfaceKHR        surface;
    
    VkSwapchainKHR      swapchain;
    VkExtent2D          swapchainSize;
    uint32_t            imageCount;
    VkImage *           images;
    VkImageView *       imageViews;
    
    VkFormat            depthFormat;
    VkImage             depthImage;
    VkDeviceMemory      depthImageMemory;
    VkImageView         depthImageView;
    
    VkFormat            imageFormat;
    VkRenderPass        renderPass;
    
    VkPipeline          colorPipeline;
    VkPipelineLayout    colorPipelineLayout;
    
    VkPipeline          texturePipeline;
    VkPipelineLayout    texturePipelineLayout;
    VkSampler           sampler;
    VkDescriptorPool    descriptorPool;
    VkDescriptorSetLayout   texDescriptorSetLayout;
    
    VkFramebuffer *     framebuffers;
    
    unsigned            currentFrame;
    VkSemaphore *       imageAvailableSemaphores;
    VkSemaphore *       renderFinishedSemaphores;
    VkFence *           inFlightFences;
} Window;

Window *createWindow(void);
int destroyWindow(Window *window);
int selectPhysicalDevice(Window *window);
int createLogicalDevice(Window *window);
int setupSwapchain(Window *window);
int createImageViews(Window *window);
int createDepthResources(Window *window);
int createRenderpass(Window *window);
int setupPipeline(Window *window);
int createFramebuffers(Window *window);
int createSync(Window *window);

#endif /* init_h */

//
//  init.c
//  AgeOfAnarchy
//
//  Created by Emmanuel Mera on 03/02/2023.
//

#include "init.h"
#include "MEM_alloc.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

//-----------------------------------------------//
//-     Local functions                         -//
//-----------------------------------------------//

static void print_error(char str[]);
static void print_warning(char str[]);
static void error_callback(int error, const char *description);
static const char **get_required_extensions(uint32_t *cc);
static int is_device_suitable(Window *window, VkPhysicalDevice device);
static uint32_t *get_source_code(const char *file, size_t *size);

//-----------------------------------------------//
//-     Implementation                          -//
//-----------------------------------------------//

Window *createWindow(void) {
    Window *window = MEM_malloc_aligned(sizeof(Window), 16, __func__);
    
    if(!glfwInit()) {
        print_error("unable to initialise glfw");
        MEM_free(window);
        return NULL;
    }
    
    glfwSetErrorCallback(&error_callback);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    GLFWmonitor* monitor =  glfwGetPrimaryMonitor();
    
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    window->glfwWindow = glfwCreateWindow(mode->width, mode->height, "Age of Anarchy", NULL, NULL);
    if (window->glfwWindow == NULL) {
        MEM_free(window);
        return NULL;
    }
    
    assert(glfwVulkanSupported() == 1);
    
    VkApplicationInfo appInfo;
    memset(&appInfo, 0, sizeof(VkApplicationInfo));
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Age of Anarchy";
    appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
    appInfo.pEngineName = "Unicorn Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);
    appInfo.apiVersion = VK_API_VERSION_1_3;
    
    VkInstanceCreateInfo instanceInfo;
    memset(&instanceInfo, 0, sizeof(VkInstanceCreateInfo));
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    
#ifdef __APPLE__
    instanceInfo.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif /* __APPLE__ */
    
    instanceInfo.pApplicationInfo = &appInfo;
    
#ifdef DEBUG
    instanceInfo.enabledLayerCount = 1;
    const char *layer = "VK_LAYER_KHRONOS_validation";
    instanceInfo.ppEnabledLayerNames = &layer;
#endif /* DEBUG */
    
    instanceInfo.ppEnabledExtensionNames = get_required_extensions(&instanceInfo.enabledExtensionCount);
    
    // instance creation
    if (vkCreateInstance(&instanceInfo, NULL, &window->instance) != VK_SUCCESS) {
        print_error("fail to create instance");
        MEM_free(window);
        MEM_free((void*)instanceInfo.ppEnabledExtensionNames);
        return NULL;
    }
    
    MEM_free((void*)instanceInfo.ppEnabledExtensionNames);
    
    // surface creation
    if (glfwCreateWindowSurface(window->instance, window->glfwWindow, NULL, &window->surface)) {
        print_error("unable to bind vulkan to the window");
        MEM_free(window);
        return NULL;
    }
    
    return window;
}

int destroyWindow(Window *window) {
    if (!window) {
        return 1;
    }
    
    vkDeviceWaitIdle(window->device);
    
    if (window->imageAvailableSemaphores) {
        for (int i=0; i<MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroySemaphore(window->device, window->imageAvailableSemaphores[i], NULL);
        }
        MEM_free(window->imageAvailableSemaphores);
    }
    
    if (window->renderFinishedSemaphores) {
        for (int i=0; i<MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroySemaphore(window->device, window->renderFinishedSemaphores[i], NULL);
        }
        MEM_free(window->renderFinishedSemaphores);
    }
    
    if (window->inFlightFences) {
        for (int i=0; i<MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroyFence(window->device, window->inFlightFences[i], NULL);
        }
        MEM_free(window->inFlightFences);
    }
    
    if (window->colorPipeline) {
        vkDestroyPipeline(window->device, window->colorPipeline, NULL);
    }
    
    if (window->colorPipelineLayout) {
        vkDestroyPipelineLayout(window->device, window->colorPipelineLayout, NULL);
    }
    
    if (window->renderPass) {
        vkDestroyRenderPass(window->device, window->renderPass, NULL);
    }
    
    if (window->framebuffers) {
        for (int i=0; i<window->imageCount; i++) {
            vkDestroyFramebuffer(window->device, window->framebuffers[i], NULL);
        }
        MEM_free(window->framebuffers);
    }
    
    if (window->imageViews) {
        for (int i=0; i<window->imageCount; i++) {
            vkDestroyImageView(window->device, window->imageViews[i], NULL);
        }
        MEM_free(window->imageViews);
    }
    
    if (window->images) {
        MEM_free(window->images);
    }
    
    if (window->depthImageView) {
        vkDestroyImageView(window->device, window->depthImageView, NULL);
    }
    
    if (window->depthImage) {
        vkDestroyImage(window->device, window->depthImage, NULL);
    }
    
    if (window->depthImageMemory) {
        vkFreeMemory(window->device, window->depthImageMemory, NULL);
    }
    
    if (window->swapchain) {
        vkDestroySwapchainKHR(window->device, window->swapchain, NULL);
    }
    
    if (window->device) {
        vkDestroyDevice(window->device, NULL);
        MEM_free(window->graphicQueueFamily.queues);
        MEM_free(window->transferQueueFamily.queues);
    }
    
    if (window->surface) {
        vkDestroySurfaceKHR(window->instance, window->surface, NULL);
    }
    
    if (window->instance) {
        vkDestroyInstance(window->instance, NULL);
    }
    
    if (window->glfwWindow) {
        glfwDestroyWindow(window->glfwWindow);
    }
    
    MEM_free(window);
    return 1;
}

int selectPhysicalDevice(Window *window) {
    uint32_t count = 0;
    if (vkEnumeratePhysicalDevices(window->instance, &count, NULL) != VK_SUCCESS) {
        print_error("unable to enumerate present physical devices");
        return 0;
    }
    if (count == 0) {
        print_error("no device found");
        return 0;
    }
    
    VkPhysicalDevice *physicalDevices = MEM_malloc(count*sizeof(VkPhysicalDevice), __func__);
    if (vkEnumeratePhysicalDevices(window->instance, &count, physicalDevices) != VK_SUCCESS) {
        print_error("unable to enumerate present physical devices");
        MEM_free(physicalDevices);
        return 0;
    }
    
    VkPhysicalDevice chosenPhysicalDevice = VK_NULL_HANDLE;
    for (int i=0; i<count; i++) {
        if (is_device_suitable(window, physicalDevices[i])) {
            chosenPhysicalDevice = physicalDevices[i];
            break;
        }
    }
    
    if (chosenPhysicalDevice == VK_NULL_HANDLE) {
        print_error("unabled to find a suitable device");
        MEM_free(physicalDevices);
        return 0;
    }
    
    window->physicalDevice = chosenPhysicalDevice;
    
    MEM_free(physicalDevices);
    return 1;
}

int createLogicalDevice(Window *window) {
    uint32_t count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(window->physicalDevice, &count, NULL);
    if (count == 0) {
        print_warning("unabled to find a queue on this device");
        return 0;
    }
    
    VkQueueFamilyProperties *queueFamilyProperties = MEM_malloc(count*sizeof(VkQueueFamilyProperties), __func__);
    vkGetPhysicalDeviceQueueFamilyProperties(window->physicalDevice, &count, queueFamilyProperties);
    uint32_t selectedQueueFamily[2] = {count, count}; // first family for graphics, second for transfer
    for (int i=0; i<count; i++) {
        if (selectedQueueFamily[0] == count
            && queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT
            && (queueFamilyProperties[i].queueCount != 0)) {
            selectedQueueFamily[0] = i;
        }
        if (selectedQueueFamily[1] == count
            && queueFamilyProperties[i].queueFlags & VK_QUEUE_TRANSFER_BIT
            && selectedQueueFamily[0] != i
            && (queueFamilyProperties[i].queueCount != 0)) {
            selectedQueueFamily[1] = i;
        }
    }
    
    if (selectedQueueFamily[0] == count || selectedQueueFamily[1] == count) {
        print_warning("unabled to find a suitable queue on this device");
        MEM_free(queueFamilyProperties);
        return 0;
    }
    
    VkDeviceQueueCreateInfo queueInfo[2];
    memset(queueInfo, 0, 2*sizeof(VkDeviceQueueCreateInfo));
    queueInfo[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueInfo[0].queueFamilyIndex = selectedQueueFamily[0];
    queueInfo[0].queueCount = queueFamilyProperties[selectedQueueFamily[0]].queueCount;
    float *priorGraphics = MEM_malloc(queueInfo[0].queueCount*sizeof(float), __func__);
    for (int i=0; i<queueInfo[0].queueCount; i++) {
        priorGraphics[i] = 1.F;
    }
    queueInfo[0].pQueuePriorities = priorGraphics;
    window->graphicQueueFamily.queueCount = queueInfo[0].queueCount;
    
    queueInfo[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueInfo[1].queueFamilyIndex = selectedQueueFamily[1];
    queueInfo[1].queueCount = queueFamilyProperties[selectedQueueFamily[1]].queueCount;
    float *priorTransfer = MEM_malloc(queueInfo[1].queueCount*sizeof(float), __func__);
    for (int i=0; i<queueInfo[1].queueCount; i++) {
        priorTransfer[i] = 1.F;
    }
    queueInfo[1].pQueuePriorities = priorTransfer;
    window->transferQueueFamily.queueCount = queueInfo[1].queueCount;
    
    MEM_free(queueFamilyProperties);
    
    VkDeviceCreateInfo deviceInfo;
    memset(&deviceInfo, 0, sizeof(VkDeviceCreateInfo));
    deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceInfo.queueCreateInfoCount = 2;
    deviceInfo.pQueueCreateInfos = queueInfo;
#ifdef __APPLE__
    deviceInfo.enabledExtensionCount = 2;
    const char **extensionName = MEM_calloc_array(2, sizeof(char *), __func__);
    extensionName[0] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
    extensionName[1] = "VK_KHR_portability_subset";
#else
    deviceInfo.enabledExtensionCount = 1;
    const char **extensionName = MEM_calloc_array(1, sizeof(char *), __func__);
    extensionName[0] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
#endif
    deviceInfo.ppEnabledExtensionNames = extensionName;
    if (vkCreateDevice(window->physicalDevice, &deviceInfo, NULL, &window->device) != VK_SUCCESS) {
        print_warning("unabled to create the logical device");
        MEM_free(extensionName);
        MEM_free(priorGraphics);
        MEM_free(priorTransfer);
        return 0;
    }
    MEM_free(extensionName);
    MEM_free(priorGraphics);
    MEM_free(priorTransfer);
    
    window->graphicQueueFamily.queues = MEM_malloc_aligned(window->graphicQueueFamily.queueCount*sizeof(VkQueue), 32, __func__);
    for (int i=0; i<window->graphicQueueFamily.queueCount; i++) {
        vkGetDeviceQueue(window->device, selectedQueueFamily[0], i, &window->graphicQueueFamily.queues[i]);
    }
    window->graphicQueueFamily.family = selectedQueueFamily[0];
    
    window->transferQueueFamily.queues = MEM_malloc_aligned(window->transferQueueFamily.queueCount*sizeof(VkQueue), 32, __func__);
    for (int i=0; i<window->transferQueueFamily.queueCount; i++) {
        vkGetDeviceQueue(window->device, selectedQueueFamily[1], i, &window->transferQueueFamily.queues[i]);
    }
    window->transferQueueFamily.family = selectedQueueFamily[1];
    
    return 1;
}

int setupSwapchain(Window *window) {
    uint32_t surfaceFormatCount;
    if (vkGetPhysicalDeviceSurfaceFormatsKHR(window->physicalDevice, window->surface, &surfaceFormatCount, NULL) != VK_SUCCESS) {
        print_error("unabled to retrieve available surface format");
        return 0;
    }
    
    VkSurfaceFormatKHR *availableFormats = MEM_calloc_array(surfaceFormatCount, sizeof(VkSurfaceFormatKHR), __func__);
    if (vkGetPhysicalDeviceSurfaceFormatsKHR(window->physicalDevice, window->surface, &surfaceFormatCount, availableFormats) != VK_SUCCESS) {
        print_error("unabled to retrieve available surface format");
        MEM_free(availableFormats);
        return 0;
    }
    
    VkSurfaceFormatKHR surfaceFormat = availableFormats[0];
    for (int i=0; i<surfaceFormatCount; i++) {
        if (availableFormats[i].format == VK_FORMAT_B8G8R8A8_SRGB && availableFormats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            surfaceFormat = availableFormats[i];
            break;
        }
    }
    MEM_free(availableFormats);
    window->imageFormat = surfaceFormat.format;
    
    
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(window->physicalDevice, window->surface, &surfaceCapabilities) != VK_SUCCESS) {
        print_error("unable to retrieve surface capabilities");
        return 0;
    }
    
    uint32_t presentCount = 0;
    if (vkGetPhysicalDeviceSurfacePresentModesKHR(window->physicalDevice, window->surface, &presentCount, NULL) != VK_SUCCESS) {
        print_error("unabled to retrieve available surface format");
        return 0;
    }
    
    VkPresentModeKHR *availablePresentModes = MEM_calloc_array(presentCount, sizeof(VkPresentModeKHR), __func__);
    if (vkGetPhysicalDeviceSurfacePresentModesKHR(window->physicalDevice, window->surface, &presentCount, availablePresentModes) != VK_SUCCESS) {
        print_error("unabled to retrieve available surface format");
        MEM_free(availablePresentModes);
        return 0;
    }
    
    VkPresentModeKHR presentMode = availablePresentModes[0];
    for (int i=0; i<presentCount; i++) {
        if (availablePresentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
            presentMode = availablePresentModes[i];
            break;
        }
    }
    MEM_free(availablePresentModes);
    
    VkSwapchainCreateInfoKHR swapchainInfo;
    memset(&swapchainInfo, 0, sizeof(VkSwapchainCreateInfoKHR));
    swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainInfo.surface = window->surface;
    swapchainInfo.minImageCount = surfaceCapabilities.minImageCount;
    swapchainInfo.imageFormat = surfaceFormat.format;
    swapchainInfo.imageColorSpace = surfaceFormat.colorSpace;
    swapchainInfo.imageExtent = surfaceCapabilities.currentExtent;
    swapchainInfo.imageArrayLayers = 1;
    swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchainInfo.preTransform = surfaceCapabilities.currentTransform;
    swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainInfo.presentMode = presentMode;
    swapchainInfo.clipped = VK_TRUE;
    swapchainInfo.oldSwapchain = VK_NULL_HANDLE;
    
    vkCreateSwapchainKHR(window->device, &swapchainInfo, NULL, &window->swapchain);
    
    if (vkGetSwapchainImagesKHR(window->device, window->swapchain, &window->imageCount, NULL) != VK_SUCCESS) {
        print_error("unabled to get swapchain images");
        return 0;
    }
    window->images = MEM_calloc_array(window->imageCount, sizeof(VkImage), __func__);
    if (vkGetSwapchainImagesKHR(window->device, window->swapchain, &window->imageCount, window->images) != VK_SUCCESS) {
        print_error("unabled to get swapchain images");
        MEM_free(window->images);
        return 0;
    }
    
    window->swapchainSize = surfaceCapabilities.currentExtent;
    
    return 1;
}

int createImageViews(Window *window) {
    window->imageViews = MEM_malloc_aligned(window->imageCount*sizeof(VkImageView), 32, __func__);
    
    for (int i=0; i<window->imageCount; i++) {
        VkImageViewCreateInfo imageViewInfo;
        memset(&imageViewInfo, 0, sizeof(VkImageViewCreateInfo));
        imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewInfo.image = window->images[i];
        imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewInfo.format = window->imageFormat;
        imageViewInfo.components.r = VK_COMPONENT_SWIZZLE_R;
        imageViewInfo.components.g = VK_COMPONENT_SWIZZLE_G;
        imageViewInfo.components.b = VK_COMPONENT_SWIZZLE_B;
        imageViewInfo.components.a = VK_COMPONENT_SWIZZLE_A;
        imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageViewInfo.subresourceRange.baseMipLevel = 0;
        imageViewInfo.subresourceRange.levelCount = 1;
        imageViewInfo.subresourceRange.baseArrayLayer = 0;
        imageViewInfo.subresourceRange.layerCount = 1;
        if (vkCreateImageView(window->device, &imageViewInfo, NULL, &window->imageViews[i]) != VK_SUCCESS) {
            print_error("unabled to create image views");
            MEM_free(window->imageViews);
            return 0;
        }
    }
    
    return 1;
}

VkFormat findSupportedFormat(Window *window, VkFormat *candidats, int candidatsCount, VkImageTiling tiling, VkFormatFeatureFlags features) {
    for (int i=0; i<candidatsCount; i++) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(window->physicalDevice, candidats[i], &props);
        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
            return candidats[i];
        } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
            return candidats[i];
        }
    }
    return candidats[0];
}

int createDepthResources(Window *window) {
    VkFormat format = findSupportedFormat(window,
                                          (VkFormat[2]){VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
                                          2,
                                          VK_IMAGE_TILING_OPTIMAL,
                                          VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    
    
    VkImageCreateInfo imageInfo;
    imageInfo.flags = 0;
    imageInfo.pNext = NULL;
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = window->swapchainSize.width;
    imageInfo.extent.height = window->swapchainSize.height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    if (vkCreateImage(window->device, &imageInfo, NULL, &window->depthImage) != VK_SUCCESS) {
        print_error("unabled to create depth image");
        return 0;
    }
    
    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(window->device, window->depthImage, &memRequirements);
    
    VkMemoryAllocateInfo allocInfo;
    memset(&allocInfo, 0, sizeof(VkMemoryAllocateInfo));
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(window->physicalDevice, &memProperties);
    for (int i=0; i<memProperties.memoryTypeCount; i++) {
        if (memRequirements.memoryTypeBits & (i << 1)
            && (memProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) == VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
            allocInfo.memoryTypeIndex = i;
            break;
        }
    }
    
    if (vkAllocateMemory(window->device, &allocInfo, NULL, &window->depthImageMemory) != VK_SUCCESS) {
        print_error("unabled to create memory for depth image");
        return 0;
    }
    
    vkBindImageMemory(window->device, window->depthImage, window->depthImageMemory, 0);
    
    VkImageViewCreateInfo imageViewInfo;
    memset(&imageViewInfo, 0, sizeof(VkImageViewCreateInfo));
    imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewInfo.image = window->depthImage;
    imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageViewInfo.format = format;
    imageViewInfo.components.r = VK_COMPONENT_SWIZZLE_R;
    imageViewInfo.components.g = VK_COMPONENT_SWIZZLE_G;
    imageViewInfo.components.b = VK_COMPONENT_SWIZZLE_B;
    imageViewInfo.components.a = VK_COMPONENT_SWIZZLE_A;
    imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    imageViewInfo.subresourceRange.baseMipLevel = 0;
    imageViewInfo.subresourceRange.levelCount = 1;
    imageViewInfo.subresourceRange.baseArrayLayer = 0;
    imageViewInfo.subresourceRange.layerCount = 1;
    if (vkCreateImageView(window->device, &imageViewInfo, NULL, &window->depthImageView) != VK_SUCCESS) {
        print_error("unabled to create image views");
        MEM_free(window->imageViews);
        return 0;
    }
    
    window->depthFormat = format;
    
    return 1;
}

int createRenderpass(Window *window) {
    VkAttachmentDescription attachmentDescription;
    attachmentDescription.flags = 0;
    attachmentDescription.format = window->imageFormat;
    attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
    attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    
    VkAttachmentDescription depthAttachment;
    depthAttachment.flags = 0;
    depthAttachment.format = window->depthFormat;
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    
    VkAttachmentReference colorAtt;
    colorAtt.attachment = 0;
    colorAtt.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    
    VkAttachmentReference depthAttachmentRef;
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    
    VkSubpassDescription subpassDescription;
    subpassDescription.flags = 0;
    subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDescription.inputAttachmentCount = 0;
    subpassDescription.pInputAttachments = NULL;
    subpassDescription.colorAttachmentCount = 1;
    subpassDescription.pColorAttachments = &colorAtt;
    subpassDescription.pResolveAttachments = NULL;
    subpassDescription.pDepthStencilAttachment = &depthAttachmentRef;
    subpassDescription.preserveAttachmentCount = 0;
    subpassDescription.pPreserveAttachments = NULL;
    
    VkSubpassDependency dependency;
    dependency.dependencyFlags = 0;
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    
    VkAttachmentDescription attachments[] = {attachmentDescription, depthAttachment};
    VkRenderPassCreateInfo renderPassInfo;
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.pNext = NULL;
    renderPassInfo.flags = 0;
    renderPassInfo.attachmentCount = 2;
    renderPassInfo.pAttachments = attachments;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpassDescription;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;
    
    if (vkCreateRenderPass(window->device, &renderPassInfo, NULL, &window->renderPass) != VK_SUCCESS) {
        return 0;
    }
    return 1;
}

int setupPipeline(Window *window) {
    VkShaderModuleCreateInfo vertShaderModuleInfo;
    memset(&vertShaderModuleInfo, 0, sizeof(VkShaderModuleCreateInfo));
    vertShaderModuleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    vertShaderModuleInfo.pCode = get_source_code("../Resources/vert.spv", &vertShaderModuleInfo.codeSize);
    VkShaderModule vertShaderModule;
    if (vkCreateShaderModule(window->device, &vertShaderModuleInfo, NULL, &vertShaderModule) != VK_SUCCESS) {
        print_error("unabled to create a shader module");
        MEM_free((void*)vertShaderModuleInfo.pCode);
        return 0;
    }
    MEM_free((void*)vertShaderModuleInfo.pCode);
    
    VkPipelineShaderStageCreateInfo vertShaderInfo;
    memset(&vertShaderInfo, 0, sizeof(VkPipelineShaderStageCreateInfo));
    vertShaderInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderInfo.pName = "main";
    vertShaderInfo.module = vertShaderModule;
    
    VkShaderModuleCreateInfo fragShaderModuleInfo;
    memset(&fragShaderModuleInfo, 0, sizeof(VkShaderModuleCreateInfo));
    fragShaderModuleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    fragShaderModuleInfo.pCode = get_source_code("../Resources/frag.spv", &fragShaderModuleInfo.codeSize);
    VkShaderModule fragShaderModule;
    if (vkCreateShaderModule(window->device, &fragShaderModuleInfo, NULL, &fragShaderModule) != VK_SUCCESS) {
        print_error("unabled to create a shader module");
        vkDestroyShaderModule(window->device, vertShaderModule, NULL);
        MEM_free((void*)fragShaderModuleInfo.pCode);
        return 0;
    }
    MEM_free((void*)fragShaderModuleInfo.pCode);
    
    VkPipelineShaderStageCreateInfo fragShaderInfo;
    memset(&fragShaderInfo, 0, sizeof(VkPipelineShaderStageCreateInfo));
    fragShaderInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderInfo.pName = "main";
    fragShaderInfo.module = fragShaderModule;
    
    VkPipelineShaderStageCreateInfo shaders[2] = {vertShaderInfo, fragShaderInfo};
    
    VkVertexInputBindingDescription bindingDescription;
    bindingDescription.binding = 0;
    bindingDescription.stride = 6*sizeof(float);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    
    VkVertexInputAttributeDescription posAttribute;
    posAttribute.binding = 0;
    posAttribute.location = 0;
    posAttribute.offset = 0;
    posAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
    
    VkVertexInputAttributeDescription normalAttribute;
    normalAttribute.binding = 0;
    normalAttribute.location = 1;
    normalAttribute.offset = 3*sizeof(float);
    normalAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
    
    VkVertexInputAttributeDescription attributes[] = {posAttribute, normalAttribute};
    
    VkPipelineVertexInputStateCreateInfo vertexInfo;
    memset(&vertexInfo, 0, sizeof(VkPipelineVertexInputStateCreateInfo));
    vertexInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInfo.vertexBindingDescriptionCount = 1;
    vertexInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInfo.vertexAttributeDescriptionCount = 2;
    vertexInfo.pVertexAttributeDescriptions = attributes;
    
    VkPipelineInputAssemblyStateCreateInfo assemblyInfo;
    memset(&assemblyInfo, 0, sizeof(VkPipelineInputAssemblyStateCreateInfo));
    assemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    assemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    assemblyInfo.primitiveRestartEnable = VK_FALSE;
    
    VkPipelineViewportStateCreateInfo viewportInfo;
    memset(&viewportInfo, 0, sizeof(VkPipelineViewportStateCreateInfo));
    viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportInfo.viewportCount = 1;
    viewportInfo.scissorCount = 1;
    
    VkPipelineRasterizationStateCreateInfo rasterizationInfo;
    memset(&rasterizationInfo, 0, sizeof(rasterizationInfo));
    rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationInfo.depthBiasClamp = VK_FALSE;
    rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
    rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizationInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizationInfo.depthBiasEnable = VK_FALSE;
    rasterizationInfo.lineWidth = 1.f;
    
    VkPipelineMultisampleStateCreateInfo multisampleInfo;
    memset(&multisampleInfo, 0, sizeof(VkPipelineMultisampleStateCreateInfo));
    multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampleInfo.sampleShadingEnable = VK_FALSE;
    
    VkPipelineColorBlendAttachmentState colorBlendAttachment;
    memset(&colorBlendAttachment, 0, sizeof(VkPipelineColorBlendAttachmentState));
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_A_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_R_BIT;
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_SUBTRACT;
    
    VkPipelineColorBlendStateCreateInfo colorBlendInfo;
    memset(&colorBlendInfo, 0, sizeof(VkPipelineColorBlendStateCreateInfo));
    colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendInfo.logicOpEnable = VK_FALSE;
    colorBlendInfo.attachmentCount = 1;
    colorBlendInfo.pAttachments = &colorBlendAttachment;
    
    VkPipelineDynamicStateCreateInfo dynamicStateInfo;
    memset(&dynamicStateInfo, 0, sizeof(VkPipelineDynamicStateCreateInfo));
    dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicStateInfo.dynamicStateCount = 4;
    VkDynamicState dynamicStates[] = {VK_DYNAMIC_STATE_SCISSOR, VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_STENCIL_REFERENCE, VK_DYNAMIC_STATE_STENCIL_WRITE_MASK};
    dynamicStateInfo.pDynamicStates = dynamicStates;
    
    VkPushConstantRange constantRange;
    constantRange.size = 21*sizeof(float);
    constantRange.offset = 0;
    constantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    
    VkPipelineLayoutCreateInfo layoutInfo;
    memset(&layoutInfo, 0, sizeof(VkPipelineLayoutCreateInfo));
    layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutInfo.setLayoutCount = 0;
    layoutInfo.pushConstantRangeCount = 1;
    layoutInfo.pPushConstantRanges = &constantRange;
    VkPipelineLayout layout;
    if (vkCreatePipelineLayout(window->device, &layoutInfo, NULL, &layout) != VK_SUCCESS) {
        print_error("unabled to create the pipeline layout");
        return 0;
    }
    window->colorPipelineLayout = layout;
    
    VkPipelineDepthStencilStateCreateInfo depthStencil;
    memset(&depthStencil, 0, sizeof(VkPipelineDepthStencilStateCreateInfo));
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    
    VkStencilOpState stencilState;
    memset(&stencilState, 0, sizeof(VkStencilOpState));
    stencilState.failOp = VK_STENCIL_OP_KEEP;
    stencilState.passOp = VK_STENCIL_OP_INCREMENT_AND_CLAMP;
    stencilState.depthFailOp = VK_STENCIL_OP_KEEP;
    stencilState.compareOp = VK_COMPARE_OP_GREATER_OR_EQUAL;
    stencilState.compareMask = 1;
    stencilState.writeMask = 1;
    stencilState.reference = 1;
    
    depthStencil.front = stencilState;
    
    VkGraphicsPipelineCreateInfo pipelineInfo;
    memset(&pipelineInfo, 0, sizeof(VkGraphicsPipelineCreateInfo));
    pipelineInfo.sType =                VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount =           2;
    pipelineInfo.pStages =              shaders;
    pipelineInfo.pVertexInputState =    &vertexInfo;
    pipelineInfo.pInputAssemblyState =  &assemblyInfo;
    pipelineInfo.pViewportState =       &viewportInfo;
    pipelineInfo.pRasterizationState =  &rasterizationInfo;
    pipelineInfo.pMultisampleState =    &multisampleInfo;
    pipelineInfo.pColorBlendState =     &colorBlendInfo;
    pipelineInfo.pDynamicState =        &dynamicStateInfo;
    pipelineInfo.pDepthStencilState =   &depthStencil;
    pipelineInfo.layout =               layout;
    pipelineInfo.renderPass =           window->renderPass;
    pipelineInfo.basePipelineHandle =   VK_NULL_HANDLE;
    
    if (vkCreateGraphicsPipelines(window->device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &window->colorPipeline) != VK_SUCCESS) {
        print_error("unabled to create the graphic pipeline");
        vkDestroyShaderModule(window->device, vertShaderModule, NULL);
        vkDestroyShaderModule(window->device, fragShaderModule, NULL);
        return 0;
    }
    
    vkDestroyShaderModule(window->device, vertShaderModule, NULL);
    vkDestroyShaderModule(window->device, fragShaderModule, NULL);
    
    return 1;
}

int createFramebuffers(Window *window) {
    window->framebuffers = MEM_calloc_array(window->imageCount, sizeof(VkFramebuffer), __func__);
    
    for (int i=0; i<window->imageCount; i++) {
        VkImageView attachments[] = {window->imageViews[i], window->depthImageView};
        
        VkFramebufferCreateInfo framebufferInfo;
        memset(&framebufferInfo, 0, sizeof(VkFramebufferCreateInfo));
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = window->renderPass;
        framebufferInfo.attachmentCount = 2;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = window->swapchainSize.width;
        framebufferInfo.height = window->swapchainSize.height;
        framebufferInfo.layers = 1;
        if (vkCreateFramebuffer(window->device, &framebufferInfo, NULL, &window->framebuffers[i]) != VK_SUCCESS) {
            print_error("unabled to create the framebuffer");
            MEM_free(window->framebuffers);
            return 0;
        }
    }
    
    return 1;
}

int createSync(Window *window) {
    window->imageAvailableSemaphores = MEM_calloc_array(MAX_FRAMES_IN_FLIGHT, sizeof(VkSemaphore), __func__);
    window->renderFinishedSemaphores = MEM_calloc_array(MAX_FRAMES_IN_FLIGHT, sizeof(VkSemaphore), __func__);
    window->inFlightFences = MEM_calloc_array(MAX_FRAMES_IN_FLIGHT, sizeof(VkFence), __func__);
    
    VkSemaphoreCreateInfo semaphoreInfo;
    memset(&semaphoreInfo, 0, sizeof(VkSemaphoreCreateInfo));
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    
    VkFenceCreateInfo fenceInfo;
    memset(&fenceInfo, 0, sizeof(VkFenceCreateInfo));
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    
    for (int i=0; i<MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(window->device, &semaphoreInfo, NULL, &window->imageAvailableSemaphores[i]) != VK_SUCCESS |
            vkCreateSemaphore(window->device, &semaphoreInfo, NULL, &window->renderFinishedSemaphores[i]) != VK_SUCCESS |
            vkCreateFence(window->device, &fenceInfo, NULL, &window->inFlightFences[i]) != VK_SUCCESS) {
            print_error("unabled to create sync objects");
            MEM_free(window->imageAvailableSemaphores);
            MEM_free(window->renderFinishedSemaphores);
            MEM_free(window->inFlightFences);
            return 0;
        }
    }
    
    return 1;
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

void error_callback(int error, const char *description) {
    fprintf(stderr, "[ \033[31mERROR\033[29m ] : glfw (error code : %d) : %s\n", error, description);
}

int check_layer_support(const char *layer) {
    uint32_t count;
    if (vkEnumerateInstanceLayerProperties(&count, NULL) != VK_SUCCESS) {
        print_error("unabled to get supported layers");
        return 0;
    }
    
    VkLayerProperties *layerProperties = MEM_calloc_array(count, sizeof(VkLayerProperties), __func__);
    if (vkEnumerateInstanceLayerProperties(&count, layerProperties) != VK_SUCCESS) {
        print_error("unabled to get supported layers");
        MEM_free(layerProperties);
        return 0;
    }
    
    for (int i=0; i<count; i++) {
        if (!strncmp(layer, layerProperties[i].layerName, VK_MAX_EXTENSION_NAME_SIZE)) {
            MEM_free(layerProperties);
            return 1;
        }
    }
    
    MEM_free(layerProperties);
    return 0;
}

const char **get_required_extensions(uint32_t *cc) {
    uint32_t count = 0;
    const char **glfwRequiredExtensions = glfwGetRequiredInstanceExtensions(&count);
    
    const char **requiredExtensions = MEM_calloc_array(count+2, sizeof(char *), __func__);
    memcpy(requiredExtensions, glfwRequiredExtensions, count*sizeof(char*));
    
#ifdef DEBUG
    if (check_layer_support("VK_LAYER_KHRONOS_validation")) {
        requiredExtensions[count] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
        count++;
    } else {
        print_warning("unabled to load validation layer");
    }
#endif /* DEBUG */
    
#ifdef __APPLE__
    requiredExtensions[count] = VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME;
    count++;
#endif /* __APPLE__ */
    
    *cc = count;
    
    return requiredExtensions;
}

int is_device_suitable(Window *window, VkPhysicalDevice device) {
    if (!glfwGetPhysicalDevicePresentationSupport(window->instance, device, 0)) {
        return 0;
    }
    
    uint32_t supportedExtensionsCount;
    if (vkEnumerateDeviceExtensionProperties(device, NULL, &supportedExtensionsCount, NULL) != VK_SUCCESS) {
        print_warning("unable to find physical device property");
        return 0;
    }
    
    VkExtensionProperties *supportedExtensions = MEM_calloc_array(supportedExtensionsCount, sizeof(VkExtensionProperties), __func__);
    if (vkEnumerateDeviceExtensionProperties(device, NULL, &supportedExtensionsCount, supportedExtensions) != VK_SUCCESS) {
        print_warning("unable to find physical device property");
        MEM_free(supportedExtensions);
        return 0;
    }
    
    int extensionsSupported = 0;
    uint32_t count;
    const char **requiredExtensions = get_required_extensions(&count);
    for (int i=0; i<count; i++) {
        extensionsSupported = 0;
        for (int j=0; j<supportedExtensionsCount; j++) {
            if (strncmp(requiredExtensions[i], supportedExtensions[j].extensionName, VK_MAX_EXTENSION_NAME_SIZE)) {
                extensionsSupported = 1;
                break;
            }
        }
        if (!extensionsSupported) {
            MEM_free(supportedExtensions);
            MEM_free(requiredExtensions);
            return 0;
        }
    }
    
    MEM_free(requiredExtensions);
    MEM_free(supportedExtensions);
    return 1;
}

uint32_t *get_source_code(const char *file, size_t *size) {
    FILE *f = fopen(file, "rb");
    fseek(f, 0, SEEK_END);
    *size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    uint32_t *content = MEM_calloc_array(*size, sizeof(char), __func__);
    fread(content, 1, *size, f);
    
    fclose(f);
    return content;
}

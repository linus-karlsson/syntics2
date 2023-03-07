#pragma once
#include "defines.h"

#define VKAPI_ATTR
#define VKAPI_CALL __stdcall

typedef struct VkQueue_T* VkQueue;
typedef struct VkBuffer_T* VkBuffer;
typedef struct VkDeviceMemory_T* VkDeviceMemory;
typedef u64 VkDeviceSize;
typedef struct VkImage_T* VkImage;
typedef struct VkDeviceMemory_T* VkDeviceMemory;
typedef struct VkImageView_T* VkImageView;
typedef struct VkSampler_T* VkSampler;
typedef struct VkDescriptorPool_T* VkDescriptorPool;
typedef struct VkDescriptorSet_T* VkDescriptorSet;
typedef struct VkPipeline_T* VkPipeline;
typedef struct VkPipelineLayout_T* VkPipelineLayout;
typedef struct VkDescriptorSetLayout_T* VkDescriptorSetLayout;
typedef struct VkSwapchainKHR_T* VkSwapchainKHR;
typedef struct VkFramebuffer_T* VkFramebuffer;
typedef struct VkRenderPass_T* VkRenderPass;
typedef struct VkPhysicalDevice_T* VkPhysicalDevice;
typedef struct VkDevice_T* VkDevice;
typedef struct VkSurfaceKHR_T* VkSurfaceKHR;
typedef struct VkCommandPool_T* VkCommandPool;
typedef struct VkCommandBuffer_T* VkCommandBuffer;
typedef struct VkFence_T* VkFence;
typedef struct VkSemaphore_T* VkSemaphore;
typedef struct VkInstance_T* VkInstance;
typedef struct VkSurfaceKHR_T* VkSurfaceKHR;
typedef struct VkDebugUtilsMessengerEXT_T* VkDebugUtilsMessengerEXT;
typedef struct VkRect2D VkRect2D;
typedef u32 VkBool32;

typedef struct VkExtent2D VkExtent2D;

typedef struct VkDebugUtilsMessengerCallbackDataEXT
    VkDebugUtilsMessengerCallbackDataEXT;
typedef struct VkDebugUtilsMessengerCreateInfoEXT VkDebugUtilsMessengerCreateInfoEXT;
typedef struct VkAllocationCallbacks VkAllocationCallbacks;

typedef uint32_t VkFlags;
typedef enum VkResult VkResult;
typedef enum VkDebugUtilsMessageSeverityFlagBitsEXT
    VkDebugUtilsMessageSeverityFlagBitsEXT;
typedef enum VkDebugUtilsMessageTypeFlagBitsEXT VkDebugUtilsMessageTypeFlagBitsEXT;
typedef VkFlags VkDebugUtilsMessageTypeFlagsEXT;
typedef VkFlags VkDebugUtilsMessageSeverityFlagsEXT;
typedef VkFlags VkDebugUtilsMessengerCreateFlagsEXT;
typedef enum VkCullModeFlagBits VkCullModeFlagBits;
typedef VkFlags VkCullModeFlags;
typedef enum VkImageViewType VkImageViewType;
typedef enum VkImageAspectFlagBits VkImageAspectFlagBits;
typedef VkFlags VkImageAspectFlags;
typedef enum VkImageTiling VkImageTiling;
typedef enum VkImageUsageFlagBits VkImageUsageFlagBits;
typedef VkFlags VkImageUsageFlags;
typedef enum VkMemoryPropertyFlagBits VkMemoryPropertyFlagBits;
typedef VkFlags VkMemoryPropertyFlags;
typedef enum VkPrimitiveTopology VkPrimitiveTopology;
typedef enum VkFormat VkFormat;
typedef enum VkSampleCountFlagBits VkSampleCountFlagBits;

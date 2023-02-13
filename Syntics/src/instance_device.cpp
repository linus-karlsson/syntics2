#include "instance_device.h"
#include "region_alloc.h"
#ifdef LINUX
#include <vulkan/vulkan_xcb.h>
#else
#include <vulkan/vulkan_win32.h>
#endif

typedef struct Instance_State
{
    VkInstance instance;
    VkDebugUtilsMessengerEXT debug_messenger;
} Instance_State;

static Instance_State internal_state = {};
static b8 INITILIZED = false;

const VkInstance& get_instance()
{
    if (!INITILIZED) SY_ERROR("Tyring to access intance that is not initialized");
    return internal_state.instance;
}
const VkDebugUtilsMessengerEXT& get_debug_messenger()
{
    if (!INITILIZED)
        SY_ERROR("Tyring to access debug messenger that is not initialized");
    return internal_state.debug_messenger;
}

void init_instance(Region_Alloc* region)
{
    if (INITILIZED) SY_ERROR("Instance already initialized");

    u32 version_supported = 0;
    VK_ASSERT(vkEnumerateInstanceVersion(&version_supported));
#if 0
    synt_LOG("\nVulkan Version: %u.%u.%u.%u\n",
             VK_API_VERSION_VARIANT(version_supported),
             VK_API_VERSION_MAJOR(version_supported),
             VK_API_VERSION_MINOR(version_supported),
             VK_API_VERSION_PATCH(version_supported));
#endif

    INIT_0(VkApplicationInfo, app_info);
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = "Sandy";
    app_info.applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
    app_info.pEngineName = "Syntics";
    app_info.engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
    app_info.apiVersion = VK_API_VERSION_1_3;

    u32 extension_count = 2;
    const char* extensions[3] = {
        VK_KHR_SURFACE_EXTENSION_NAME,
#ifdef LINUX
        VK_KHR_XCB_SURFACE_EXTENSION_NAME,
#else
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME
#endif
    };

    INIT_0(VkInstanceCreateInfo, info);
    info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    info.pApplicationInfo = &app_info;

    if (VALIDATIONS_ENABLE)
    {
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = config_debug_info();
        const char* validations[] = { "VK_LAYER_KHRONOS_validation" };
        info.enabledLayerCount = 1;
        info.ppEnabledLayerNames = validations;
        info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;

        extensions[extension_count++] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
    }

    info.enabledExtensionCount = extension_count;
    info.ppEnabledExtensionNames = extensions;

#if 0
    synt_LOG("\nExtensions used: \n");
    for (u32i = 0; i < extension_count; i++)
        synt_LOG("\t%s\n", extensions[i]);
    synt_LOG("\n");
#endif

    internal_state.instance = VK_NULL_HANDLE;

    VK_ASSERT(vkCreateInstance(&info, NULL, &internal_state.instance));

    INITILIZED = true;
}

VKAPI_ATTR VkBool32 VKAPI_CALL msg_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{

    if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
    {
        SY_ERROR(pCallbackData->pMessage);
    }

    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
    {
        synt_LOG_Term(pCallbackData->pMessage);
    }

    return VK_TRUE;
}

VkDebugUtilsMessengerCreateInfoEXT config_debug_info()
{
    INIT_0(VkDebugUtilsMessengerCreateInfoEXT, out);
    out.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    out.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                          VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                          VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    out.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                      VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                      VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    out.pfnUserCallback = msg_callback;

    return out;
}

void init_debug_messenger()
{
    if (!VALIDATIONS_ENABLE) return;

    VkDebugUtilsMessengerCreateInfoEXT messenger_info = config_debug_info();

    PFN_vkCreateDebugUtilsMessengerEXT callback =
        (PFN_vkCreateDebugUtilsMessengerEXT)(vkGetInstanceProcAddr(
            internal_state.instance, "vkCreateDebugUtilsMessengerEXT"));

    if (callback)
    {
        if (callback(internal_state.instance, &messenger_info, NULL,
                     &internal_state.debug_messenger))
            SY_ERROR("Failed to initialize debug messenger");
    }
    else
        SY_ERROR("Error extension is not present");
}

void destroy_debug_messenger(VkInstance instance,
                             VkDebugUtilsMessengerEXT debugMessenger,
                             const VkAllocationCallbacks* pAllocator)
{
    PFN_vkDestroyDebugUtilsMessengerEXT callback =
        (PFN_vkDestroyDebugUtilsMessengerEXT)(vkGetInstanceProcAddr(
            instance, "vkDestroyDebugUtilsMessengerEXT"));

    if (callback) callback(instance, debugMessenger, pAllocator);
}

Queue_Family_Indices get_queue_indices(Region_Alloc* region,
                                       VkPhysicalDevice physical_device,
                                       VkSurfaceKHR surface, b8* all_supported)
{
    u32 queue_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_count, NULL);

    Temp_Alloc<VkQueueFamilyProperties> queue_props(region, queue_count);

    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_count,
                                             queue_props.data);

    INIT_0(Queue_Family_Indices, indices);
    b8 graphic_supported = false;
    b8 presentation_supported = false;
    for (u32 i = 0; i < queue_count; i++)
    {
        if (queue_props.data[i].queueCount > 0 &&
            (queue_props.data[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) ==
                VK_QUEUE_GRAPHICS_BIT)
        {
            indices.indices[GRAPHICS_QUEUE_IDX] = i;

            graphic_supported = true;
        }
        VkBool32 surface_support = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, surface,
                                             &surface_support);
        if (surface_support && graphic_supported &&
            indices.indices[GRAPHICS_QUEUE_IDX] == i)
        {
            presentation_supported = true;
        }
    }
    *all_supported = graphic_supported && presentation_supported;

    b8 dublicate = false;
    indices.num_index_fam = 0;
    for (u32 i = 0; i < sy_SIZE(indices.indices); i++)
    {
        for (u32 j = 0; j < i; j++)
        {
            if (indices.indices[i] == indices.indices[j])
            {
                dublicate = true;
            }
        }
        if (!dublicate) indices.num_index_fam++;

        dublicate = false;
        dublicate = true;
    }

    return indices;
}

void pick_physical_device(Region_Alloc* region, VkInstance instance,
                          VkSurfaceKHR surface, VkPhysicalDevice* physical_device,
                          Queue_Family_Indices* q_indices)
{
    u32 device_count = 0;
    VK_ASSERT(vkEnumeratePhysicalDevices(instance, &device_count, NULL));

    Temp_Alloc<VkPhysicalDevice> physical_devices(region, device_count);

    VK_ASSERT(
        vkEnumeratePhysicalDevices(instance, &device_count, physical_devices.data));

    *physical_device = VK_NULL_HANDLE;

    Temp_Alloc<char*> buffer(region, device_count + 1);
    Temp_Alloc<VkPhysicalDeviceProperties> props(region, device_count);
    buffer.data[0] = (char*)"\nAvailable Physical devices: ";
    b8 supported = false;
    u32 device_index = 0;
    for (u32 i = 0; i < device_count; i++)
    {
        vkGetPhysicalDeviceProperties(physical_devices.data[i], &props.data[i]);
        buffer.data[i + 1] = props.data[i].deviceName;
        if (!supported)
        {
            *q_indices = get_queue_indices(region, physical_devices.data[i], surface,
                                           &supported);
            if (supported)
            {
                *physical_device = physical_devices.data[i];
                device_index = i;
            }
        }
    }
#if 0
    synt_LOG("%s\n\t", buffer.data[0]);
    for (u32i = 0; i < device_count; i++)
        synt_LOG("%s\n\t", buffer.data[i + 1]);


    synt_LOG("\nDevice in use: \n");
    synt_LOG("\t%s\n\n", props.data[device_index].deviceName);
#endif
    assert(physical_device);
}

void create_logical_device(VkPhysicalDevice physical_device,
                           Queue_Family_Indices q_indices, VkDevice* device)
{
    *device = VK_NULL_HANDLE;

    f32 queue_prio = 1.0f;
    INIT_ARR0(VkDeviceQueueCreateInfo, queue_infos, sy_SIZE(q_indices.indices));

    for (u32 i = 0; i < q_indices.num_index_fam; i++)
    {
        INIT_0(VkDeviceQueueCreateInfo, queue_info);
        queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_info.queueCount = 1;
        queue_info.pQueuePriorities = &queue_prio;
        queue_info.queueFamilyIndex = q_indices.indices[i];

        queue_infos[i] = queue_info;
    }

    // synt_LOG("\nNumber of queue indices: %u\n\n", q_indices.num_index_fam);

    const char* extensions[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    INIT_0(VkDeviceCreateInfo, device_info);
    device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_info.queueCreateInfoCount = q_indices.num_index_fam;
    device_info.pQueueCreateInfos = queue_infos;
    device_info.enabledExtensionCount = sy_SIZE(extensions);
    device_info.ppEnabledExtensionNames = extensions;

    VK_ASSERT(vkCreateDevice(physical_device, &device_info, NULL, device));
}

#ifdef LINUX
void create_surface(Linux_Platform xcb, VkSurfaceKHR* surface)
{
    INIT_0(VkXcbSurfaceCreateInfoKHR, surface_info);
    surface_info.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
    surface_info.connection = xcb.connection;
    surface_info.window = xcb.window;

    *surface = VK_NULL_HANDLE;
    VK_ASSERT(vkCreateXcbSurfaceKHR(internal_state.instance, &surface_info, NULL,
                                    surface));
}
#else
void create_surface(HWND win, VkSurfaceKHR* surface)
{
    INIT_0(VkWin32SurfaceCreateInfoKHR, surface_info);
    surface_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surface_info.hwnd = win;
    surface_info.hinstance = GetModuleHandle(0);

    *surface = VK_NULL_HANDLE;
    VK_ASSERT(vkCreateWin32SurfaceKHR(internal_state.instance, &surface_info, NULL,
                                      surface));
}
#endif

void destroy_instance()
{
    destroy_debug_messenger(internal_state.instance, internal_state.debug_messenger,
                            NULL);
    vkDestroyInstance(internal_state.instance, NULL);
}


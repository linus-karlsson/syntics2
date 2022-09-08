#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <vulkan/vulkan.h>
#include <xcb/xcb.h>
#include <vulkan/vulkan_xcb.h>
#include <assert.h>

typedef float f32;
typedef double f64;

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;

typedef uint32_t b32;
typedef uint8_t b8;

typedef int64_t i64;
typedef int32_t i32;
typedef int16_t i16;
typedef int8_t i8;

#ifdef DEBUG
static const b8 VALIDATIONS_ENABLE = 1;
#else
static const b8 VALIDATIONS_ENABLE = 0;
#endif

#define CALLOC(type, n) (type*)calloc(n, sizeof(type))
#define SIZE(array) (u32)(sizeof(array) / sizeof(array[0]))
#define VK_ASSERT(function)                                                         \
    {                                                                               \
        VkResult res = function;                                                    \
        assert(res == VK_SUCCESS);                                                  \
    }

typedef struct XCB_Props
{

    u32 width;
    u32 height;
    i32 screen_number;
    xcb_connection_t* connection;
    xcb_screen_t* screen;
    xcb_window_t window;

} XCB_Props;

XCB_Props initialize_xcb(u32 width, u32 height)
{

    XCB_Props xcb_props;
    xcb_props.screen_number = 0;

    xcb_props.connection = xcb_connect(NULL, &xcb_props.screen_number);

    // I'm using screen number one so don't need to iterate screens.
    const xcb_setup_t* setup   = xcb_get_setup(xcb_props.connection);
    xcb_screen_iterator_t iter = xcb_setup_roots_iterator(setup);
    xcb_props.screen           = iter.data;

    xcb_props.window = xcb_generate_id(xcb_props.connection);

    u32 mask         = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
    u32 value_list[] = {
        xcb_props.screen->black_pixel,

        XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE |
            XCB_EVENT_MASK_POINTER_MOTION | XCB_EVENT_MASK_ENTER_WINDOW |
            XCB_EVENT_MASK_LEAVE_WINDOW | XCB_EVENT_MASK_KEY_PRESS |
            XCB_EVENT_MASK_KEY_RELEASE,
    };

    xcb_create_window(xcb_props.connection, XCB_COPY_FROM_PARENT, xcb_props.window,
                      xcb_props.screen->root, 0, 0, width, height, 0,
                      XCB_WINDOW_CLASS_INPUT_OUTPUT, xcb_props.screen->root_visual,
                      mask, value_list);

    xcb_map_window(xcb_props.connection, xcb_props.window);

    xcb_flush(xcb_props.connection);

    xcb_props.height = height;
    xcb_props.width  = width;

    return xcb_props;
}

VkInstance create_instance()
{
    u32 version_supported = 0;
    VK_ASSERT(vkEnumerateInstanceVersion(&version_supported));
    printf("Vulkan Version: %u.%u.%u.%u\n",
           VK_API_VERSION_VARIANT(version_supported),
           VK_API_VERSION_MAJOR(version_supported),
           VK_API_VERSION_MINOR(version_supported),
           VK_API_VERSION_PATCH(version_supported));

    VkApplicationInfo app_info = {};
    app_info.sType             = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.apiVersion        = VK_API_VERSION_1_3;
    app_info.engineVersion     = VK_MAKE_API_VERSION(0, 1, 0, 0);
    app_info.pEngineName       = "Syntics";
    app_info.pApplicationName  = "Sandy";

    u32 extension_count       = 2;
    const char* extensions[3] = { VK_KHR_SURFACE_EXTENSION_NAME,
                                  VK_KHR_XCB_SURFACE_EXTENSION_NAME };

    VkInstanceCreateInfo info = {};
    info.sType                = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    info.pApplicationInfo     = &app_info;

    if (VALIDATIONS_ENABLE)
    {
        const char* validations[]     = { "VK_LAYER_KHRONOS_validation" };
        info.enabledLayerCount        = 1;
        info.ppEnabledLayerNames      = validations;
        extensions[extension_count++] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
    }

    info.enabledExtensionCount   = extension_count;
    info.ppEnabledExtensionNames = extensions;

    printf("\nExtensions used: \n");
    for (u32 i = 0; i < extension_count; i++)
        printf("\t%s\n", extensions[i]);

    VkInstance instance = VK_NULL_HANDLE;

    VK_ASSERT(vkCreateInstance(&info, NULL, &instance));

    return instance;
}

VkPhysicalDevice enumerate_py_devices(VkInstance instance)
{
    u32 device_count = 0;
    VK_ASSERT(vkEnumeratePhysicalDevices(instance, &device_count, NULL));
    VkPhysicalDevice* physical_devices = CALLOC(VkPhysicalDevice, device_count);
    VK_ASSERT(vkEnumeratePhysicalDevices(instance, &device_count, physical_devices));

    printf("\nPhysical devices: \n");
    u32 index = 0;
    for (u32 i = 0; i < device_count; i++)
    {
        VkPhysicalDeviceProperties props = {};
        vkGetPhysicalDeviceProperties(physical_devices[i], &props);
        printf("\t%s\n", props.deviceName);
    }

    VkPhysicalDevice physical_device = physical_devices[index];
    free(physical_devices);

    return physical_device;
}

#define GRAPHICS_QUEUE_IDX 0

typedef struct Queue_Family_Indices
{
    u32 indices[1];
} Queue_Family_Indices;

Queue_Family_Indices get_queue_indices(VkPhysicalDevice physical_device)
{
    u32 queue_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_count, NULL);

    VkQueueFamilyProperties* queue_props =
        CALLOC(VkQueueFamilyProperties, queue_count);

    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_count,
                                             queue_props);

    Queue_Family_Indices indices = {};
    for (u32 i = 0; i < queue_count; i++)
    {
        if ((queue_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) ==
            VK_QUEUE_GRAPHICS_BIT)
        {
            indices.indices[GRAPHICS_QUEUE_IDX] = i;
            break;
        }
    }
    free(queue_props);
    return indices;
}

VkDevice logical_device(VkPhysicalDevice physical_device,
                        Queue_Family_Indices q_indices)
{
    VkDevice device = VK_NULL_HANDLE;

    float queue_prio = 1.0f;
    VkDeviceQueueCreateInfo queue_infos[SIZE(q_indices.indices)];

    u8 dublicate_indices = 0;
    u32 actual_number    = 0;
    for (u32 i = 0; i < SIZE(q_indices.indices); i++)
    {
        for (u32 j = 0; j < actual_number; j++)
        {
            if (queue_infos[j].queueFamilyIndex == q_indices.indices[i])
                dublicate_indices = 1;
        }
        if (!dublicate_indices)
        {
            VkDeviceQueueCreateInfo queue_info = {};
            queue_info.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queue_info.queueCount       = 1;
            queue_info.pQueuePriorities = &queue_prio;
            queue_info.queueFamilyIndex = q_indices.indices[i];

            queue_infos[i] = queue_info;
            actual_number++;
        }
        dublicate_indices = 0;
    }

    printf("\nNumber of queue indices: %u\n", actual_number);

    VkDeviceCreateInfo device_info   = {};
    device_info.sType                = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_info.queueCreateInfoCount = actual_number;
    device_info.pQueueCreateInfos    = queue_infos;

    VK_ASSERT(vkCreateDevice(physical_device, &device_info, NULL, &device));

    return device;
}

VkSurfaceKHR get_surface(VkInstance instance, XCB_Props* xcb)
{
    VkXcbSurfaceCreateInfoKHR surface_info = {};
    surface_info.sType      = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
    surface_info.connection = xcb->connection;
    surface_info.window     = xcb->window;

    VkSurfaceKHR surface = VK_NULL_HANDLE;

    VK_ASSERT(vkCreateXcbSurfaceKHR(instance, &surface_info, NULL, &surface));

    return surface;
}

VkCommandPool create_command_pool(VkDevice device, u32 queue_fam_index)
{
    VkCommandPoolCreateInfo create_info = {};
    create_info.sType                   = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    create_info.queueFamilyIndex        = queue_fam_index;

    VkCommandPool command_pool = VK_NULL_HANDLE;
    VK_ASSERT(vkCreateCommandPool(device, &create_info, NULL, &command_pool));

    return command_pool;
}

void allocate_commandbuffer(VkDevice device, VkCommandPool command_pool,
                            VkCommandBuffer* command_buffer)
{

    VkCommandBufferAllocateInfo alloc_info = {};
    alloc_info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandPool        = command_pool;
    alloc_info.commandBufferCount = 1;

    VK_ASSERT(vkAllocateCommandBuffers(device, NULL, command_buffer));
}

void record_commandbuffer(VkCommandBuffer command_buffer) {}

int main()
{
    XCB_Props xcb = {};

    VkInstance instance = VK_NULL_HANDLE;

    VkPhysicalDevice physical_device = VK_NULL_HANDLE;
    VkDevice device                  = VK_NULL_HANDLE;
    VkQueue graphic_queue            = VK_NULL_HANDLE;
    Queue_Family_Indices q_indices   = {};

    VkSurfaceKHR surface = VK_NULL_HANDLE;

    VkCommandPool command_pool = VK_NULL_HANDLE;

    xcb = initialize_xcb(800, 600);

    instance = create_instance();
    assert(instance);

    physical_device = enumerate_py_devices(instance);
    assert(physical_device);

    q_indices = get_queue_indices(physical_device);

    device = logical_device(physical_device, q_indices);
    assert(device);

    surface = get_surface(instance, &xcb);
    assert(surface);

    vkGetDeviceQueue(device, q_indices.indices[GRAPHICS_QUEUE_IDX], 0,
                     &graphic_queue);
    assert(graphic_queue);

    command_pool =
        create_command_pool(device, q_indices.indices[GRAPHICS_QUEUE_IDX]);
    assert(command_pool);

    vkDestroyCommandPool(device, command_pool, NULL);
    vkDestroyDevice(device, NULL);
    vkDestroySurfaceKHR(instance, surface, NULL);
    vkDestroyInstance(instance, NULL);

    printf("\nComplete!\n");

    return 0;
}

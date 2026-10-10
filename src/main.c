#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define VK_USE_PLATFORM_WAYLAND_KHR
#include <vk_platform.h>
#include <glad/vulkan.h>

#define GLFW_INCLUDE_NONE
#define GLFW_EXPOSE_NATIVE_WAYLAND
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#define __TOSTRHLP(x) #x
#define TOSTR(x) __TOSTRHLP(x)
#define ABRT() { puts("\n\naborted at " TOSTR(__LINE__) ":" __FILE__); abort(); }

int main(void)
{
    if (!glfwInit())
    {
        puts("failed initializating GLFW");
        return 1;
    }

    if (!glfwVulkanSupported())
    {
        puts("Vulkan doesn't supported on this platform/device.");
        return 1;
    }

    if (!gladLoadVulkanUserPtr(NULL, (void *)glfwGetInstanceProcAddress, NULL)) ABRT();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow *w;
    if (!(w = glfwCreateWindow(800, 600, "Vulkan Test", NULL, NULL))) ABRT();

    static const char *exts[] = {"VK_KHR_wayland_surface", "VK_KHR_surface"};
    static const VkInstanceCreateInfo instancecreateinfo =
    {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = NULL,
        .pApplicationInfo = NULL,

        .enabledLayerCount = 0,
        .ppEnabledLayerNames = NULL,

        .enabledExtensionCount = sizeof(exts) / sizeof(const char *),
        .ppEnabledExtensionNames = exts, //glfwGetRequiredInstanceExtensions()

        .flags = 0
    };
    VkInstance instance;
    if (vkCreateInstance(&instancecreateinfo, NULL, &instance) != VK_SUCCESS) ABRT();
    if (!gladLoadVulkanUserPtr(NULL, (void *)glfwGetInstanceProcAddress, instance)) ABRT();
    
    /*
    VkWaylandSurfaceCreateInfoKHR waylandcreateinfo =
    {
        .sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR,
        .pNext = NULL,
        .flags = 0,
    };
    waylandcreateinfo.display = glfwGetWaylandDisplay();
    waylandcreateinfo.surface = glfwGetWaylandWindow(w);
    
    VkSurfaceKHR surface;
    if (vkCreateWaylandSurfaceKHR(instance, &waylandcreateinfo, NULL, &surface) != VK_SUCCESS) ABRT();
    */

    uint32_t physdevscount;
    if (vkEnumeratePhysicalDevices(instance, &physdevscount, NULL) != VK_SUCCESS) ABRT();
    printf("Physical devices count: %u.\n", physdevscount);
    if (!physdevscount) { puts("there's no physical devices. uh?"); ABRT(); }
    
    VkPhysicalDevice *physdevs = malloc(physdevscount * sizeof(VkPhysicalDevice));
    if (!physdevs) { puts("memory allocation failed"); ABRT(); }
    if (vkEnumeratePhysicalDevices(instance, &physdevscount, physdevs) != VK_SUCCESS) ABRT();
    for (uint32_t i = 0; i < physdevscount; i++)
    {
        VkPhysicalDevice physdev = physdevs[i];

        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(physdev, &props);

        printf("Physical device index #%u:\n -  ID: %u.\n -  Name: \"%s\".\n -  Type: ", i, props.deviceID, props.deviceName);
        switch (props.deviceType)
        {
            case VK_PHYSICAL_DEVICE_TYPE_OTHER:
                puts("other.");
                break;

            case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                puts("integrated GPU.");
                break;

            case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                puts("discrete GPU.");
                break;

            case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
                puts("virtual GPU.");
                break;

            case VK_PHYSICAL_DEVICE_TYPE_CPU:
                puts("CPU.");
                break;

            default:
                puts("(unknown).");
        }

        printf(" -  API version: %u.\n -  Driver version: %u.\n -  Vendor ID: %u.\n\n", props.apiVersion, props.driverVersion, props.vendorID);
    }

    free(physdevs);

    /*
    VkPhysicalDevice physdev;
    if (vkEnumeratePhysicalDevices(instance, 1, &physdev) != VK_SUCCESS) ABRT();

    VkDeviceCreateInfo devcrinfo =
    {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .flags = 0,
        .pNext = NULL,

        .enabledExtensionCount = 0,
        .enabledLayerCount = 0,
        .pEnabledFeatures
    }
    vkCreateDevice(physdev, )

    vkGetDeviceQueue()

    glfwGetPhysicalDevicePresentationSupport(instance, physdev, VK_QUEUE_)
    */

    /*
    VkSurfaceKHR surface;
    if (glfwCreateWindowSurface(instance, w, NULL, &surface) != VK_SUCCESS) ABRT();

    while (!glfwWindowShouldClose(w))
    {
        glfwPollEvents();
    }
    */
    
    glfwTerminate();
    return 0;
}

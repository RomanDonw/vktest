#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define VK_USE_PLATFORM_WAYLAND_KHR
#include <vk_platform.h>
#include <glad/vulkan.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "util.h"

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

    // =========================================

    VkInstanceCreateInfo instancecreateinfo = { .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
    instancecreateinfo.ppEnabledExtensionNames = glfwGetRequiredInstanceExtensions(&instancecreateinfo.enabledExtensionCount);

    VkInstance instance;
    if (vkCreateInstance(&instancecreateinfo, NULL, &instance) != VK_SUCCESS) ABRT();
    if (!gladLoadVulkanUserPtr(NULL, (void *)glfwGetInstanceProcAddress, instance)) ABRT();

    // =========================================

    VkPhysicalDevice physdev;
    {
        uint32_t physdevscount;
        if (vkEnumeratePhysicalDevices(instance, &physdevscount, NULL) != VK_SUCCESS) ABRT();
        printf("Physical devices count: %u.\n", physdevscount);
        if (!physdevscount) { puts("there's no physical devices. uh?"); ABRT(); }
        
        VkPhysicalDevice *physdevs = malloc_s(physdevscount * sizeof(VkPhysicalDevice));
        if (vkEnumeratePhysicalDevices(instance, &physdevscount, physdevs) != VK_SUCCESS) { free(physdevs); ABRT(); }

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

        printf("Select prefer physical device: ");
        uint32_t physdevidx;
        scanf("%u", &physdevidx);
        if (physdevidx >= physdevscount) { puts("Specified index out of physical devices array bounds."); free(physdevs); goto errorquit_afterselphysdev; }
        printf("Selected physical device index #%u.\n", physdevidx);

        physdev = physdevs[physdevidx];
        free(physdevs);
    }

    // =========================================

    putchar('\n');

    // =========================================

    uint32_t qfamindex;
    {
        uint32_t qfamscount;
        vkGetPhysicalDeviceQueueFamilyProperties(physdev, &qfamscount, NULL);
        printf("Query families count: %u.\n", qfamscount);
        if (!qfamscount) ABRT();

        VkQueueFamilyProperties *qfamprops = malloc_s(qfamscount * sizeof(VkQueueFamilyProperties));
        vkGetPhysicalDeviceQueueFamilyProperties(physdev, &qfamscount, qfamprops);

        int64_t idx = -1;
        for (uint32_t i = 0; i < qfamscount; i++)
        {
            VkQueueFamilyProperties qfam = qfamprops[i];
            printf("Queue family index #%u:\n -  Queue count: %u.\n -  Queue flags:\n", i, qfam.queueCount);

            if (qfam.queueFlags & VK_QUEUE_GRAPHICS_BIT) puts("     -  Graphics.");
            if (qfam.queueFlags & VK_QUEUE_COMPUTE_BIT) puts("     -  Compute.");
            if (qfam.queueFlags & VK_QUEUE_TRANSFER_BIT) puts("     -  Transfer.");
            if (qfam.queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) puts("     -  Sparse binding.");
            if (qfam.queueFlags & VK_QUEUE_PROTECTED_BIT) puts("     -  Protected.");

            if (idx < 0 && (qfam.queueFlags & VK_QUEUE_GRAPHICS_BIT) && (qfam.queueFlags & VK_QUEUE_COMPUTE_BIT)) idx = i;

            putchar('\n');
        }

        free(qfamprops);

        if (idx < 0) { puts("There's no queue family with support of graphics & computing."); goto errorquit_afterselphysdev; }
        qfamindex = (uint32_t)idx;
        printf("Selected queue family index #%u.\n", qfamindex);
    }

    // =========================================

    if (!glfwGetPhysicalDevicePresentationSupport(instance, physdev, qfamindex))
    { puts("Specified device & queue family doesn't support presentation."); goto errorquit_afterselphysdev; }

    // =========================================
    
    float qpriority = 1;
    VkDeviceQueueCreateInfo queuecrinfo =
    {
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .pNext = NULL,
        .flags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT,

        .queueCount = 1,
        .queueFamilyIndex = qfamindex,
        .pQueuePriorities = &qpriority
    };
    VkDeviceCreateInfo devcrinfo =
    {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &queuecrinfo
    };
    VkDevice dev;
    if (vkCreateDevice(physdev, &devcrinfo, NULL, &dev) != VK_SUCCESS) ABRT();

    VkQueue q;
    vkGetDeviceQueue(dev, qfamindex, 0, &q);

    // =========================================

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow *w;
    if (!(w = glfwCreateWindow(800, 600, "Vulkan Test", NULL, NULL))) ABRT();

    // =========================================

    vkDestroyDevice(dev, NULL);
    vkDestroyInstance(instance, NULL);
    glfwTerminate();
    return 0;

    errorquit_aftercrdev:
        vkDestroyDevice(dev, NULL);
    errorquit_afterselphysdev:
        vkDestroyInstance(instance, NULL);
        glfwTerminate();
    return 1;
}

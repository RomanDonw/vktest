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

    if (!gladLoadVulkanUserPtr(physdev, (void *)glfwGetInstanceProcAddress, instance)) ABRT();

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
    static const char *devexts[] = {"VK_KHR_swapchain"};
    VkDeviceCreateInfo devcrinfo =
    {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &queuecrinfo,
        .ppEnabledExtensionNames = devexts,
        .enabledExtensionCount = sizeof(devexts) / sizeof(const char *)
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

    VkSurfaceKHR surface;
    if (glfwCreateWindowSurface(instance, w, NULL, &surface) != VK_SUCCESS)
    { puts("failed creating window surface"); goto errorquit_aftercrdev; }

    VkSurfaceFormatKHR surfacefmt;
    {
        uint32_t sfmtcount;
        if (vkGetPhysicalDeviceSurfaceFormatsKHR(physdev, surface, &sfmtcount, NULL) != VK_SUCCESS)
        { puts("failed getting count of physical device surface formats"); goto errorquit_aftercrsurface; }

        if (!sfmtcount) { puts("There's no physical device surface formats"); goto errorquit_aftercrsurface; }

        printf("Physical device surface formats count: %u.\n", sfmtcount);

        VkSurfaceFormatKHR *fmts = malloc_s(sfmtcount * sizeof(VkSurfaceFormatKHR));
        if (vkGetPhysicalDeviceSurfaceFormatsKHR(physdev, surface, &sfmtcount, fmts) != VK_SUCCESS)
        { puts("failed getting physical device surface formats"); goto errorquit_aftercrsurface; }

        for (uint32_t i = 0; i < sfmtcount; i++)
        {
            VkSurfaceFormatKHR fmt = fmts[i];

            printf("Physical device surface format index #%u:\n -  Color space: ", i);

            switch (fmt.colorSpace)
            {
                case VK_COLOR_SPACE_SRGB_NONLINEAR_KHR:
                    puts("non-linear SRGB.");
                    break;

                default:
                    puts("(unknown)");
            }

            printf(" -  Format: %llu.\n\n", (unsigned long long)fmt.format);
        }

        printf("Select prefer physical device surface format: ");
        uint32_t i;
        scanf("%u", &i);
        if (i >= sfmtcount)
        { puts("Specified index out of physical device surface formats array bounds."); free(fmts); goto errorquit_aftercrsurface; }

        surfacefmt = fmts[i];
        free(fmts);
        printf("Selected physical device surface format #%u.\n", i);
    }

    // =========================================

    VkPresentModeKHR presentmode;
    {
        uint32_t pmodescount;
        if (vkGetPhysicalDeviceSurfacePresentModesKHR(physdev, surface, &pmodescount, NULL) != VK_SUCCESS)
        { puts("failed getting count of physical device surface present modes"); goto errorquit_aftercrsurface; }

        printf("Present modes count: %u.\n", pmodescount);

        VkPresentModeKHR *pmodes = malloc_s(pmodescount * sizeof(VkPresentModeKHR));
        if (vkGetPhysicalDeviceSurfacePresentModesKHR(physdev, surface, &pmodescount, pmodes) != VK_SUCCESS)
        { puts("failed getting physical device surface present modes list"); free(pmodes); goto errorquit_aftercrsurface; }

        puts("Present modes:");
        for (uint32_t i = 0; i < pmodescount; i++)
        {
            printf(" [%u]: ", i);
            switch (pmodes[i])
            {
                case VK_PRESENT_MODE_IMMEDIATE_KHR:
                    puts("Immediate.");
                    break;

                case VK_PRESENT_MODE_MAILBOX_KHR:
                    puts("Mailbox.");
                    break;

                case VK_PRESENT_MODE_FIFO_KHR:
                    puts("FIFO.");
                    break;

                case VK_PRESENT_MODE_FIFO_RELAXED_KHR:
                    puts("FIFO relaxed.");
                    break;
                
                default:
                    puts("(unknown).");
            }
        }

        printf("Select prefer present mode: ");
        uint32_t i;
        scanf("%u", &i);
        if (i >= pmodescount)
        { puts("Specified index out of physical device surface formats array bounds."); free(pmodes); goto errorquit_aftercrsurface; }

        presentmode = pmodes[i];
        free(pmodes);
        printf("Selected present mode #%u.\n", i);
    }

    // =========================================

    VkSurfaceCapabilitiesKHR surfacecapab;
    if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physdev, surface, &surfacecapab) != VK_SUCCESS)
    { puts("failed to get physical device surface capabilities"); goto errorquit_aftercrsurface; }

    #if 0

    VkSwapchainCreateInfoKHR swchaincinfo =
    {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = surface,
        .imageArrayLayers = 1,
        .minImageCount = surfacecapab.minImageCount + 1,
        .imageFormat = surfacefmt.format,
        .imageColorSpace = surfacefmt.colorSpace,
        .imageExtent = surfacecapab.currentExtent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .preTransform = surfacecapab.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = presentmode,
        .clipped = VK_TRUE,
        .oldSwapchain = NULL
    };
    
    VkSwapchainKHR swapchain;
    if (vkCreateSwapchainKHR(dev, &swchaincinfo, NULL, &swapchain) != VK_SUCCESS)
    { puts("failed creating swapchain"); goto errorquit_aftercrsurface; }

    // =========================================

    vkDestroySwapchainKHR(dev, swapchain, NULL);
    #endif
    vkDestroySurfaceKHR(instance, surface, NULL);
    vkDestroyDevice(dev, NULL);
    vkDestroyInstance(instance, NULL);
    glfwTerminate();
    return 0;

    errorquit_aftercreateswapchain:
        //vkDestroySwapchainKHR(dev, swapchain, NULL);
    errorquit_aftercrsurface:
        vkDestroySurfaceKHR(instance, surface, NULL);
    errorquit_aftercrdev:
        vkDestroyDevice(dev, NULL);
    errorquit_afterselphysdev:
        vkDestroyInstance(instance, NULL);
        glfwTerminate();
    return 1;
}

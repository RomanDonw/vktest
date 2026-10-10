#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <vk_platform.h>
#include <glad/vulkan.h>
#include <stdio.h>

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
    
    glfwTerminate();
    return 0;
}

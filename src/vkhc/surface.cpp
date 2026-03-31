// Implementation of class 'Surface'

#include <surface.h>
#include <instance.h>
#include <device.h>
#include <glfw-context.h>


namespace vkhc
{

// ---------------------------------------------------------------------------
// Constructor

Surface::Surface( Device * p_device, Instance * p_instance, GLFWContext * glfw_context ) 
{
    device = p_device ;
    instance = p_instance ;

    assert( device != nullptr );
    assert( instance != nullptr );
    assert( glfw_context != nullptr );
    
    glfwCreateWindowSurface( instance->vk_instance, glfw_context->glfw_window, nullptr, &vk_surface );
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR( device->vk_gpu, vk_surface, &vk_capabilities);
    vkGetPhysicalDeviceSurfaceFormatsKHR( device->vk_gpu, vk_surface, &formatCount, nullptr);
    formats.resize( formatCount );
    vkGetPhysicalDeviceSurfaceFormatsKHR( device->vk_gpu, vk_surface, &formatCount, formats.data());
    surfaceFormat = formats[0];
    
    std::cout << "Surface created with format " << surfaceFormat.format << std::endl ;
}
// ---------------------------------------------------------------------------

glm::uvec2 Surface::getCurrentExtent()
{
    return glm::uvec2( vk_capabilities.currentExtent.width, vk_capabilities.currentExtent.height );
}

// --------------------------------------------------------------------------
// update 'vk_capabilities.currentExtent' when the window is resized 

void Surface::updateExtent( int new_width, int new_height )
{
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR( device->vk_gpu, vk_surface, &vk_capabilities);

    VkExtent2D extent{};
    if ( vk_capabilities.currentExtent.width != UINT32_MAX) {
        extent = vk_capabilities.currentExtent;
    } else {
        extent.width = static_cast<uint32_t>(new_width);
        extent.height = static_cast<uint32_t>(new_height);
        extent.width = std::clamp(extent.width, vk_capabilities.minImageExtent.width, vk_capabilities.maxImageExtent.width);
        extent.height = std::clamp(extent.height, vk_capabilities.minImageExtent.height, vk_capabilities.maxImageExtent.height);
    }
    vk_capabilities.currentExtent = extent;
}

// ---------------------------------------------------------------------------
// adds commands to a command buffer to set the viewport and scissor to cover the whole surface

void Surface::setFullViewportAndScissor( VkCommandBuffer & vk_cmd_buffer  )
{
    VkViewport viewport{ 0,0, (float) vk_capabilities.currentExtent.width, (float) vk_capabilities.currentExtent.height, 0,1 };
    VkRect2D scissor{ {0,0}, vk_capabilities.currentExtent };

    vkCmdSetViewport( vk_cmd_buffer, 0,1,&viewport);
    vkCmdSetScissor( vk_cmd_buffer, 0,1,&scissor);
}

// ---------------------------------------------------------------------------
// sets the viewport and scissor using only a rectangle which covers the render area (not the whole surface)
// the render area is to the left of the window, its width is the window size minus the GUI width

void Surface::setRenderAreaViewportAndScissor( VkCommandBuffer & vk_cmd_buffer, uint32_t gui_width )
{
    const glm::uvec2 ss = getCurrentExtent() ;  // current window (surface) size
    
    const uint32_t render_area_width = ss.x - gui_width ;

    VkViewport viewport{ 0,0, (float) render_area_width, (float) ss.y, 0,1 };
    VkRect2D scissor{ {0,0}, {render_area_width, ss.y} };

    vkCmdSetViewport( vk_cmd_buffer, 0,1,&viewport);
    vkCmdSetScissor( vk_cmd_buffer, 0,1,&scissor);
}
// --------------------------------------------------------------------------

Surface::~Surface() 
{
    vkDestroySurfaceKHR( instance->vk_instance, vk_surface, nullptr );
    std::cout << "Deleted surface" << std::endl ;
}

// ---------------------------------------------------------------------------
} ; // end of namespace vkhc

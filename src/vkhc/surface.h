// Declaration of class 'Surface' 
// Encapsulates a 'VkSurfaceKHR' object 

#pragma once 

#include <common.h>



namespace vkhc
{

class Surface 
{
    public:
    
    VkSurfaceKHR                    vk_surface  ;
    uint32_t                        formatCount;
    VkSurfaceCapabilitiesKHR        vk_capabilities ;
    std::vector<VkSurfaceFormatKHR> formats ; 
    VkSurfaceFormatKHR              surfaceFormat  ;
    
    Device * device = nullptr ;
    Instance * instance = nullptr ;

    // constructor
    Surface( Device * p_device, Instance * p_instance, GLFWContext * glfw_context ) ;
    glm::uvec2 getCurrentExtent() ;

    // update 'vk_capabilities.currentExtent' when the window is resized 
    void updateExtent( int new_width, int new_height ) ;
    
    // adds commands to a command buffer to set the viewport and scissor to cover the whole surface
    void setFullViewportAndScissor( VkCommandBuffer & vk_cmd_buffer  ) ;

    // adds command to a command buffer to 
    // sets the viewport and scissor using only a rectangle which covers the render area (not the whole surface)
    // the render area is to the left of the window, its width is the window size minus the GUI width
    void setRenderAreaViewportAndScissor( VkCommandBuffer & vk_cmd_buffer, uint32_t gui_width ) ;

    // destructor
    ~Surface() ;
    
} ;

} // vkhc namespace end 


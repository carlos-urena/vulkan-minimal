// Declaration of 'RenderPass' class 
//
// Encapsulates a 'VkRenderPass' object and its creation, as well as 
// commands to begin and end a render pass on a command buffer

#pragma once 

#include <common.h>
#include <instance.h>
#include <device.h>
#include <glfw-context.h>
#include <surface.h>



namespace vkhc
{

class RenderPass
{
    public:
    VkRenderPass vk_render_pass = VK_NULL_HANDLE ;
    Device *     device  = nullptr ;
    Surface *    surface = nullptr ;

    // constructor 
    RenderPass( Device * p_device, Surface * p_surface );

    // add a render pass begin command to a command buffer
    void begin( VkCommandBuffer & vk_cmd_buffer,  const VkFramebuffer & vk_framebuffer, const VkClearValue & clear_color ) ;
    
    // ends a render pass on a command buffer
    void end( VkCommandBuffer & vk_cmd_buffer ) ;

    // destroys the render pass 
    ~RenderPass() ;
    
} ;

} // vkhc namespace end 


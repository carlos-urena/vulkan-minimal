// Declaration of the class 'VulkanContext' 
//
// encapsulate all the vulkan state objects, such as: 
// - the GLFW context and window
// - the Vulkan instance
// - the Vulkan device and queue
// - the Vulkan surface
// - the Vulkan render pass
// - the Vulkan swap chain
// - the command pool and command buffers
// - the synchronization objects (semaphores and fences)
// - the IMGUI state (IMGUI context, IMGUI render pass, etc.)

#pragma once 

#include <common.h>
#include <instance.h>
#include <device.h>
#include <glfw-context.h>
#include <surface.h>
#include <render-pass.h>


// ***********************************************************************************
// vulkan helper classes, mainly related to vulkan but also GLFW and IMGUI

namespace vkhc
{

// -------------------------------------------------------------------------------

class VulkanContext
{
    public:

    GLFWContext *           glfw_context = nullptr ;
    Instance *              instance     = nullptr ;
    Surface *               surface      = nullptr ;
    Device *                device       = nullptr ;
    RenderPass *            render_pass  = nullptr ;
    SwapChain *             swap_chain   = nullptr ;
    CommandPoolAndBuffers * cmd_buffers  = nullptr ;
    SyncObjects *           sync_objects = nullptr ;
    IMGUIContext *            imgui_state  = nullptr ;

    // vulkan state singleton
    static VulkanContext* ctx_instance ;

    // true right after a buffer resize event is raised, but before the SwapChain is re-configured.
    bool framebufferResized = false ;  

    VulkanContext( int nx, int ny, const char * title );
    ~VulkanContext() ;
    void resizeWindow() ;
    void framebufferResizedCallback( GLFWwindow* window, int nx, int ny );

    // called before/after all drawing commands, each frame
    bool beginFrame( const VkClearValue & clear_color, VkCommandBuffer & vk_cmd, uint32_t & imageIndex ) ;
    void endFrame( VkCommandBuffer vk_cmd, uint32_t imageIndex ) ;

    // returns true when the user has clicked the close button of the window 
    bool windowShouldClose() ;

    // wait for the device to be idle (used at the end of the program, and when the window is resized)
    void waitDeviceIdle() ;

    // process pending events, without blocking
    void pollEvents() ;

    // adds a command to set the viewport to cover the whole GLFW window
    void setFullViewport( VkCommandBuffer vk_cmd ) ;

    // adds a command to set the viewport to cover just the render area (left to GUI)
    void setRenderAreaViewport( VkCommandBuffer vk_cmd ) ;

    // returns the extent of the current render area (left to GUI)
    glm::uvec2 getRenderAreaExtent() ;

    // begins rendering IMGUI widgets in a frame 
    void beginIMGUIFrame( VkCommandBuffer & vk_cmd) ;

    // ends rendering IMGUI widgets in a frame 
    void endIMGUIFrame( VkCommandBuffer & vk_cmd ) ;


} ;  // fin de la clase VulkanContext



} // fin del namespace vkhc 


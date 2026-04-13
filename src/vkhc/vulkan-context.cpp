// Implementation of the class 'VulkanContext' 
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

#include <common.h>
#include <vulkan-context.h>
#include <device.h>
#include <glfw-context.h>
#include <surface.h>
#include <render-pass.h>
#include <swap-chain.h>
#include <imgui-context.h>
#include <command-pool-buffers.h>




namespace vkhc
{

// definition of the pointer to the context singleton 
VulkanContext* VulkanContext::ctx_instance = nullptr ;

// -------------------------------------------------------------------------------------
// This is called each time the user clicks resizes the window  

void framebufferResizeCallback( GLFWwindow* window, int nx, int ny ) 
{
    std::cout << "Framebuffer resize callback called. New size == " << nx << " x " << ny << "." << std::endl ;
    if ( VulkanContext::ctx_instance != nullptr )
        VulkanContext::ctx_instance->framebufferResizedCallback( window, nx, ny );
    else 
        throw std::runtime_error("Warning: framebuffer resize callback called but VulkanState instance is null !!");
}




// -------------------------------------------------------------------------------
// Implementación de la clase VulkanState

VulkanContext::VulkanContext( int nx, int ny, const char * title )
{            
    Assert( ctx_instance == nullptr, " VulkanContext::VulkanContext:'ctx_instance' instance already exists !!");

    Assert( nx > 0, "VulkanContext constructor: 'nx' must be > 0" );
    Assert( ny > 0, "VulkanContext constructor: 'ny' must be > 0" );

    // initialize all state objects 
    glfw_context = new GLFWContext{ nx, ny, title }; assert( glfw_context != nullptr );  
    instance     = new Instance{};                                  assert( instance != nullptr );
    device       = new Device{ instance };                          assert( device != nullptr );
    sync_objects = new SyncObjects{ device };                       assert( sync_objects != nullptr );
    surface      = new Surface{ device, instance, glfw_context };     assert( surface != nullptr );
    render_pass  = new RenderPass{ device, surface };               assert( render_pass != nullptr );
    swap_chain   = new SwapChain{ device, surface, render_pass };   assert( swap_chain != nullptr );
    cmd_buffers  = new CommandPoolAndBuffers{ device, swap_chain }; assert( cmd_buffers != nullptr );
    imgui_state  = new IMGUIContext{ instance, device, surface, render_pass, swap_chain, glfw_context };                  
    assert( imgui_state != nullptr );

    // register this object as the context singleton
    ctx_instance = this ;
}
// --------------------------------------------------------------------------------

VulkanContext::~VulkanContext()
{
    // destroy object (in reverse creation order).

    delete imgui_state ;      imgui_state  = nullptr ; // (must be destroyed when the device still exists))
    delete cmd_buffers ;      cmd_buffers  = nullptr ;
    delete swap_chain ;       swap_chain   = nullptr ;  
    delete render_pass ;      render_pass  = nullptr ; 
    delete surface ;          surface      = nullptr ; // destruir antes de la instancia
    delete sync_objects ;     sync_objects = nullptr ;
    delete device ;           device       = nullptr ;
    delete instance ;         instance     = nullptr ;
    delete glfw_context ;     glfw_context = nullptr ;

    ctx_instance = nullptr ; // reset the context singleton pointer to null
}

// --------------------------------------------------------------------------------
// function to recreate the surface, swap chain, and command buffers when 
// the size of the window changes 

void VulkanContext::resizeWindow() 
{
    assert( glfw_context != nullptr ); 
    assert( device != nullptr );
    assert( surface != nullptr );
    assert( render_pass != nullptr );
    assert( swap_chain != nullptr );

    // get new GLFW framebuffer (window) size.
    int width = 0;
    int height = 0;
    glfw_context->getCurrentWindowSize( width, height );  // waits for the size to be > 0 and returns width and height

    std::cout << "Resizing window ..." << std::endl;

    vkDeviceWaitIdle( device->vk_device );
    surface->updateExtent( width, height );  // updates surface->vk_capabilities.currentExtent based on the new window size
    swap_chain->recreate(  ); // recreates the swap chain and its image views and framebuffers based on the new surface capabilities (extent)
    cmd_buffers->recreate( swap_chain ); // recreates the command buffers based on the new number of images in the swap chain
    imgui_state->windowResized( swap_chain );
    
    std::cout << "Window resize processed ok." << std::endl;
}
// --------------------------------------------------------------------------------

void VulkanContext::framebufferResizedCallback( GLFWwindow* window, int nx, int ny ) 
{
    std::cout << "Framebuffer resize callback called. New size == " << nx << " x " << ny << "." << std::endl ;
    framebufferResized = true;
}
// --------------------------------------------------------------------------------
    
// begins a frame, returns true if the image has been acquired, or false if the window 
// has been resized. When it returns true, writes 'vk_cmd'  (command buffer to use) 
// and 'imageIndex' (index in the swapchain of the acquired image to render to).

bool VulkanContext::beginFrame( const VkClearValue & clear_color, VkCommandBuffer & vk_cmd, uint32_t & imageIndex ) 
{
    if ( framebufferResized ) 
    {
        framebufferResized = false;
        std::cout << "Framebuffer resized, recreating state." << std::endl;
        resizeWindow();
        return false  ;
    }

    // try to acquire an image, but if the swap-chain has been resized, start again the loop 
    // with the updated swap-chain
    // writes 'imageIndex' 
    
    if ( ! swap_chain->acquireNextImage( sync_objects, &imageIndex ) ) 
        return false ;  
    
    vk_cmd = cmd_buffers->resetAndGetCommandBuffer( imageIndex ) ; 
    
    VkCommandBufferBeginInfo cbbi{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    vkBeginCommandBuffer( vk_cmd, &cbbi );

    VkFramebuffer vk_framebuffer = swap_chain->getFrameBuffer( imageIndex ) ; // return framebuffer for this imageIndex
    render_pass->begin( vk_cmd, vk_framebuffer, clear_color ); 

    // sets viewport and scissor to cover the entire surface 
    surface->setFullViewportAndScissor( vk_cmd ); 

    return true  ;

}

// --------------------------------------------------------------------------------

void VulkanContext::endFrame( VkCommandBuffer vk_cmd, uint32_t imageIndex ) 
{
    render_pass->end( vk_cmd );
    vkEndCommandBuffer( vk_cmd );
    
    device->submitCommandBuffer( vk_cmd, sync_objects ) ;
    swap_chain->presentDeviceQueue( &imageIndex, sync_objects, framebufferResized  ) ;

}
// --------------------------------------------------------------------------------

bool VulkanContext::windowShouldClose()
{
    assert( glfw_context != nullptr );
    return glfwWindowShouldClose( glfw_context->glfw_window );
}
// --------------------------------------------------------------------------------

void VulkanContext::waitDeviceIdle() 
{
    assert( device != nullptr );
    vkDeviceWaitIdle( device->vk_device );
}
// --------------------------------------------------------------------------------

void VulkanContext::pollEvents() 
{
    glfwPollEvents();
}

// -------------------------------------------------------------------------------
// adds a command to set the viewport to cover the whole GLFW window

void VulkanContext::setFullViewport( VkCommandBuffer vk_cmd )
{
    assert( surface != nullptr );
    surface->setFullViewportAndScissor( vk_cmd );
}

// -----------------------------------------------------------------------------
// adds a command to set the viewport to cover just the render area (left to GUI)

void VulkanContext::setRenderAreaViewport( VkCommandBuffer vk_cmd )
{
    assert( imgui_state != nullptr );
    assert( surface != nullptr );
    const uint32_t gui_wx = imgui_state->getGUIWidth() ;
    surface->setRenderAreaViewportAndScissor( vk_cmd, gui_wx );
}

// ------------------------------------------------------------------------------

glm::uvec2 VulkanContext::getRenderAreaExtent() 
{
    assert( imgui_state != nullptr );
    assert( surface != nullptr );
    const uint32_t   gui_wx  = imgui_state->getGUIWidth() ;
    const glm::uvec2 win_ext = surface->getCurrentExtent();

    return glm::uvec2( win_ext.x - gui_wx , win_ext.y );
}

// -------------------------------------------------------------------------------

// begins rendering IMGUI widgets in a frame 
void VulkanContext::beginIMGUIFrame( VkCommandBuffer & vk_cmd ) 
{
    assert( imgui_state != nullptr );
    setFullViewport( vk_cmd ) ;
    imgui_state->beginFrame();
}
// -------------------------------------------------------------------------------
// ends rendering IMGUI widgets in a frame 

void VulkanContext::endIMGUIFrame( VkCommandBuffer & vk_cmd ) 
{
    assert( imgui_state != nullptr );
    imgui_state->endFrame( vk_cmd );
}

} // vkhc namespace end 

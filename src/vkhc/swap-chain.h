// Declaration of class 'SwapChain' 
// Encapsulates swap chain (VkSwapchainKHR), and 
// associated images, images views, and framebuffers

#pragma once 

#include <common.h>

namespace vkhc
{

// -------------------------------------------------------------------------------


class SwapChain
{
    public:
    
    VkSwapchainKHR vk_swap_chain;
    uint32_t imageCount;

    std::vector<VkImage>         swapchainImages ;
    std::vector<VkImageView>     vk_image_views ;
    std::vector<VkFramebuffer>   framebuffers ;
    int min_image_count = 0 ;

    Device * device = nullptr ;
    Surface * surface = nullptr ;
    RenderPass * render_pass = nullptr ;

    // Methods:

    void createImageView( VkImage & vk_image, VkImageView & vk_image_view ) ;
    
    // initializes vk_image_views from the vk_images
    void createImageViews() ;
    void createFramebuffer( VkImageView & vk_image_view, VkFramebuffer & vk_framebuffer ) ;
    void createFramebuffers() ;
    void createImages() ;
    void createSwapChain() ;
    void initialize() ;
    
    // constructor 
    SwapChain( Device * p_device, Surface * p_surface, RenderPass * p_render_pass ) ;
    
    // recreates the swap chain when the window is resized 
    void recreate(  ) ;
    
    // gets one of the framebuffers 
    VkFramebuffer getFrameBuffer( const uint32_t imageIndex ) ;
    
    // present the device queue to this swap chain (to be called after submitting at least a command buffer)
    void presentDeviceQueue(  uint32_t * imageIndex_ptr, SyncObjects * sync_objects, bool framebufferResized  );
    
    // ACQUIRE 'imageIndex' each frame, return false if an image cannot been acquired because the 
    // swap chain is out of date, in that case the swap chain is recreated, otherwise it returns true. 
    bool acquireNextImage( SyncObjects * sync_objects, uint32_t * imageIndex_ptr );
    
    // termination
    void destroy() ;
     ~SwapChain() ;
    
} ;


} // fin del namespace vkhc 


// Declaration of class 'SwapChain' 
//
// Encapsulates swap chain (VkSwapchainKHR), and 
// associated images (VkImage), images views (VkImageView), and framebuffers (VkFramebuffer)


#include <swap-chain.h>
#include <device.h>
#include <surface.h>
#include <render-pass.h>
#include <vulkan-context.h>

namespace vkhc
{

void SwapChain::createImageView( VkImage & vk_image, VkImageView & vk_image_view )
{
    assert( surface != nullptr ); 

    VkImageViewCreateInfo ivci{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
    ivci.image = vk_image ;
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ivci.format = surface->surfaceFormat.format;
    ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    ivci.subresourceRange.levelCount = 1;
    ivci.subresourceRange.layerCount = 1;

    if ( vkCreateImageView( device->vk_device, &ivci, nullptr, &vk_image_view ) != VK_SUCCESS)
        throw std::runtime_error("Failed to recreate image view");
}
// -----------------------------------------------------------------------------
// initializes vk_image_views from the vk_images

void SwapChain::createImageViews()
{
    assert( imageCount > 0 );
    vk_image_views.resize( imageCount );
    for ( uint32_t i = 0 ; i < imageCount; i++) {
        createImageView( swapchainImages[i], vk_image_views[i] );
    }
}

// -----------------------------------------------------------------------------

void SwapChain::createFramebuffer( VkImageView & vk_image_view, VkFramebuffer & vk_framebuffer )
{
    assert( surface != nullptr );
    assert( render_pass != nullptr );

    VkFramebufferCreateInfo fbci{ VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
    fbci.renderPass = render_pass->vk_render_pass ;
    fbci.attachmentCount = 1;
    fbci.pAttachments = &vk_image_view;
    fbci.width = surface->vk_capabilities.currentExtent.width;
    fbci.height = surface->vk_capabilities.currentExtent.height;
    fbci.layers = 1;

    if ( vkCreateFramebuffer(device->vk_device, &fbci, nullptr, &vk_framebuffer ) != VK_SUCCESS )
        throw std::runtime_error("Failed to create framebuffer");
}
// -----------------------------------------------------------------------------

void SwapChain::createFramebuffers()
{
    assert( imageCount > 0 );
    framebuffers.resize( imageCount );

    for ( uint32_t i = 0; i < imageCount; i++) {
        createFramebuffer( vk_image_views[i], framebuffers[i] );
    }
}
// -----------------------------------------------------------------------------

void SwapChain::createImages()
{
    assert( device != nullptr );

    vkGetSwapchainImagesKHR( device->vk_device, vk_swap_chain, &imageCount, nullptr);
    assert( imageCount > 0 );

    swapchainImages.resize( imageCount );
    vkGetSwapchainImagesKHR( device->vk_device, vk_swap_chain, &imageCount, 
                                swapchainImages.data() );
}
// -----------------------------------------------------------------------------

void SwapChain::createSwapChain()
{
    VkSwapchainCreateInfoKHR sci{ VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };

    sci.surface = surface->vk_surface ;
    {
        uint32_t desiredImageCount = surface->vk_capabilities.minImageCount + 1;
        std::cout << "Creating swapchain. Image count min: " << surface->vk_capabilities.minImageCount << ", max: " << surface->vk_capabilities.maxImageCount << "." << std::endl ;
        if (surface->vk_capabilities.maxImageCount > 0 && desiredImageCount > surface->vk_capabilities.maxImageCount)
            desiredImageCount = surface->vk_capabilities.maxImageCount;
        sci.minImageCount = desiredImageCount;
    }
    min_image_count = sci.minImageCount ; // register min image count

    sci.imageFormat = surface->surfaceFormat.format;
    sci.imageColorSpace = surface->surfaceFormat.colorSpace;
    sci.imageExtent = surface->vk_capabilities.currentExtent;
    sci.imageArrayLayers = 1;
    sci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    sci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    sci.preTransform = surface->vk_capabilities.currentTransform;
    sci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    sci.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    sci.clipped = VK_TRUE;

    if ( vkCreateSwapchainKHR(device->vk_device, &sci, nullptr, &vk_swap_chain) != VK_SUCCESS ) // crea la swap chain nueva
        throw std::runtime_error("Failed to recreate swapchain");
}
// -----------------------------------------------------------------------------

void SwapChain::initialize()
{
    createSwapChain();
    createImages();
    createImageViews();
    createFramebuffers();
}
// -----------------------------------------------------------------------------
// constructor 

SwapChain::SwapChain( Device * p_device, Surface * p_surface, RenderPass * p_render_pass )
{
    device = p_device ;            assert( device != nullptr );
    surface = p_surface ;          assert( surface != nullptr );
    render_pass = p_render_pass ;  assert( render_pass != nullptr );

    initialize();
    std::cout << "Swapchain created with " << imageCount << " images" << std::endl;
}
// -----------------------------------------------------------------------------

    void SwapChain::destroy()
{
    // destroy framebuffers and image views in the swap chain ...
    for (auto framebuffer : framebuffers)
        vkDestroyFramebuffer( device->vk_device, framebuffer, nullptr);
    framebuffers.clear();

    for ( auto imageView : vk_image_views )
        vkDestroyImageView( device->vk_device, imageView, nullptr);
    vk_image_views.clear();

    vkDestroySwapchainKHR( device->vk_device, vk_swap_chain, nullptr);  /// elimina la swap chain antigua (hacerlo antes?)
}
// --------------------------------------------------------------------------------
// recreates the swap chain when the window is resized 

void SwapChain::recreate(  ) 
{
    assert( device != nullptr );
    assert( surface != nullptr );
    assert( render_pass != nullptr );

    destroy();
    initialize();
}

// --------------------------------------------------------------------------------
// gets one of the framebuffers 

VkFramebuffer SwapChain::getFrameBuffer( const uint32_t imageIndex ) 
{
    assert( imageIndex < framebuffers.size() );
    return framebuffers[ imageIndex ];
}
// --------------------------------------------------------------------------------
// present the device queue to this swap chain (to be called after submitting at least a command buffer)

void SwapChain::presentDeviceQueue(  uint32_t * imageIndex_ptr, SyncObjects * sync_objects, bool framebufferResized  )
{
    assert( device != nullptr );
    assert( imageIndex_ptr != nullptr );
    assert( sync_objects != nullptr );
    

    VkPresentInfoKHR present{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
    present.waitSemaphoreCount = 1;
    present.pWaitSemaphores = &(sync_objects->renderFinishedSemaphore);
    present.swapchainCount = 1;
    present.pSwapchains = & vk_swap_chain ;
    present.pImageIndices = imageIndex_ptr ;

    VkResult presentResult = vkQueuePresentKHR( device->vk_queue, &present);

    if ( presentResult == VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR || framebufferResized ) 
    {
        framebufferResized = false;
        assert( VulkanContext::ctx_instance != nullptr );
        VulkanContext::ctx_instance->resizeWindow();
    } 
    else if ( presentResult != VK_SUCCESS ) 
    {
        throw std::runtime_error("Failed to present swapchain image !!");
    }
}


// -------------------------------------------------------------------------

// ACQUIRE 'imageIndex' each frame, return false if an image cannot been acquired because the 
// swap chain is out of date, in that case the swap chain is recreated, otherwise it returns true. 

bool SwapChain::acquireNextImage( SyncObjects * sync_objects, uint32_t * imageIndex_ptr )
{
    assert( device != nullptr );
    assert( imageIndex_ptr != nullptr );
    assert( sync_objects != nullptr );

    uint32_t imageIndex;

    vkWaitForFences( device->vk_device, 1, &(sync_objects->inFlightFence), VK_TRUE, UINT64_MAX);
    vkResetFences( device->vk_device, 1, &(sync_objects->inFlightFence));

    VkResult acquireResult = vkAcquireNextImageKHR(
        device->vk_device,
        vk_swap_chain,
        UINT64_MAX,
        sync_objects->imageAvailableSemaphore,
        VK_NULL_HANDLE,
        &imageIndex
    );

    if ( acquireResult == VK_ERROR_OUT_OF_DATE_KHR ) 
    {
        std::cout << "Resizing window in order to re-creating the swapchain because it is out of date" << std::endl;
        assert( VulkanContext::ctx_instance != nullptr );
        VulkanContext::ctx_instance->resizeWindow();
        return false ;
    }
    if ( acquireResult != VK_SUCCESS && acquireResult != VK_SUBOPTIMAL_KHR )
        throw std::runtime_error("Failed to acquire next swapchain image");

    *imageIndex_ptr = imageIndex ;
    return true ;
}

// --------------------------------------------------------------------------------

SwapChain::~SwapChain() 
{
    destroy();
    std::cout << "Deleted swap chain" << std::endl ;
}

} // fin del namespace vkhc 


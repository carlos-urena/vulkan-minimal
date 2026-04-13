// Implementation of 'RenderPass' class 
//
// Encapsulates a 'VkRenderPass' object and its creation, as well as 
// commands to begin and end a render pass on a command buffer

#include <render-pass.h>

namespace vkhc
{


RenderPass::RenderPass( Device * p_device, Surface * p_surface )
{
    device  = p_device ;  Assert( device != nullptr, "RenderPass::RenderPass 'device' is null" );
    surface = p_surface ; Assert( surface != nullptr, "RenderPass::RenderPass 'surface' is null" );
    
    VkAttachmentDescription color_attachment 
    {   .format        = surface->surfaceFormat.format,
        .samples       = VK_SAMPLE_COUNT_1_BIT,
        .loadOp        = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp       = VK_ATTACHMENT_STORE_OP_STORE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout   = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    } ;
    VkAttachmentReference color_ref 
    {   .attachment   = 0, 
        .layout       = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    } ;
    VkSubpassDescription sub_pass
    {   .pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = 1,
        .pColorAttachments    = &color_ref
    } ;
    VkRenderPassCreateInfo rpci
    {   .sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments    = &color_attachment,
        .subpassCount    = 1,
        .pSubpasses      = &sub_pass
    } ;
    
    VkResult result = vkCreateRenderPass( device->vk_device, &rpci, nullptr, &vk_render_pass );
    Assert( result == VK_SUCCESS, "vkCreateRenderPass failed" );
}

// -----------------------------------------------------------------------------
// add a 'render pass begin' command to a command buffer

void RenderPass::begin( VkCommandBuffer & vk_cmd_buffer,  const VkFramebuffer & vk_framebuffer, const VkClearValue & clear_color ) 
{
    Assert( surface != nullptr, "RenderPass::begin 'surface' is null" );

    VkRect2D render_area
    {   .offset = { 0, 0 },
        .extent = surface->vk_capabilities.currentExtent
    } ;
    VkRenderPassBeginInfo rpbi
    {   .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass        = vk_render_pass ,
        .framebuffer       = vk_framebuffer ,
        .renderArea        = render_area,
        .clearValueCount   = 1,
        .pClearValues      = &clear_color
    } ;
    vkCmdBeginRenderPass( vk_cmd_buffer, &rpbi, VK_SUBPASS_CONTENTS_INLINE);
}

// -----------------------------------------------------------------------------
// adds a 'render pass ends' command to a command buffer

void RenderPass::end( VkCommandBuffer & vk_cmd_buffer )
{
    vkCmdEndRenderPass( vk_cmd_buffer );
}

// -----------------------------------------------------------------------------
// destroys the render pass

RenderPass::~RenderPass()
{
    if (vk_render_pass != VK_NULL_HANDLE) {
        vkDestroyRenderPass( device->vk_device, vk_render_pass, nullptr);
        vk_render_pass = VK_NULL_HANDLE;
    }

    std::cout << "Deleted render pass." << std::endl ;
}

} // vkhc namespace end 


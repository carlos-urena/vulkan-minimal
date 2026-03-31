// Implementation of 'RenderPass' class 
//
// Encapsulates a 'VkRenderPass' object and its creation, as well as 
// commands to begin and end a render pass on a command buffer

#include <render-pass.h>

namespace vkhc
{


RenderPass::RenderPass( Device * p_device, Surface * p_surface )
{
    device = p_device ;   assert( device != nullptr );
    surface = p_surface ; assert( surface != nullptr );
    
    colorAttachment.format = surface->surfaceFormat.format;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    
    colorRef.attachment = 0;
    colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorRef;
    
    rpci.attachmentCount = 1;
    rpci.pAttachments = &colorAttachment;
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;

    vkCreateRenderPass( device->vk_device, &rpci, nullptr, &vk_render_pass );
}

// -----------------------------------------------------------------------------
// add a render pass begin command to a command buffer

void RenderPass::begin( VkCommandBuffer & vk_cmd_buffer,  const VkFramebuffer & vk_framebuffer, const VkClearValue & clear_color ) 
{
    assert( surface != nullptr );

    VkRenderPassBeginInfo rpbi{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
    
    rpbi.renderPass        = vk_render_pass ;
    rpbi.framebuffer       = vk_framebuffer ;
    rpbi.renderArea.extent = surface->vk_capabilities.currentExtent;
    rpbi.clearValueCount   = 1;
    rpbi.pClearValues      = &clear_color;

    vkCmdBeginRenderPass( vk_cmd_buffer, &rpbi, VK_SUBPASS_CONTENTS_INLINE);
}

// -----------------------------------------------------------------------------
// ends a render pass on a command buffer

void RenderPass::end( VkCommandBuffer & vk_cmd_buffer )
{
    vkCmdEndRenderPass( vk_cmd_buffer );
}

    
} // fin del namespace vkhc 


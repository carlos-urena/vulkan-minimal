// Implementation of class Texture 
//
// 

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <common.h>
#include <vulkan-context.h>
#include <device.h>
#include <pipeline.h>
#include <command-pool-buffers.h>
#include <textures.h>

namespace vkhc
{
// ------------------------------------------------------------------------------

TexturesSet::TexturesSet( VulkanContext * p_context ) 
{
    assert( p_context != nullptr );
    assert( p_context->device != nullptr );
    assert( p_context->device->vk_device != VK_NULL_HANDLE );
    context = p_context ;


    // create sampler for all the textures in the set. We use the same sampler 
    // for all textures, but we could use different samplers for different textures 
    // if we wanted to.

    VkSamplerCreateInfo sampler_ci{ VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
    sampler_ci.magFilter = VK_FILTER_LINEAR;
    sampler_ci.minFilter = VK_FILTER_LINEAR;
    sampler_ci.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_ci.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_ci.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_ci.anisotropyEnable = VK_FALSE;
    sampler_ci.maxAnisotropy = 1.0f;
    sampler_ci.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    sampler_ci.unnormalizedCoordinates = VK_FALSE;
    sampler_ci.compareEnable = VK_FALSE;
    sampler_ci.compareOp = VK_COMPARE_OP_ALWAYS;
    sampler_ci.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler_ci.mipLodBias = 0.0f;
    sampler_ci.minLod = 0.0f;
    sampler_ci.maxLod = 0.0f;

    if ( vkCreateSampler( context->device->vk_device, &sampler_ci, nullptr, &vk_sampler ) != VK_SUCCESS ) {
        throw std::runtime_error( "Failed to create texture sampler" );
    }
} ;
// --------------------------------------------------------------------------------

uint32_t TexturesSet::add( Texture * texture ) 
{
    assert( texture != nullptr );
    if ( textures.size() >= max_textures ) {
        std::cerr << "Exceeded maximum number of textures allowed (which is " << max_textures <<  ")" << std::endl ;
        throw std::runtime_error( "max number of textures exceeded" );
    }
    textures.push_back( texture ) ;
    return static_cast<uint32_t>( textures.size() - 1 ) ; // return index of the added texture
}
// --------------------------------------------------------------------------------

uint32_t TexturesSet::add( const std::string & file_path )
{
    TextureFromFile * texture = new TextureFromFile( context, file_path ) ;
    return add( texture ) ;
}

// --------------------------------------------------------------------------------

void TexturesSet::bindTo( BasicPipeline & pipeline ) 
{
    // we bind the textures set to the pipeline by binding a descriptor set with an array of texture samplers, one for each texture in the set. 
    // the 'texture_index' push constant is used to select the active texture in the fragment shader, by indexing into this array of samplers. 

    assert( context != nullptr );
    assert( context->device != nullptr );

    Device * device = context->device ;
    assert( device != nullptr );

    assert( vk_sampler != VK_NULL_HANDLE );

    std::vector<VkDescriptorImageInfo> image_infos ;
    for ( Texture * texture : textures ) 
    {
        VkDescriptorImageInfo image_info{};
        image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        image_info.imageView = texture->vk_image_view ;
        image_info.sampler = vk_sampler ; // we use the same sampler for all textures, but we could use different samplers for different textures if we wanted to.
        image_infos.push_back( image_info );
    }

    VkWriteDescriptorSet descriptor_write{};
    descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_write.dstSet = pipeline.vk_textures_descriptor_set;
    descriptor_write.dstBinding = 0;
    descriptor_write.dstArrayElement = 0;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor_write.descriptorCount = static_cast<uint32_t>( image_infos.size() );
    descriptor_write.pImageInfo = image_infos.data();

    vkUpdateDescriptorSets( device->vk_device, 1, &descriptor_write, 0, nullptr );
}

// --------------------------------------------------------------------------------
// Aux functions for image layout transition

void  transitionImageLayout( VulkanContext * context, VkImage vk_image, VkImageLayout old_layout, VkImageLayout new_layout )
{
    assert( context != nullptr );
    assert( context->cmd_buffers != nullptr );

    VkCommandBuffer vk_cmd = context->cmd_buffers->beginSingleTimeCommands();

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = old_layout;
    barrier.newLayout = new_layout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = vk_image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    VkPipelineStageFlags dst_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;

    if ( old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL ) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dst_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if ( old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL ) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        src_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dst_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else {
        throw std::runtime_error( "Unsupported image layout transition!" );
    }

    vkCmdPipelineBarrier( vk_cmd, src_stage, dst_stage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

    context->cmd_buffers->endSingleTimeCommands( vk_cmd );
};

// -----------------------------------------------------------------------------

void Texture::copyBufferToImage( VkBuffer buffer, VkImage image, uint32_t width, uint32_t height ) 
{
    assert( context != nullptr );
    assert( context->cmd_buffers != nullptr );

    CommandPoolAndBuffers * cpb = context->cmd_buffers ;

    VkCommandBuffer vk_cmd = cpb->beginSingleTimeCommands();

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {width, height, 1};

    vkCmdCopyBufferToImage( vk_cmd, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    
    cpb->endSingleTimeCommands( vk_cmd );
}

// --------------------------------------------------------------------------------

void Texture::initialize() 
{
    //std::cout << "Texture::initialize() begins" << std::endl ;

    assert( width > 0 );
    assert( height > 0 );
      
    assert( src_data != nullptr );

    assert( context != nullptr );  
    Device * device = context->device ;
    assert( device != nullptr );

    VkDevice vk_device = context->device->vk_device ;
    assert( vk_device != nullptr );
    assert( total_size_bytes > 0 );

    // 1. Create a staging buffer and a staging host-visible memory and upload source pixels 
    // from the buffer to the memory (creates: vk_staging_buffer, vk_staging_memory)

    vk_total_size_bytes = static_cast<VkDeviceSize>( total_size_bytes ) ;
    VkBuffer vk_staging_buffer = VK_NULL_HANDLE ;
    VkDeviceMemory vk_staging_memory = VK_NULL_HANDLE ;
    VkBufferUsageFlagBits usage_flags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT ;

    // create buffer and host-visible and host-coherent memory
    device->createBufferAndCopyData( vk_total_size_bytes, src_data, usage_flags, 
                                     vk_staging_buffer, vk_staging_memory );

    // 2. Create the Image Object (vk_image)
    
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = image_format;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

    VkResult result = vkCreateImage( vk_device, &imageInfo, nullptr, &vk_image );
    if ( result != VK_SUCCESS) {
        throw std::runtime_error("Failed to create image!");
    }

    // 3. Allocate and bind image memory (vk_memory)
    VkMemoryRequirements image_mem_req{} ;
    vkGetImageMemoryRequirements( vk_device, vk_image, &image_mem_req );

    VkMemoryAllocateInfo image_mai{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
    image_mai.allocationSize = image_mem_req.size;
    image_mai.memoryTypeIndex = device->findMemoryTypeIndex( image_mem_req, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );

    if ( vkAllocateMemory( vk_device, &image_mai, nullptr, &vk_memory ) != VK_SUCCESS ) {
        throw std::runtime_error( "Failed to allocate memory for texture image!" );
    }

    if ( vkBindImageMemory( vk_device, vk_image, vk_memory, 0 ) != VK_SUCCESS ) {
        throw std::runtime_error( "Failed to bind texture image memory!" );
    }

    // 4. Copy data from the staging buffer to the image (vk_staging_buffer -> vk_image)

    transitionImageLayout( context, vk_image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL );
    copyBufferToImage( vk_staging_buffer, vk_image, width, height );
    transitionImageLayout( context, vk_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL );

    vkDestroyBuffer( vk_device, vk_staging_buffer, nullptr );
    vkFreeMemory( vk_device, vk_staging_memory, nullptr );

    // 5. Create the image view object (vk_image_view)

    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = vk_image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D; // Matches sampler2D
    viewInfo.format = image_format;    // Must match the VkImage format

    // Component mapping allows you to swizzle channels (e.g., swap Red and Blue)
    // VK_COMPONENT_SWIZZLE_IDENTITY means "keep it as is"
    viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView( vk_device, &viewInfo, nullptr, &vk_image_view) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture image view!");
    }
} 

// --------------------------------------------------------------------------------
// sample constructor, provides an example source data for the texture 

Texture::Texture(  VulkanContext * p_context ) 
{
    // testing an example texture parameters, WIP 
    assert( p_context != nullptr );
    context = p_context ;
}

// --------------------------------------------------------------------------------

Texture::~Texture() 
{
    assert( context != nullptr );
    assert( context->device != nullptr );
    assert( context->device->vk_device != VK_NULL_HANDLE );

    VkDevice vk_device = context->device->vk_device ;

    vkDestroyImageView( vk_device, vk_image_view, nullptr );
    vkDestroyImage( vk_device, vk_image, nullptr );
    vkFreeMemory( vk_device, vk_memory, nullptr );
    if ( src_data != nullptr )
    {
        delete[] src_data ;
        src_data = nullptr ;
    }
} 

// --------------------------------------------------------------------------------

ProceduralTexture1::ProceduralTexture1( VulkanContext * p_context ) 

:   Texture( p_context )
{
    width = 513 ;   // non power of two, just for testing, see: https://stackoverflow.com/questions/36028497/loading-non-power-of-two-textures-in-vulkan
    height = 517 ;
    
    // create a sample source data:

    assert( image_format == VK_FORMAT_R8G8B8A8_SRGB ); 
    total_size_bytes = width * height * 4 ; // assuming 4 bytes per pixel (RGBA) (see previous assert)
    src_data = new unsigned char[ total_size_bytes ] ; assert( src_data != nullptr );

    constexpr int sz = 10 ;

    for( uint32_t i = 0; i < width * height; ++i ) 
    {
        uint32_t x = i % width ;
        uint32_t y = i / width ;  
        float r,g,b ;

        unsigned ix = x /sz ;
        unsigned iy = y /sz ;
        
        r = float(ix % 5)/ 4.0 ;
        g = float(iy % 5)/ 4.0 ;
        b = float((ix + iy) % 5)/ 4.0 ;

        src_data[4*i + 0] = static_cast<unsigned char> (255.0*float(r)) ;
        src_data[4*i + 1] = static_cast<unsigned char> (255.0*float(g)) ;
        src_data[4*i + 2] = static_cast<unsigned char> (255.0*float(b)) ;
        src_data[4*i + 3] = 255 ;
    }
    
    // do all vulkan staff
    initialize() ;

    std::cout << "Example procedural texture created (" << width << " x " << height << ")" << std::endl ;
}

// --------------------------------------------------------------------------------

TextureFromFile::TextureFromFile( VulkanContext * p_context, const std::string & p_file_path ) 

:   Texture( p_context )
{
    file_path = p_file_path ;

    int loaded_width = 0 ;
    int loaded_height = 0 ;
    int loaded_channels = 0 ;

    stbi_uc * loaded_data = stbi_load( file_path.c_str(), &loaded_width, &loaded_height, &loaded_channels, STBI_rgb_alpha ) ;
    if ( loaded_data == nullptr )
    {
        std::cerr << "Error loading texture image file: " << file_path << std::endl ;
        throw std::runtime_error( "Failed to load texture image file: " + file_path ) ;
    }
    
    width = static_cast<uint32_t>( loaded_width ) ;
    height = static_cast<uint32_t>( loaded_height ) ;
    total_size_bytes = width * height * 4 ;

    src_data = new unsigned char[ total_size_bytes ] ;
    assert( src_data != nullptr ) ;

    for ( uint32_t i = 0; i < width * height; ++i )
    {
        src_data[4*i + 0] = loaded_data[4*i + 0] ;
        src_data[4*i + 1] = loaded_data[4*i + 1] ;
        src_data[4*i + 2] = loaded_data[4*i + 2] ;
        src_data[4*i + 3] = 255 ;
    }

    stbi_image_free( loaded_data ) ;

    using namespace std ;
    cout << "Loaded texture image from file: " << file_path 
         << " (" << width << " x " << height << "), channels: " << loaded_channels << endl ;

    // initialize vulkan stuff
    initialize() ;
}

// --------------------------------------------------------------------------------

} // vkhc namespace end 
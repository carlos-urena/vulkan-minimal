// Declaration of classes TexturesSet, Texture, ProceduralTexture1, TextureFromFile.
//
// Texture: A class for objects which encapsulate a texture loaded to GPU memory
// TextureSet: a set of texture which can be bound to a descriptor set and accessed in shaders, 
#pragma once 

#include <common.h>
#include <vulkan-context.h>

namespace vkhc
{

// -----------------------------------------------------------------------------

class Texture 
{
    public:

    uint32_t        width = 0 ;
    uint32_t        height = 0 ;
    VkDeviceMemory  vk_memory ;    // memory in the GPU used to store image pixels 
    VkImage         vk_image ;     // image object in the GPU
    VkImageView     vk_image_view ; // image view object, used to bind the image to a descriptor set and access it in shaders

    uint32_t        total_size_bytes = 0 ;    // total size in bytes, an unsigned integer
    VkDeviceSize    vk_total_size_bytes = 0 ; // total size in bytes, type VkDeviceSize, for use in Vulkan API calls
    
    static constexpr VkFormat image_format = VK_FORMAT_R8G8B8A8_SRGB ; // this should be a parameter of the texture class, but for now we just hardcode it here.

    unsigned char * src_data = nullptr ; // pointer to the texture data in CPU memory, used for initialization
    VulkanContext * context = nullptr ;

    void initialize() ;
    Texture( VulkanContext * p_context ) ;
    ~Texture() ;

    void copyBufferToImage( VkBuffer buffer, VkImage image, uint32_t width, uint32_t height ) ;
} ;

// -----------------------------------------------------------------------------
// A texture set.

class TexturesSet 
{
    public:

    // max number of textures allowed.
    static constexpr uint32_t max_textures = 64 ; 
    VulkanContext * context = nullptr ;

    VkSampler vk_sampler { VK_NULL_HANDLE }; // shared sampler used by texture descriptors
    
    TexturesSet( VulkanContext * p_context ) ;

    std::vector<Texture *> textures ; // textures in this set, each with its index in the set (index in the vector)

    uint32_t add( Texture * texture ) ; // adds any texture. 
    uint32_t add( const std::string & file_path ) ; // creates a TextureFromFile and adds it to the set, given the file path of the texture image. Returns the index of the added texture in the set.

    void bindTo( BasicPipeline & pipeline ) ; // binds the textures set to a pipeline, by binding a descriptor set with an array of texture samplers, one for each texture in the set. The 'texture_index' push constant is used to select the active texture in the fragment shader, by indexing into this array of samplers.void TexturesSet::bindToPipeline( BasicPipeline & pipeline ) 

} ;

// --------------------------------------------------------------------------------
// A simple procedural texture, just for testing

class ProceduralTexture1 : public Texture 
{
    public:
    ProceduralTexture1( VulkanContext * p_context ) ;
} ;

// --------------------------------------------------------------------------------
// A texture loaded from an image file 

class TextureFromFile : public Texture 
{
    public:
    std::string file_path ;

    TextureFromFile( VulkanContext * p_context, const std::string & p_file_path ) ;
} ;

} // vkhc namespace end 
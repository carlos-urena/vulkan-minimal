// Implementation of the class 'VertexBuffer' 
//
// Encapsulates a vertex buffer (vkVertexBuffer) and its memory (vkDeviceMemory),


#include <vertex-buffer.h>
#include <device.h>

// ***********************************************************************************
// vulkan helper classes, mainly related to vulkan but also GLFW and IMGUI

namespace vkhc
{

void VertexBuffer::initialize
(
    Device * p_device,
    const size_t p_num_tuples,
    const size_t p_values_per_tuple,
    const size_t p_value_size_bytes,
    const byte * src_data,
    const VkBufferUsageFlags p_usage_flags  
)
{
    device = p_device;
    usage_flags = p_usage_flags ; 

    assert( device != nullptr );
    
    assert( p_num_tuples > 0 ) ;
    assert( p_values_per_tuple > 0 );
    assert( p_value_size_bytes > 0 );
    assert( src_data != nullptr );
    assert( usage_flags & VK_BUFFER_USAGE_VERTEX_BUFFER_BIT || usage_flags & VK_BUFFER_USAGE_INDEX_BUFFER_BIT );

    num_tuples          = p_num_tuples ;
    values_per_tuple    = p_values_per_tuple ;
    num_values          = num_tuples * values_per_tuple ;
    value_size_bytes    = p_value_size_bytes ;
    total_size_bytes    = num_values * value_size_bytes ;
    vk_total_size_bytes = static_cast<VkDeviceSize>( total_size_bytes ) ;

    // create buffer and host-visible and host-coherent memory
    device->createBufferAndCopyData( vk_total_size_bytes, src_data, usage_flags, 
                                     vk_buffer, vk_memory );

} 

VertexBuffer::VertexBuffer( Device * device, const std::span< const glm::vec2 > data_span ) 
{
    assert( data_span.size() > 0 );

    initialize( device,  data_span.size(), 2, sizeof(float), 
                reinterpret_cast<const byte*>(data_span.data()), 
                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT );

    std::cout << "Created vertex buffer with " << data_span.size() << " 'vec2'" << std::endl;
}

VertexBuffer::VertexBuffer( Device * device, const std::span< const glm::vec3 > data_span ) 
{
    assert( data_span.size() > 0 );

    initialize( device, data_span.size(), 3, sizeof(float), 
                reinterpret_cast<const byte*>(data_span.data()), 
                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT );

    std::cout << "Created vertex buffer with " << data_span.size() << " 'vec3'" << std::endl;
}

VertexBuffer::VertexBuffer( Device * device, const std::span< const glm::uvec3 > data_span ) 
{
    assert( data_span.size() > 0 );
    
    initialize( device, data_span.size(), 3, sizeof(unsigned int), 
        reinterpret_cast<const byte*>(data_span.data()),
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT
    );

    std::cout << "Created index buffer with " << data_span.size() << " 'uvec3'" << std::endl;
}

// adds this vertex buffer bind command to  a command buffer
void VertexBuffer::bind( VkCommandBuffer & vk_cmd_buffer, const uint32_t binding ) 
{
    assert( device != nullptr );
    assert( vk_cmd_buffer != VK_NULL_HANDLE );
    assert( usage_flags & VK_BUFFER_USAGE_VERTEX_BUFFER_BIT );

    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers( vk_cmd_buffer, binding, 1, &vk_buffer, offsets );
}

void VertexBuffer::bindIndices( VkCommandBuffer & vk_cmd_buffer, const VkIndexType indexes_type ) 
{
    assert( device != nullptr );
    assert( vk_cmd_buffer != VK_NULL_HANDLE );
    assert( usage_flags & VK_BUFFER_USAGE_INDEX_BUFFER_BIT );

    vkCmdBindIndexBuffer( vk_cmd_buffer, vk_buffer, 0, indexes_type );
}

VertexBuffer::~VertexBuffer() 
{
    vkDestroyBuffer( device->vk_device, vk_buffer, nullptr );
    vkFreeMemory( device->vk_device, vk_memory, nullptr );
}


} // end namespace vkhc 


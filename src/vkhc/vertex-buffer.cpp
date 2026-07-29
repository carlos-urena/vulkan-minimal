// Implementation of the class 'VertexBuffer' 
//
// Encapsulates a vertex buffer (vkVertexBuffer) and its memory (vkDeviceMemory),


#include <vkhc/vertex-buffer.h>
#include <vkhc/device.h>

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

    Assert( device != nullptr, "VertexBuffer::initialize: device pointer is null" );
    
    Assert( p_num_tuples > 0,       "VertexBuffer::initialize: number of tuples is zero" );
    Assert( p_values_per_tuple > 0, "VertexBuffer::initialize: values per tuple is zero" );
    Assert( p_value_size_bytes > 0, "VertexBuffer::initialize: value size in bytes is zero" );
    Assert( src_data != nullptr,    "VertexBuffer::initialize: source data pointer is null" );
    Assert( usage_flags & VK_BUFFER_USAGE_VERTEX_BUFFER_BIT || usage_flags & VK_BUFFER_USAGE_INDEX_BUFFER_BIT, 
                                    "VertexBuffer::initialize: buffer usage flags are invalid" );

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
// -------------------------------------------------------------------------------------------

VertexBuffer::VertexBuffer( Device * device, const std::span< const glm::vec2 > data_span ) 
{
    Assert( data_span.size() > 0, "VertexBuffer::VertexBuffer (vec2) - data span is empty" );

    initialize( device,  data_span.size(), 2, sizeof(float), 
                reinterpret_cast<const byte*>(data_span.data()), 
                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT );

    std::cout << "Created vertex buffer with " << data_span.size() << " 'vec2'" << std::endl;
}
// -------------------------------------------------------------------------------------------



VertexBuffer::VertexBuffer( Device * device, const std::span< const glm::vec3 > data_span ) 
{
    Assert( data_span.size() > 0, "VertexBuffer::VertexBuffer (vec3)- data span is empty" );

    initialize( device, data_span.size(), 3, sizeof(float), 
                reinterpret_cast<const byte*>(data_span.data()), 
                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT );

    std::cout << "Created vertex buffer with " << data_span.size() << " 'vec3'" << std::endl;
}
// -------------------------------------------------------------------------------------------


VertexBuffer::VertexBuffer( Device * device, const std::span< const glm::uvec3 > data_span ) 
{
    Assert( data_span.size() > 0, "VertexBuffer::VertexBuffer (uvec3) - data span is empty" );

    initialize( device, data_span.size(), 3, sizeof(unsigned int), 
        reinterpret_cast<const byte*>(data_span.data()),
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT
    );

    std::cout << "Created index buffer with " << data_span.size() << " 'uvec3'" << std::endl;
}
// -------------------------------------------------------------------------------------------

VertexBuffer::VertexBuffer( Device * device, const std::span< const unsigned > data_span ) 
{
    Assert( data_span.size() > 0, "VertexBuffer::VertexBuffer (unsigned) - data span is empty" );

    initialize( device, data_span.size(), 1, sizeof(unsigned int), 
        reinterpret_cast<const byte*>(data_span.data()),
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT
    );

    std::cout << "Created index buffer with " << data_span.size() << " 'unsigned int'" << std::endl;
}
// -------------------------------------------------------------------------------------------

// adds this vertex buffer bind command to  a command buffer

void VertexBuffer::bind( VkCommandBuffer & vk_cmd_buffer, const uint32_t binding ) 
{
    Assert( device != nullptr, "VertexBuffer::bind: device pointer is null" );
    Assert( vk_cmd_buffer != VK_NULL_HANDLE, "VertexBuffer::bind: command buffer is null" );
    Assert( usage_flags & VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, "VertexBuffer::bind: buffer is not a vertex buffer" );
    Assert( binding < device->maxVertexInputBindings, "VertexBuffer::bind: binding index exceeds maximum allowed by the device" );

    static const VkDeviceSize cero_offset[] = { 0 };
    vkCmdBindVertexBuffers( vk_cmd_buffer, binding, 1, &vk_buffer, cero_offset );
}
// -------------------------------------------------------------------------------------------

void VertexBuffer::bindIndices( VkCommandBuffer & vk_cmd_buffer, const VkIndexType indexes_type ) 
{
    Assert( device != nullptr, "VertexBuffer::bindIndices: device pointer is null" );
    Assert( vk_cmd_buffer != VK_NULL_HANDLE, "VertexBuffer::bindIndices: command buffer is null" );
    Assert( usage_flags & VK_BUFFER_USAGE_INDEX_BUFFER_BIT, "VertexBuffer::bindIndices: buffer is not an index buffer" );

    vkCmdBindIndexBuffer( vk_cmd_buffer, vk_buffer, 0, indexes_type );
}
// -------------------------------------------------------------------------------------------

VertexBuffer::~VertexBuffer() 
{
    vkDestroyBuffer( device->vk_device, vk_buffer, nullptr );
    vkFreeMemory( device->vk_device, vk_memory, nullptr );
}


} // end namespace vkhc 


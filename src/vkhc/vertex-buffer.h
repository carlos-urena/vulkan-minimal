// Declaration of the class 'VertexBuffer' 
//
// Encapsulates a vertex buffer (vkVertexBuffer) and its memory (vkDeviceMemory),

#pragma once 
#include <common.h>

namespace vkhc
{

class VertexBuffer
{
    public:

    Device * device = nullptr ; // a pointer to the device is kept for each buffer ? 

    VkBuffer       vk_buffer;
    VkDeviceMemory vk_memory ;
    size_t         num_tuples = 0 ;
    size_t         values_per_tuple = 0 ;
    size_t         num_values = 0 ;
    size_t         value_size_bytes = 0 ; 
    size_t         total_size_bytes = 0 ;
    VkDeviceSize   vk_total_size_bytes ; 

    VkBufferUsageFlags usage_flags ;
    VkIndexType        index_type ; // only used for index buffers, ignored for vertex buffers, but set in the constructor for index buffers.

    typedef unsigned char byte ;

    void initialize(
        Device * p_device,
        const size_t p_num_tuples,
        const size_t p_values_per_tuple,
        const size_t p_value_size_bytes,
        const byte * src_data,
        const VkBufferUsageFlags p_usage_flags  
    );
    
    VertexBuffer( Device * device, const std::span< const glm::vec2 > data_span ) ;
    
    
    VertexBuffer( Device * device, const std::span< const glm::vec3 > data_span ) ;
    

    VertexBuffer( Device * device, const std::span< const glm::uvec3 > data_span ) ;
    

    // adds this vertex buffer bind command to  a command buffer
    void bind( VkCommandBuffer & vk_cmd_buffer, const uint32_t binding ) ;
    

    void bindIndices( VkCommandBuffer & vk_cmd_buffer, const VkIndexType indexes_type ) ;
    

    ~VertexBuffer() ;
    

} ; // fin de la clase VertexBuffer

} // vkhc namespace end 


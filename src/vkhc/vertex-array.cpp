// Implementation of class 'VertexArray' 
//
// This class encapsulates the state of a vertex array, which consists of a 
// set of vertex buffers (with their corresponding attributes) and an optional 
// index buffer.

#include <vertex-array.h>
#include <vertex-buffer.h>
#include <vulkan-context.h>

namespace vkhc
{

// -------------------------------------------------------------------------------
// Constructor

VertexArray::VertexArray( VulkanContext & p_vulkan_context, const VkPrimitiveTopology p_topology ) 

:   topology( p_topology ), 
    vulkan_context( p_vulkan_context )
{    
}

// -------------------------------------------------------------------------------
// adds an already created vertex buffer, the caller is responsible to keep it alive 
// and delete it  after this vertex array is deleted

void VertexArray::addVertexBuffer( VertexBuffer * vertex_buffer ) 
{
    assert( vertex_buffer != nullptr );
    vertex_buffers.push_back( vertex_buffer );
    owner.push_back( false );
}
// -------------------------------------------------------------------------------
// sets a vertex buffer as the index buffer, the caller is responsible to keep it 
// alive and delete it after this vertex array is deleted.

void VertexArray::setIndexBuffer( VertexBuffer * vertex_buffer ) 
{
    assert( vertex_buffer != nullptr );
    assert( index_buffer == nullptr );
    index_buffer = vertex_buffer ;
    index_owner = false ;
}
// -------------------------------------------------------------------------------
// creates a vertex buffer from a data table and adds it to the vertex buffers vector,
// the vertex buffer is deleted when the destructor of this vertex array is called 

void VertexArray::addAttribData( const std::span< const glm::vec2 > data_span ) 
{
    vertex_buffers.push_back( new VertexBuffer( vulkan_context.device, data_span ) );
    owner.push_back( true );
}
// -------------------------------------------------------------------------------
// creates a vertex buffer from a data table and adds it to the vertex buffers vector,
// the vertex buffer is deleted when the destructor of this vertex array is called 

void VertexArray::addAttribData( const std::span< const glm::vec3 > data_span ) 
{
    vertex_buffers.push_back( new VertexBuffer( vulkan_context.device, data_span ) );  
    owner.push_back( true );
}

// -------------------------------------------------------------------------------

void VertexArray::setIndexData( const std::span< const glm::uvec3 > data_span ) 
{
    setIndexBuffer( new VertexBuffer( vulkan_context.device, data_span ) );
    index_owner = true ;
}
// -------------------------------------------------------------------------------


VertexArray::~VertexArray() 
{
    // delete each vertex buffer
    for ( size_t i = 0 ; i < vertex_buffers.size() ; i++ ) 
        if ( owner[i] ) delete vertex_buffers[i] ;
    
    vertex_buffers.clear();
    
    // delete the index buffer
    if ( index_buffer != nullptr && index_owner ) 
        delete index_buffer ;
    index_buffer = nullptr ;

    std::cout << "Deleted vertex array." << std::endl ;
}
// -------------------------------------------------------------------------------

void VertexArray::draw( VkCommandBuffer & vk_cmd_buffer )  
{
    assert( ! vertex_buffers.empty() );
    assert( vertex_buffers[0] != nullptr );
    assert( vertex_buffers[0]->num_tuples > 0 );
    //std::cout << "Vertex buffer 0 has " << vertex_buffers[0]->num_tuples << " tuples." << std::endl ;

    for( uint32_t i = 1 ; i < vertex_buffers.size() ; i++ ) 
    {
        assert( vertex_buffers[i] != nullptr );
        //std::cout << "Vertex buffer " << i << " has " << vertex_buffers[i]->num_tuples << " tuples." << std::endl ;
        assert( vertex_buffers[i]->num_tuples == vertex_buffers[0]->num_tuples ); // all vertex buffers should have the same number of vertexes (same size)
    }

    uint32_t i = 0 ;
    for ( auto vertex_buffer : vertex_buffers ) 
        vertex_buffer->bind( vk_cmd_buffer, i++ ); // bind each vertex buffer to its corresponding binding point in the pipeline 

    if ( vulkan_context.device != nullptr && vulkan_context.device->hasDynamicPrimitiveTopology )
        vkCmdSetPrimitiveTopology( vk_cmd_buffer, topology );

    if ( index_buffer != nullptr ) 
    {
        index_buffer->bindIndices( vk_cmd_buffer, VK_INDEX_TYPE_UINT32 );
        vkCmdDrawIndexed( vk_cmd_buffer, static_cast<uint32_t>(index_buffer->num_values), 1, 0, 0, 0 );
    }
    else 
    {
        // if no index buffer is set, we can draw non-indexed with the number of vertexes determined by the first vertex buffer (assuming all vertex buffers have the same number of vertexes)
        vkCmdDraw( vk_cmd_buffer, static_cast<uint32_t>(vertex_buffers[0]->num_values), 1, 0, 0 );
    }
}

} // vkhc namespace end 


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

VertexArray::VertexArray( VulkanContext & p_vulkan_context, const VkPrimitiveTopology p_topology, const int p_num_attributes  ) 

:   topology( p_topology ), 
    num_attributes( p_num_attributes ),
    vulkan_context( p_vulkan_context )
{  
    Assert( num_attributes > 0, "VertexArray constructor: num_attributes must be > 0" );
    vertex_buffers.resize( num_attributes, nullptr );
    owner.resize( num_attributes, false );
}

// -------------------------------------------------------------------------------
// clears a vertex buffer if it is set and owned.

void VertexArray::clearVertexBuffer( const uint32_t attribute_index ) 
{
    Assert( attribute_index < num_attributes, "VertexArray::clearVertexBuffer: attribute_index is out of bounds" );
    if ( vertex_buffers[attribute_index] == nullptr ) 
        return ;
        
    if ( owner[attribute_index] ) 
    {   delete vertex_buffers[attribute_index] ;
        vertex_buffers[attribute_index] = nullptr ;
        owner[attribute_index] = false ;
    }
}

// -------------------------------------------------------------------------------
// adds an already created vertex buffer, the caller is responsible to keep it alive 
// and delete it  after this vertex array is deleted

void VertexArray::setVertexBuffer( const uint32_t attribute_index, VertexBuffer * vertex_buffer ) 
{
    Assert( vertex_buffer != nullptr, "VertexArray::setVertexBuffer: vertex_buffer is null" );
    Assert( attribute_index < num_attributes, "VertexArray::setVertexBuffer: attribute_index is out of bounds" );
    clearVertexBuffer( attribute_index ) ; // clear the previous vertex buffer in this index if it is set and owned

    // for vertex atributes, check that the binding 0 is first and that the others bindings match the number of attrs
    if ( attribute_index > 0 ) 
    { 
        Assert( vertex_buffers[0] != nullptr, "VertexArray::setVertexBuffer: attribute_index > 0 but vertex_buffers[0] is null" );
        Assert( vertex_buffers[0]->num_tuples == vertex_buffer->num_tuples, "VertexArray::setVertexBuffer: attribute_index > 0 but vertex_buffers[0] has a different number of tuples than the new vertex buffer" );       
    }
    vertex_buffers[attribute_index] = vertex_buffer ;
    owner[attribute_index] = false ;
}

// -------------------------------------------------------------------------------
// creates a vertex buffer from a data table and adds it to the vertex buffers vector,
// the vertex buffer is deleted when the destructor of this vertex array is called 

void VertexArray::setAttribData( const uint32_t attribute_index, const std::span< const glm::vec2 > data_span ) 
{
    Assert( attribute_index < num_attributes, "VertexArray::setAttribData: attribute_index is out of bounds" );
    setVertexBuffer( attribute_index, new VertexBuffer( vulkan_context.device, data_span ) );
    owner[attribute_index] = true ;
}
// -------------------------------------------------------------------------------
// creates a vertex buffer from a data table and adds it to the vertex buffers vector,
// the vertex buffer is deleted when the destructor of this vertex array is called 

void VertexArray::setAttribData( const uint32_t attribute_index, const std::span< const glm::vec3 > data_span ) 
{
    Assert( attribute_index < num_attributes, "VertexArray::setAttribData: attribute_index is out of bounds" );
    setVertexBuffer( attribute_index, new VertexBuffer( vulkan_context.device, data_span ) );
    owner[attribute_index] = true ;
}

// -------------------------------------------------------------------------------

void VertexArray::clearIndexBuffer()
{
    if ( index_buffer == nullptr )
        return ; 
        
    if (index_owner ) 
    {
        delete index_buffer ;
        index_buffer = nullptr ;
        index_owner = false ;
    }
}

// -------------------------------------------------------------------------------
// sets a vertex buffer as the index buffer, the caller is responsible to keep it 
// alive and delete it after this vertex array is deleted.

void VertexArray::setIndexBuffer( VertexBuffer * vertex_buffer ) 
{
    Assert( vertex_buffer != nullptr , "VertexArray::setIndexBuffer: vertex_buffer is null" );
    clearIndexBuffer() ;
    index_buffer = vertex_buffer ;
    index_owner = false ;
}


// -------------------------------------------------------------------------------

void VertexArray::setIndexData( const std::span< const glm::uvec3 > data_span ) 
{
    setIndexBuffer( new VertexBuffer( vulkan_context.device, data_span ) );
    index_owner = true ;
}
// -------------------------------------------------------------------------------

void VertexArray::setIndexData( const std::span< const unsigned > data_span ) 
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
    Assert( ! vertex_buffers.empty() , "VertexArray::draw: vertex_buffers is empty" );
    Assert( vertex_buffers[0] != nullptr , "VertexArray::draw: vertex_buffers[0] is null" );
    Assert( vertex_buffers[0]->num_tuples > 0 , "VertexArray::draw: vertex_buffers[0] has no tuples" );
    //std::cout << "Vertex buffer 0 has " << vertex_buffers[0]->num_tuples << " tuples." << std::endl ;

    for( uint32_t i = 1 ; i < num_attributes ; i++ ) 
    {
        Assert( vertex_buffers[i] != nullptr , "VertexArray::draw: vertex_buffers[i] is null" );
        //std::cout << "Vertex buffer " << i << " has " << vertex_buffers[i]->num_tuples << " tuples." << std::endl ;
        Assert( vertex_buffers[i]->num_tuples == vertex_buffers[0]->num_tuples , "VertexArray::draw: vertex_buffers[i] has a different number of tuples" ); // all vertex buffers should have the same number of vertexes (same size)
    }

    for ( uint32_t i = 0 ; i < vertex_buffers.size() ; i++ )
        vertex_buffers[i]->bind( vk_cmd_buffer, i ); // bind each vertex buffer to its corresponding binding point in the pipeline 

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


// declaration of class 'VertexArray' 
//
// This class encapsulates the state of a vertex array, which consists of a 
// set of vertex buffers (with their corresponding attributes) and an optional 
// index buffer.

#pragma once 

#include <common.h>

namespace vkhc
{

// *****************************************************************************************
// A class to encapsulate vertex array state (a set of vertex buffers with attribs and indices)

class VertexArray
{
    public:

    VkPrimitiveTopology topology ;
    std::vector<VertexBuffer *> vertex_buffers ; // vertex buffers to bind to the pipeline 
    std::vector<bool> owner ; // for each added vertex buffer, indicates if this vertex array is the owner of the buffer (and should delete it in the destructor) or not (e.g. if the buffer is shared with other vertex arrays, it should not be deleted by this vertex array)
    VertexBuffer * index_buffer = nullptr ; // optional index buffer (if not null, it will be bound as index buffer, and draw calls should be indexed)
    bool index_owner = false ;
    VulkanContext & vulkan_context ;

    // Methods:

    VertexArray( VulkanContext & p_vulkan_context, const VkPrimitiveTopology p_topology ) ;
    
    // adds an already created vertex buffer, the caller is responsible to keep it alive 
    // and delete it  after this vertex array is deleted

    void addVertexBuffer( VertexBuffer * vertex_buffer ) ;
    
    // sets a vertex buffer as the index buffer, the caller is responsible to keep it 
    // alive and delete it after this vertex array is deleted.

    void setIndexBuffer( VertexBuffer * vertex_buffer ) ;
    
    // creates a vertex buffer from a data table and adds it to the vertex buffers vector,
    // the vertex buffer is deleted when the destructor of this vertex array is called 

    void addAttribData( const std::span< const glm::vec2 > data_span ) ;
    
    // creates a vertex buffer from a data table and adds it to the vertex buffers vector,
    // the vertex buffer is deleted when the destructor of this vertex array is called 

    void addAttribData( const std::span< const glm::vec3 > data_span ) ;
    

    void setIndexData( const std::span< const glm::uvec3 > data_span ) ;
    
    // Destructor
    ~VertexArray() ;
    
    // Adds the draw command for this vertex array to a command buffer, 
    void draw( VkCommandBuffer & vk_cmd_buffer )  ;
    
} ; // fin de la clase VertexArray

} // vkhc namespace end 


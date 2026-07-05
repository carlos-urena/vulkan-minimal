// Declaration of the class 'BasicPipeline2D' 
//
// Derived from 'BasicPipeline' 
// Encapsulates a simple 3D graphics pipeline, including vertex and 
// fragment shaders, but not tesselation or vertex shaders.

#pragma once 

#include <common.h>
#include <pipeline.h>


namespace vkhc
{

// -------------------------------------------------------------------------------
// Graphics pipeline state.

class Pipeline3D : public BasicPipeline
{
    private:

    // model matrix stack 
    std::vector<glm::mat4> model_matrix_stack ;
    // current model matrix 
    glm::mat4 current_model_matrix = glm::mat4(1.0f) ; 
    // current texture index 
    int current_texture_index = -1 ; // -1 means no texture is active

    // current view and projection matrices
    glm::mat4 current_view_matrix = glm::mat4(1.0f) ;
    glm::mat4 current_projection_matrix = glm::mat4(1.0f) ;

    public:
    Pipeline3D( VulkanContext & vulkan_context, const bool p_z_buffer_enabled );  

    // state which can be changed before sending commands
    void setViewMatrix( const glm::mat4 & new_view_matrix ) ;
    void setProjectionMatrix( const glm::mat4 & new_projection_matrix ) ;

    // dynamic state (can be changed dynamically in command buffers, without re-creating the pipeline)
    void setModelMatrix( VkCommandBuffer & vk_cmd, const glm::mat4 & model_mat ) ;
    void setTextureIndex( VkCommandBuffer & vk_cmd, int index ) ;

    // saves the current model matrix on the stack, 
    // composes the current and new matrix and sets a new current model matrix in this pipeline
    // adds a command to set it as the current model matrix in the shaders
    void pushModelMatrix( VkCommandBuffer & vk_cmd, const glm::mat4 & compose_model_mat ) ;

    // pops the previous model matrix from the stack,
    // sets it as the current model matrix in this pipeline
    // adds a command to set it as the current model matrix in the shaders
    // requires that the stack is not empty, otherwise aborts the program
    void popModelMatrix( VkCommandBuffer & vk_cmd ) ;

    // empties the model matrix stack, 
    // set the current model matrix to identity, 
    // adds a command to set it the model matrix as the identity in the shaders
    // if the stack was not empty, prints a warning message to the console.
    void resetModelMatrix( VkCommandBuffer & vk_cmd ) ;
    
} ; // end class 'Pipeline3D' 

// ------------------------------------------------------------------------------

} // end namespace 'vkhc' 


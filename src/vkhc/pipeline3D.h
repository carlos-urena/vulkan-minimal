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

constexpr int max_num_materials   = 64 ; // must be less or equal to 'max_num_materials' in the shaders
constexpr int max_num_base_colors = 64 ; // must be less or equal to 'max_num_base_colors' in the shaders

// -------------------------------------------------------------------------------

class BaseColorsSet
{
    public:
    
    // vector of base colors, to be used by the shaders
    static std::vector<glm::vec4> colors ; 

    // adds a base color to the set, returns its index 
    // if the set is already full, an error is raised 

    static int addBaseColor( const glm::vec3 & additional_color ) ;
} ;


// -------------------------------------------------------------------------------
// Graphics pipeline state.

class Pipeline3D : public BasicPipeline
{
    private:

    // model matrix stack (and normal matrix stack)
    std::vector<glm::mat4> model_matrix_stack ;
    std::vector<glm::mat4> model_matrix_normals_stack ;
    
    // current model matrix and model matrix for normals
    glm::mat4 current_model_matrix = glm::mat4(1.0f) ; 
    glm::mat4 current_model_matrix_normals = glm::mat4(1.0f) ;

    // current texture index 
    int current_texture_index = -1 ; // -1 means no texture is active

    // current base color index 
    int current_base_color_index = -1 ; // -1 means no base color is active

    // current view and projection matrices
    glm::mat4 current_view_matrix = glm::mat4(1.0f) ;
    glm::mat4 current_projection_matrix = glm::mat4(1.0f) ;

    public:
    Pipeline3D( VulkanContext & vulkan_context, const bool p_z_buffer_enabled );
    
    int getBaseColorIndex() { return current_base_color_index ; } // returns the index of the current base color, or -1 if no base color is active;
    int getTextureIndex()   { return current_texture_index ;    } // returns the index of the texture with the given name, or -1 if not found

    // adds a base color to the set of base colors

    // state which can be changed before sending commands
    void setViewMatrix( const glm::mat4 & new_view_matrix ) ;
    void setProjectionMatrix( const glm::mat4 & new_projection_matrix ) ;

    // dynamic state (can be changed dynamically in command buffers, without re-creating the pipeline)
    void setModelMatrix( VkCommandBuffer & vk_cmd, const glm::mat4 & model_mat ) ;
    void setTextureIndex( VkCommandBuffer & vk_cmd, int index ) ;
    void setBaseColorIndex( VkCommandBuffer & vk_cmd, int index ) ;

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

    // send the current base colors set ('BaseColorsSet::colors') to the shaders   ()
    void setBaseColorsSet() ;
    
} ; // end class 'Pipeline3D' 

// ------------------------------------------------------------------------------

} // end namespace 'vkhc' 


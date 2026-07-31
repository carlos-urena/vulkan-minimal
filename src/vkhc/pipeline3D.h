// Declaration of the class 'BasicPipeline2D' 
//
// Derived from 'BasicPipeline' 
// Encapsulates a simple 3D graphics pipeline, including vertex and 
// fragment shaders, but not tesselation or vertex shaders.

#pragma once 

#include <vkhc/common.h>
#include <vkhc/pipeline.h>


namespace vkhc
{

constexpr int max_num_materials   = 64 ; // must be less or equal to 'max_num_materials' in the shaders
constexpr int max_num_base_colors = 64 ; // must be less or equal to 'max_num_base_colors' in the shaders
constexpr int max_num_lights      = 8 ;

// -------------------------------------------------------------------------------

class BaseColorsSet
{
    public:
    
    // vector of base colors, to be used by the shaders
    std::vector<glm::vec4> colors ; 

    // adds a base color to the set, returns its index 
    // if the set is already full, an error is raised 

    uint32_t add( const glm::vec3 & additional_color ) ;
} ;

// ----------------------------------------------------------------------------------------------
// BRDF params struct

class BrdfParams
{
    public:
        float ka  = 0.1; // ambient coefficient
        float kd  = 0.8; // diffuse coefficient
        float ks  = 0.5; // specular coefficient
        float exp = 32.0; // specular exponent

        BrdfParams() = default ;
        BrdfParams( const float p_ka, const float p_kd, const float p_ks, const float p_exp ) ;
} ;

// ----------------------------------------------------------------------------------------------
// A set of BRDF parameters 

class BrdfParamsSet
{
    private:
        static constexpr uint32_t max_materials = 64 ; // max number of materials allowed.
        std::vector<BrdfParams> brdfs_params ; // BRDF parameters in this set, each with its index in the set (index in the vector)

    public:
        BrdfParamsSet(  ) ;
        uint32_t add( const BrdfParams & brdf_params ) ; // adds a new BRDF params to the set. Returns the index of the added BRDF params in the set.
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

    // current value for the eval_illumination push constant (true or false)
    bool eval_illumination = true ; // default value, can be changed dynamically in command

    // current value for the 'wireframe_mode' push constant (true or false)
    bool wireframe_mode = false ; // default value, can be changed dynamically in command

    // current values for various rendering parameters 
    bool draw_normals = false ; // draw normals (as line segments) of indexed triangle meshes 
    bool draw_wireframe = false ; // draw wireframe (edges) of indexed triangle meshes




    public:
    Pipeline3D( VulkanContext & vulkan_context, const bool p_z_buffer_enabled );
    
    int getBaseColorIndex() { return current_base_color_index ; } // returns the index of the current base color, or -1 if no base color is active;
    int getTextureIndex()   { return current_texture_index ;    } // returns the index of the texture with the given name, or -1 if not found
    bool getEvalIllumination() { return eval_illumination ; } // returns the current value of the 'eval_illumination' push constant
    bool getWireframeMode() { return wireframe_mode ; } // returns the current value of the 'wireframe_mode' push constant

    bool getDrawNormals() { return draw_normals ; } // returns the current value of the 'draw_normals' bool
    void setDrawNormals( bool new_draw_normals ) { draw_normals = new_draw_normals ; } // sets the current value of the 'draw_normals' bool
    
    bool getDrawWireframe() { return draw_wireframe ; } // returns the current value of the 'draw_wireframe' bool
    void setDrawWireframe( bool new_draw_wireframe ) { draw_wireframe = new_draw_wireframe ; } // sets the current value of the 'draw_wireframe' bool

    // adds a base color to the set of base colors

    // state which can be changed before sending commands
    void setViewMatrix( const glm::mat4 & new_view_matrix ) ;
    void setProjectionMatrix( const glm::mat4 & new_projection_matrix ) ;

    // dynamic state (can be changed dynamically in command buffers, without re-creating the pipeline)
    void setModelMatrix( VkCommandBuffer & vk_cmd, const glm::mat4 & model_mat ) ;
    void setTextureIndex( VkCommandBuffer & vk_cmd, int index ) ;
    void setBaseColorIndex( VkCommandBuffer & vk_cmd, int index ) ;
    void setWireframeMode( VkCommandBuffer & vk_cmd, bool new_wireframe_mode ) ;

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

    // send a set of base colors to the corresponding UBO uniform in the shaders   ()
    void setBaseColorsSet( const BaseColorsSet & bcs ) ;

    // sets the current illumination evaluation mode (true or false) in the shaders
    void setEvalIllumination( VkCommandBuffer & vk_cmd, bool new_eval_illumination );
    
} ; // end class 'Pipeline3D' 

// ------------------------------------------------------------------------------

} // end namespace 'vkhc' 


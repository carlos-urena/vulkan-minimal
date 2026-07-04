// Implementation of the class 'BasicPipeline2D' 
//
// Derived from 'BasicPipeline' 
// Encapsulates a simple 2D graphics pipeline, including vertex and 
// fragment shaders, but not tesselation or vertex shaders.


#include <pipeline3D.h>
#include <device.h>
#include <render-pass.h>
#include <vulkan-context.h>
#include <textures.h>

// ***********************************************************************************
// Basic shader sources (GLSL)


/// ----------------------------------------------------------------------------------
/// VERTEX SHADER 
/// ----------------------------------------------------------------------------------

static const char* vert_shader_src = R"glsl(
#version 450

    // Inputs: per vertex attributes:

    layout (location=0) in vec3 in_position;
    layout (location=1) in vec3 in_color;
    layout (location=2) in vec2 in_tex_coords ;

    // Inputs: push constants block:

    layout( push_constant, std430 ) uniform push_constants_block {
        mat4 model_mat ; // model matrix (object to world)
        int  texture_index ; // active texture index, -1 if no texture is active.
        int  material_index ; // active material index
    } pc ;

    // Inputs: uniform buffer object (WIP):

    const int max_materials = 64 ; // must be equal to 'MaterialsSet::max_materials'

    layout( set=0, binding=0 ) uniform ubo_block {
        mat4 view_mat; // view matrix (world to camera)
        mat4 proj_mat; // projection matrix (camera to clip)
        vec4 material_params[max_materials]; // array of materials parameters, indexed by 'material_index' push constant
        vec4 materials_colors[max_materials]; // array of materials colors, indexed by 'material_index' push constant
    } ubo ;

    // Inputs: array of texture samplers 
    // (we will use the 'texture_index' push constant to index into this array)

    const int max_textures = 64 ; // must be equal to 'TexturesSet::max_textures'
    layout( set=1, binding=0 ) uniform sampler2D textures[max_textures]; // array of texture samplers

    // Outputs: to fragment shader (or..)

    layout (location=0) out vec3 color;
    layout (location=1) out vec2 tex_coords ;

    void main() 
    {
        gl_Position = ubo.proj_mat * ubo.view_mat * pc.model_mat * vec4( in_position, 1.0 );
        color = in_color ;
        tex_coords = in_tex_coords ;
    }
)glsl";

/// ----------------------------------------------------------------------------------
/// FRAGMENT SHADER 
/// ----------------------------------------------------------------------------------

static const char* frag_shader_src = R"glsl(
#version 450
    // Inputs: push constants block:

    layout( push_constant, std430 ) uniform push_constants_block {
        mat4 model_mat ; // model matrix (object to world)
        int  texture_index ; // active texture index, -1 if no texture is active.
        int  material_index ; // active material index
    } pc ;

    // Inputs: uniform buffer object:

    const int max_materials = 64 ; // must be equal to 'MaterialsSet::max_materials'

    layout( set=0, binding=0 ) uniform ubo_block {
        mat4 view_mat; // view matrix (world to camera)
        mat4 proj_mat; // projection matrix (camera to clip)
        vec4 material_params[max_materials]; // array of materials parameters, indexed by 'material_index' push constant
        vec4 materials_colors[max_materials]; // array of materials colors, indexed by 'material_index' push constant
    } ubo ;

    // Inputs: an array of texture samplers (we will use the 'texture_index' push constant to index into this array and select the active texture, in the future when we implement texturing)
    const int max_textures = 64 ; // must be equal to 'TexturesSet::max_textures'
    layout( set=1, binding=0 ) uniform sampler2D textures[max_textures]; // array of texture samplers

    // Inputs: varying from vertex shader

    layout (location=0) in vec3 color;
    layout (location=1) in vec2 tex_coords ;

    // Output: fragment color 

    layout (location=0) out vec4 out_color;

    // --------------- 
    // Main function.

    void main()
    {
        if ( pc.texture_index >= 0 ) // if a texture is active, use it to determine the fragment color
             out_color = texture( textures[ pc.texture_index ], tex_coords ) ;
        else
            out_color = vec4( color, 1.0 );
    }
)glsl";


// -----------------------------------------------------------------------------------

namespace vkhc
{
static std::string 
    vertShaderSrc_string = std::string { vert_shader_src },
    fragShaderSrc_string = std::string { frag_shader_src };

Pipeline3D::Pipeline3D( VulkanContext & vulkan_context,
                        bool p_depth_test_enabled,
                        bool p_depth_write_enabled,
                        VkCompareOp p_depth_compare_op )

:   BasicPipeline( vulkan_context ) 
{
    using namespace std ; 
    cout << "Creating 3D pipeline..." << endl ;

    // set metadata about  push constants 
    addPushConstant( "model_mat", sizeof(glm::mat4) ); // model matrix 
    addPushConstant( "texture_index", sizeof(int) ); // active texture index, -1 if no texture is active.
    addPushConstant( "material_index", sizeof(int) ); // active material index

    const int max_materials = 64 ; // must be equal to 'MaterialsSet::max_materials' (move there when possible)

    // set metadata about UBO uniforms 
    addUBOUniform( "view_mat", sizeof(glm::mat4) ); // view matrix
    addUBOUniform( "proj_mat", sizeof(glm::mat4) ); // projection matrix
    addUBOUniform( "material_params",  sizeof(glm::vec4)*max_materials ); // array of materials parameters
    addUBOUniform( "materials_colors", sizeof(glm::vec4)*max_materials ); // array of materials colors
    
    // set shaders sources 
    shaders_sources = 
    {   .vertex_shader_src   = & vertShaderSrc_string, 
        .fragment_shader_src = & fragShaderSrc_string
    };

    // set attributes formats (must correspond with inputs to the vertex shaders the shaders sources)
    attributes_formats = 
    {   VK_FORMAT_R32G32B32_SFLOAT, // position (atrib 0) X,Y,Z
        VK_FORMAT_R32G32B32_SFLOAT, // color (atrib 1) R,G,B
        VK_FORMAT_R32G32_SFLOAT     // texture coords (atrib 2) U,V
    }; // color

    // set default (initial) primitive topology (can be changed dynamically in command buffers)
    default_primitive_topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST ;

    // Depth behavior is caller-configurable through constructor params.
    depth_test_enabled = p_depth_test_enabled;
    depth_write_enabled = p_depth_write_enabled;
    depth_compare_op = p_depth_compare_op;

    // initialize the vulkan pipeline  (in the context)
    initialize( ) ; 

    cout << "3D pipeline created." << endl ;
}
// ------------------------------------------------------------------------------

void Pipeline3D::setViewMatrix( const glm::mat4 & view_mat ) 
{
    setUBOUniform( "view_mat", value_ptr( view_mat ) );
}
// ------------------------------------------------------------------------------

void Pipeline3D::setProjectionMatrix( const glm::mat4 & proj_mat ) 
{
    setUBOUniform( "proj_mat", value_ptr( proj_mat ) );
}
// ------------------------------------------------------------------------------

void Pipeline3D::setTextureIndex( VkCommandBuffer & vk_cmd, int index ) 
{
    setPushConstant( vk_cmd, "texture_index", &index ); 
}
// ------------------------------------------------------------------------------

void Pipeline3D::setModelMatrix( VkCommandBuffer & vk_cmd, const glm::mat4 & model_mat ) 
{
    setPushConstant( vk_cmd, "model_mat", value_ptr( model_mat ) ); 
}
// ------------------------------------------------------------------------------


} // end namespace 'vkhc' 


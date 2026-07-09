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
/// Common inputs declarations for all shaders
/// Includes: UBO uniforms, push constants, and texture samplers
/// ----------------------------------------------------------------------------------

static const char* common_decls = R"glsl(

// Inputs: push constants block:

    layout( push_constant, std430 ) uniform push_constants_block {
        mat4 model_mat ; // model matrix (object to world)
        int  texture_index ; // active texture index, -1 if no texture is active.
        int  material_index ; // active material index, -1 if no material is active
        int base_color_index ; // base color , -1 use either texture color of interpolated vertex color
    } pc ;

    // Inputs: uniform buffer object (WIP):

    const int max_num_materials   = 64 ; // must be equal to 'MaterialsSet::max_materials'
    const int max_num_base_colors = 64 ; // must be equal to 'MaterialsSet::max_base_colors'

    layout( set=0, binding=0 ) uniform ubo_block {
        
        mat4 view_mat; // view matrix (world to camera)
        mat4 proj_mat; // projection matrix (camera to clip)
        
        vec4 material_params[max_num_materials]; // array of materials parameters, indexed by 'material_index' push constant
        vec4 materials_colors[max_num_materials]; // array of materials colors, indexed by 'material_index' push constant
        int  num_materials ; // current number of entries used in the 'material_params' and 'materials_colors' arrays (used?)
        
        vec4 base_colors[max_num_base_colors]; // array of base colors, indexed by 'base_color_index' push constant
        int  num_base_colors ; // current number of entries used in the 'base_colors' array (used?)
    
    } ubo ;

    // Inputs: array of texture samplers 
    // (we will use the 'texture_index' push constant to index into this array)

    const int max_textures = 64 ; // must be equal to 'TexturesSet::max_textures'
    layout( set=1, binding=0 ) uniform sampler2D textures[max_textures]; // array of texture samplers

)glsl";


/// ----------------------------------------------------------------------------------
/// VERTEX SHADER 
/// ----------------------------------------------------------------------------------

static const char* vert_shader_src = R"glsl(
#version 450

//#common_inputs_declarations

    // Inputs: per vertex attributes:

    layout (location=0) in vec3 in_position;
    layout (location=1) in vec3 in_color;
    layout (location=2) in vec2 in_tex_coords ;

    // Outputs: to fragment shader (or..)

    layout (location=0) out vec3 color;
    layout (location=1) out vec2 tex_coords ;

    void main() 
    {
        mat4 flipy_mat = mat4( 1.0,  0.0,  0.0,  0.0,
                               0.0, -1.0,  0.0,  0.0,
                               0.0,  0.0,  1.0,  0.0,
                               0.0,  0.0,  0.0,  1.0 ) ;
        gl_Position = ubo.proj_mat * ubo.view_mat * flipy_mat * pc.model_mat * vec4( in_position, 1.0 );
        color = in_color ;
        tex_coords = in_tex_coords ;
    }
)glsl";

/// ----------------------------------------------------------------------------------
/// FRAGMENT SHADER 
/// ----------------------------------------------------------------------------------

static const char* frag_shader_src = R"glsl(
#version 450

//#common_inputs_declarations

    // Inputs: varying from vertex shader

    layout (location=0) in vec3 in_color;
    layout (location=1) in vec2 in_tex_coords ;

    // Output: fragment color 

    layout (location=0) out vec4 out_color;

    // --------------- 
    // Main function.

    void main()
    {
        if ( pc.texture_index >= 0 ) // if a texture is active, use it to determine the fragment color
            out_color = texture( textures[ pc.texture_index ], in_tex_coords ) ;
        else if ( pc.base_color_index >= 0 ) // if a base color is active, use it to determine the fragment color
            out_color = ubo.base_colors[ int(pc.base_color_index) ] ; 
        else // otherwise, use the interpolated vertex color
            out_color = vec4( in_color, 1.0 );
    }
)glsl";


// -----------------------------------------------------------------------------------

namespace vkhc
{

static std::string processShaderSource( const std::string & shader_src ) 
{
    std::string result = shader_src ;
    result =  insert_source( result, "common_inputs_declarations", common_decls ) ;
    return result ;
}

static std::string 
    vertShaderSrc_string = processShaderSource( std::string( vert_shader_src ) ),
    fragShaderSrc_string = processShaderSource( std::string( frag_shader_src ) );

Pipeline3D::Pipeline3D( VulkanContext & vulkan_context, const bool p_z_buffer_enabled )

:   BasicPipeline( vulkan_context, p_z_buffer_enabled ) 
{
    using namespace std ; 
    cout << "Creating 3D pipeline..." << endl ;
    name = "Pipeline 3D" ;

    // set metadata about  push constants 
    addPushConstant( "model_mat", sizeof(glm::mat4) ); // model matrix 
    addPushConstant( "texture_index", sizeof(int) ); // active texture index, -1 if no texture is active.
    addPushConstant( "material_index", sizeof(int) ); // active material index, -1 if no material is active
    addPushConstant( "base_color_index", sizeof(int) ); // base color index, -1 if no base color is active.

    
    addUBOUniform( "view_mat",         VType::MAT4x4, 1 ); // view matrix
    addUBOUniform( "proj_mat",         VType::MAT4x4, 1 ); // projection matrix
    addUBOUniform( "material_params",  VType::VEC4,   max_num_materials ); // array of materials parameters
    addUBOUniform( "materials_colors", VType::VEC4,   max_num_materials ); // array of materials colors
    addUBOUniform( "num_materials",    VType::INT,    1 ); // current number of entries used in the 'material_params' and 'materials_colors' arrays
    addUBOUniform( "base_colors",      VType::VEC4,   max_num_base_colors ); // array of base colors
    addUBOUniform( "num_base_colors",  VType::INT,    1 ); // current number of entries used in the 'base_colors' array

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

    // initialize the vulkan pipeline  (in the context)
    initialize( ) ; 

    cout << "3D pipeline created." << endl ;
}
// ------------------------------------------------------------------------------

void Pipeline3D::setViewMatrix( const glm::mat4 & new_view_matrix ) 
{
    current_view_matrix = new_view_matrix ;
    setUBOUniform( "view_mat", value_ptr( current_view_matrix ) );
}
// ------------------------------------------------------------------------------

void Pipeline3D::setProjectionMatrix( const glm::mat4 & new_projection_matrix ) 
{
    current_projection_matrix = new_projection_matrix ;
    setUBOUniform( "proj_mat", value_ptr( current_projection_matrix ) );
}
// ------------------------------------------------------------------------------

void Pipeline3D::setTextureIndex( VkCommandBuffer & vk_cmd, int index ) 
{
    current_texture_index = index ;
    setPushConstant( vk_cmd, "texture_index", &index ); 
}

void Pipeline3D::setBaseColorIndex( VkCommandBuffer & vk_cmd, int index ) 
{
    current_base_color_index = index ;
    setPushConstant( vk_cmd, "base_color_index", &index ); 
    using namespace std ;
    //cout << "Pipeline3D::setBaseColorIndex: set base color index to " << index << endl ;
}
// ------------------------------------------------------------------------------

void Pipeline3D::setModelMatrix( VkCommandBuffer & vk_cmd, const glm::mat4 & model_mat ) 
{
    current_model_matrix = model_mat ;
    setPushConstant( vk_cmd, "model_mat", value_ptr( model_mat ) ); 
}
// ------------------------------------------------------------------------------
// saves the current model matrix on the stack, 
// composes the current and new matrix and sets a new current model matrix in this pipeline and in the shaders
// adds a command to set it as the current model matrix in the shaders

void Pipeline3D::pushModelMatrix( VkCommandBuffer & vk_cmd, const glm::mat4 & compose_model_mat ) 
{
    using namespace glm ;
    model_matrix_stack.push_back( current_model_matrix ) ;
    const mat4 new_model_mat = current_model_matrix * compose_model_mat ;
    setModelMatrix( vk_cmd, new_model_mat ) ;
}

// ------------------------------------------------------------------------------

// pops the previous model matrix from the stack,
// sets it as the current model matrix in this pipeline
// adds a command to set it as the current model matrix in the shaders
// requires that the stack is not empty, otherwise aborts the program

void Pipeline3D::popModelMatrix( VkCommandBuffer & vk_cmd ) 
{
    Assert( ! model_matrix_stack.empty(), "Pipeline3D::popModelMatrix: model matrix stack is empty, cannot pop." ) ;
    const glm::mat4 previous_model_mat = model_matrix_stack.back() ;
    model_matrix_stack.pop_back() ;
    setModelMatrix( vk_cmd, previous_model_mat ) ;
}

// ------------------------------------------------------------------------------

// empties the model matrix stack, 
// set the current model matrix to identity, 
// adds a command to set it the model matrix as the identity in the shaders
// if the stack was not empty, prints a warning message to the console.

void Pipeline3D::resetModelMatrix( VkCommandBuffer & vk_cmd ) 
{
    if ( ! model_matrix_stack.empty() )
        std::cout << "Pipeline3D::resetModelMatrix: WARNING: model matrix stack was not empty, it has been cleared." << std::endl ;
    model_matrix_stack.clear() ;
    current_model_matrix = glm::mat4(1.0f) ;
    setModelMatrix( vk_cmd, glm::mat4(1.0f) );
}

// ------------------------------------------------------------------------------
// send all colors, even if not all entries have been used .... improve later.

void Pipeline3D::setBaseColorsSet() 
{
    int nc = BaseColorsSet::colors.size() ;
    Assert( nc <= max_num_base_colors, "Pipeline3D::setBaseColorsSet: number of base colors exceeds maximum allowed." ) ;
    setUBOUniform( "num_base_colors", & nc );
    setUBOUniform( "base_colors", value_ptr( BaseColorsSet::colors[0] ) );

    // for ( unsigned i = 0 ; i < BaseColorsSet::colors.size() ; ++i )
    // {
    //     using namespace glm ;
    //     vec3 c = BaseColorsSet::colors[i] ;
    //     using namespace std ;
    //     cout << "Pipeline3D::setBaseColorsSet: sending base color " << i << ": (" << c.r << ", " << c.g << ", " << c.b << ")" << endl ;
    // }

    using namespace std ;
    //cout << "Pipeline3D::setBaseColorsSet: sending " << nc << " base colors to the shaders." << endl ;
}

// ------------------------------------------------------------------------------
// Base colors sets for the 3d pipeline 

std::vector<glm::vec4> BaseColorsSet::colors{} ; 

// adds a base color to the set, returns its index 
// if the set is already full, an error is raised 

int BaseColorsSet::addBaseColor( const glm::vec3 & additional_color ) 
{
    Assert( colors.size() < max_num_base_colors, "Error: cannot add a new base color, size exceeded (increase max_num_base_colors )" );  
    colors.push_back( glm::vec4( additional_color, 1.0f ) ) ;
    return colors.size() - 1 ; // return the index of the added color
}

// ------------------------------------------------------------------------------

} // end namespace 'vkhc' 


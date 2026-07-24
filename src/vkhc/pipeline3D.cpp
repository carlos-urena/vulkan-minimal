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
        mat4 model_mat ;         // model matrix (object to world)
        mat4 model_mat_normals ; // model matrix for normals (object to world)
        int  base_color_index ;  // base color , -1 use either texture color (if texture_index>=0) or interpolated vertex color
        int  texture_index ;     // active texture index, -1 if no texture is active.
        int  material_index ;    // active material index, -1 if no material is active
        int  eval_illumination ; // if not 0, evaluate illumination, if 0, use base color .
    } pc ;

    // Inputs: uniform buffer object (WIP):

    const int max_num_materials   = 64 ; // must be equal to 'MaterialsSet::max_materials'
    const int max_num_base_colors = 64 ; // must be equal to 'MaterialsSet::max_base_colors'

    layout( set=0, binding=0 ) uniform ubo_block {
        
        mat4 view_mat; // view matrix (world to camera)
        mat4 proj_mat; // projection matrix (camera to clip)

        vec4 base_colors[max_num_base_colors]; // array of base colors, indexed by 'base_color_index' push constant
        int  num_base_colors ; // current number of entries used in the 'base_colors' array (used?)
        
        vec4 material_params[max_num_materials]; // array of materials parameters, indexed by 'material_index' push constant
        int  num_materials ; // current number of entries used in the 'material_params' and 'materials_colors' arrays (used?)
        
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

    layout (location=0) in vec3 in_position_occ;
    layout (location=1) in vec3 in_color;
    layout (location=2) in vec3 in_normal_occ;
    layout (location=3) in vec2 in_tex_coords ;

    // Outputs: to fragment shader 

    layout (location=0) out vec3 out_position_wcc;
    layout (location=1) out vec3 out_color ; 
    layout (location=2) out vec3 out_normal_wcc ;
    layout (location=3) out vec2 out_tex_coords ;

    const mat4 flipy_mat = mat4( 1.0,  0.0,  0.0,  0.0,
                                 0.0, -1.0,  0.0,  0.0,
                                 0.0,  0.0,  1.0,  0.0,
                                 0.0,  0.0,  0.0,  1.0 ) ;

    void main() 
    {
        vec4 out_position_wcc_4 = pc.model_mat * vec4( in_position_occ, 1.0 ) ;
        gl_Position      = ubo.proj_mat * ubo.view_mat * flipy_mat  * out_position_wcc_4 ;

        out_position_wcc = out_position_wcc_4.xyz ; 
        out_color        = in_color ;
        out_normal_wcc   = (pc.model_mat_normals * vec4( in_normal_occ, 0.0 )).xyz ;
        out_tex_coords   = in_tex_coords ;
    }
)glsl";

/// ----------------------------------------------------------------------------------
/// FRAGMENT SHADER 
/// ----------------------------------------------------------------------------------

static const char* frag_shader_src = R"glsl(
#version 450

//#common_inputs_declarations

    // Inputs: varying from vertex shader

    layout (location=0) in vec3 in_position_wcc;
    layout (location=1) in vec3 in_color;
    layout (location=2) in vec3 in_normal_wcc;
    layout (location=3) in vec2 in_tex_coords ;
    
    // Output: fragment color 

    layout (location=0) out vec4 out_color;

    // --------------- 
    // Main function.

    vec3 BaseColor()
    {
        if ( pc.texture_index >= 0 ) // if a texture is active, use it to determine the fragment color
            return (texture( textures[ pc.texture_index ], in_tex_coords )).rgb ;
        else if ( pc.base_color_index >= 0 ) // if a base color is active, use it to determine the fragment color
            return (ubo.base_colors[ int(pc.base_color_index) ]).rgb ; 
        else // otherwise, use the interpolated vertex color
            return in_color ;
    }
    // ----------------

    vec3 EvalIllumination( const vec3 base_color )
    {
        vec3  n = normalize( in_normal_wcc ) ;
        vec3  l = normalize( vec3( 0.0, 1.0, 0.0 ) ) ;
        float d = max( dot( n, l ), 0.0 ) ;

        return d*base_color; 
    }
    // ----------------

    void main()
    {
        vec3 bc = BaseColor() ;

        if ( pc.eval_illumination != 0 )
            out_color = vec4( EvalIllumination( bc ), 1.0 ) ;
        else
            out_color = vec4( bc, 1.0 ) ;
    }
    //----------------- 

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
    addPushConstant( "model_mat_normals", sizeof(glm::mat4) ); // model matrix for normals
    addPushConstant( "base_color_index", sizeof(int) ); // base color index, -1 if no base color is active.
    addPushConstant( "texture_index", sizeof(int) ); // active texture index, -1 if no texture is active.
    addPushConstant( "material_index", sizeof(int) ); // active material index, -1 if no material is active
    addPushConstant( "eval_illumination", sizeof(int) ); // if not 0, evaluate illumination, if 0, use base color .
    
    
    addUBOUniform( "view_mat",         VType::MAT4x4, 1 ); // view matrix
    addUBOUniform( "proj_mat",         VType::MAT4x4, 1 ); // projection matrix
    addUBOUniform( "base_colors",      VType::VEC4,   max_num_base_colors ); // array of base colors
    addUBOUniform( "num_base_colors",  VType::INT,    1 ); // current number of entries used in the 'base_colors' array
    addUBOUniform( "material_params",  VType::VEC4,   max_num_materials ); // array of materials parameters
    addUBOUniform( "num_materials",    VType::INT,    1 ); // current number of entries used in the 'material_params' and 'materials_colors' arrays
   
    // set shaders sources 
    shaders_sources = 
    {   .vertex_shader_src   = & vertShaderSrc_string, 
        .fragment_shader_src = & fragShaderSrc_string
    };

    // set attributes formats (must correspond with inputs to the vertex shaders the shaders sources)
    attributes_formats = 
    {   VK_FORMAT_R32G32B32_SFLOAT, // position (atrib 0) X,Y,Z
        VK_FORMAT_R32G32B32_SFLOAT, // color (atrib 1) R,G,B
        VK_FORMAT_R32G32B32_SFLOAT, // normal (atrib 2) X,Y,Z
        VK_FORMAT_R32G32_SFLOAT     // texture coords (atrib 3) U,V
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
// ------------------------------------------------------------------------------

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
    using namespace glm ;
    current_model_matrix         = model_mat ;
    current_model_matrix_normals = transpose( inverse( model_mat ) ) ;

    setPushConstant( vk_cmd, "model_mat", value_ptr( model_mat ) ); 
    setPushConstant( vk_cmd, "model_mat_normals", value_ptr( current_model_matrix_normals ) );  
}

// ------------------------------------------------------------------------------
// saves the current model matrix on the stack, 
// composes the current and new matrix and sets a new current model matrix in this pipeline and in the shaders
// adds a command to set it as the current model matrix in the shaders

void Pipeline3D::pushModelMatrix( VkCommandBuffer & vk_cmd, const glm::mat4 & compose_model_mat ) 
{
    using namespace glm ;
    const mat4 compose_model_mat_normals = transpose( inverse( compose_model_mat ) ) ;

    model_matrix_stack.push_back( current_model_matrix ) ;
    model_matrix_normals_stack.push_back( current_model_matrix_normals ) ;

    current_model_matrix         = current_model_matrix * compose_model_mat ;
    current_model_matrix_normals = current_model_matrix_normals * compose_model_mat_normals ;
    
    setPushConstant( vk_cmd, "model_mat", value_ptr( current_model_matrix ) ); 
    setPushConstant( vk_cmd, "model_mat_normals", value_ptr( current_model_matrix_normals ) ); 
}

// ------------------------------------------------------------------------------

// pops the previous model matrix from the stack,
// sets it as the current model matrix in this pipeline
// adds a command to set it as the current model matrix in the shaders
// requires that the stack is not empty, otherwise aborts the program

void Pipeline3D::popModelMatrix( VkCommandBuffer & vk_cmd ) 
{
    using namespace glm ;
    
    Assert( ! model_matrix_stack.empty(), "Pipeline3D::popModelMatrix: model matrix stack is empty, cannot pop." ) ;
    Assert( ! model_matrix_normals_stack.empty(), "Pipeline3D::popModelMatrix: model matrix normals stack is empty, cannot pop." ) ;
    
    current_model_matrix         = model_matrix_stack.back() ;
    current_model_matrix_normals = model_matrix_normals_stack.back() ;

    model_matrix_stack.pop_back() ;
    model_matrix_normals_stack.pop_back() ;

    setPushConstant( vk_cmd, "model_mat", value_ptr( current_model_matrix ) ); 
    setPushConstant( vk_cmd, "model_mat_normals", value_ptr( current_model_matrix_normals ) ); 
}

// ------------------------------------------------------------------------------

// empties the model matrix stack, 
// set the current model matrix to identity, 
// adds a command to set it the model matrix as the identity in the shaders
// if the stack was not empty, prints a warning message to the console.

void Pipeline3D::resetModelMatrix( VkCommandBuffer & vk_cmd ) 
{
    using namespace std ;
    using namespace glm ;

    if ( ! model_matrix_stack.empty() )
        cout << "Pipeline3D::resetModelMatrix: WARNING: model matrix stack was not empty, it has been cleared." << endl ;
    if ( ! model_matrix_normals_stack.empty() ) 
        cout << "Pipeline3D::resetModelMatrix: WARNING: model matrix normals stack was not empty, it has been cleared." << endl ;

    model_matrix_stack.clear() ;
    model_matrix_normals_stack.clear() ;

    current_model_matrix          = mat4(1.0f) ;
    current_model_matrix_normals  = mat4(1.0f);

    setPushConstant( vk_cmd, "model_mat", value_ptr( current_model_matrix ) ); 
    setPushConstant( vk_cmd, "model_mat_normals", value_ptr( current_model_matrix_normals ) ); 
}

// ------------------------------------------------------------------------------
// send all colors, even if not all entries have been used .... improve later.

void Pipeline3D::setBaseColorsSet() 
{
    int nc = BaseColorsSet::colors.size() ;
    Assert( nc <= max_num_base_colors, "Pipeline3D::setBaseColorsSet: number of base colors exceeds maximum allowed." ) ;
    setUBOUniform( "num_base_colors", & nc );
    setUBOUniform( "base_colors", value_ptr( BaseColorsSet::colors[0] ) );

    // Debug :
    // using namespace std ;

    // for ( unsigned i = 0 ; i < BaseColorsSet::colors.size() ; ++i )
    // {
    //     using namespace glm ;
    //     vec3 c = BaseColorsSet::colors[i] ;
    //     using namespace std ;
    //     cout << "Pipeline3D::setBaseColorsSet: sending base color " << i << ": (" << c.r << ", " << c.g << ", " << c.b << ")" << endl ;
    // }

    
    //cout << "Pipeline3D::setBaseColorsSet: sending " << nc << " base colors to the shaders." << endl ;
}

// ------------------------------------------------------------------------------

void Pipeline3D::setEvalIllumination( VkCommandBuffer & vk_cmd, bool new_eval_illumination )
{
    eval_illumination = new_eval_illumination ;
    int eval_illumination_int = eval_illumination ? 1 : 0 ;
    setPushConstant( vk_cmd, "eval_illumination", & eval_illumination_int );
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



} // end namespace 'vkhc' 


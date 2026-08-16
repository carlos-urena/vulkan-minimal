// Implementation of the class 'BasicPipeline2D' 
//
// Derived from 'BasicPipeline' 
// Encapsulates a simple 2D graphics pipeline, including vertex and 
// fragment shaders, but not tesselation or vertex shaders.


#include <vkhc/pipeline3D.h>
#include <vkhc/device.h>
#include <vkhc/render-pass.h>
#include <vkhc/vulkan-context.h>
#include <vkhc/textures.h>

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
        int  base_color_index ;  // base color , -1 use either texture color (if texture_index>=0) or interpolated vertex color (otherwise)
        int  texture_index ;     // active texture index, -1 if no texture is active.
        int  brdf_params_index ;  // active material paramaters index, -1 if no material is active (do not evaluate illumination)
        int  eval_illumination ; // if not 0, evaluate illumination, if 0, use base color .
        int  wireframe_mode ; 
    } pc ;

    // Inputs: uniform buffer object (WIP):

    const int max_num_brdfs_params = 64 ; // must be equal to 'MaterialsSet::max_materials'
    const int max_num_base_colors  = 64 ; // must be equal to 'MaterialsSet::max_base_colors'
    const int max_num_lights       = 8 ;
    
    layout( set=0, binding=0 ) uniform ubo_block {
        
        mat4 view_mat; // view matrix (world to camera)
        mat4 view_mat_inv ; // inverse view matrix (camera to world)
        mat4 proj_mat; // projection matrix (camera to clip)

        vec4 base_colors[max_num_base_colors]; // array of base colors, indexed by 'base_color_index' push constant
        int  num_base_colors ; // current number of entries used in the 'base_colors' array (used?)
        
        vec4 brdfs_params[max_num_brdfs_params]; // array of brdf parameters, indexed by 'brdf_params_index' push constant
        int  num_brdfs_params ; // current number of entries used in the 'brdf_params' and 'materials_colors' arrays (used?)

        vec4 lights_dir[max_num_lights] ;
        vec4 lights_color[max_num_lights] ;
        int  num_lights ;
        
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

    // const mat4 flipy_mat = mat4( 1.0,  0.0,  0.0,  0.0,
    //                              0.0, -1.0,  0.0,  0.0,
    //                              0.0,  0.0,  1.0,  0.0,
    //                              0.0,  0.0,  0.0,  1.0 ) ;

    // const mat4 z_offset_mat = mat4( 1.0,  0.0,  0.0,   0.0,
    //                                 0.0,  1.0,  0.0,   0.0,
    //                                 0.0,  0.0,  1.0,   0.0,
    //                                 0.0,  0.0,  0.001, 1.0 ) ;

    const mat4 z_offset_mat = mat4( 1.0,  0.0,  0.0,   0.0,
                                    0.0,  1.0,  0.0,   0.0,
                                    0.0,  0.0,  1.0,   -0.001,
                                    0.0,  0.0,  0.0, 1.0 ) ;


    // ------------------------------------------------------------------------------------

    mat4 GetProjectionMatrix()
    {
        if ( pc.wireframe_mode == 0 )
            return ubo.proj_mat ;
        else
            // add a small offset to the projection matrix to avoid z-fighting when drawing wireframe over filled triangles
            return ubo.proj_mat * z_offset_mat ;
    }

    // ---------------------------------------------------------------------------------------

    void main() 
    {
        vec4 out_position_wcc_4 = pc.model_mat * vec4( in_position_occ, 1.0 ) ;
        mat4 proj_mat_adjusted  = GetProjectionMatrix() ;

        //gl_Position      = proj_mat_adjusted * ubo.view_mat * flipy_mat  * out_position_wcc_4 ;
        gl_Position      = proj_mat_adjusted * ubo.view_mat * out_position_wcc_4 ;

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

    //  const mat4 flipy_mat = mat4( 1.0,  0.0,  0.0,  0.0,
    //                              0.0, -1.0,  0.0,  0.0,
    //                              0.0,  0.0,  1.0,  0.0,
    //                              0.0,  0.0,  0.0,  1.0 ) ;

    // --------------------------------------------------------------------
    // returns the base color: either the texture color, or the base color, or the color interpolated from the vertexes colors

    vec3 BaseColor()
    {
        if ( pc.texture_index >= 0 ) // if a texture is active, use it 
            return (texture( textures[ pc.texture_index ], in_tex_coords )).rgb ;
        else if ( pc.base_color_index >= 0 ) // if a base color is active, use it 
            return (ubo.base_colors[ int(pc.base_color_index) ]).rgb ; 
        else // otherwise, use the interpolated vertex color
            return in_color ;
    }

    // -----------------------------------------------------------------------------
    // returns the view vector 

    vec3 ViewVectorWCC()
    {
        //return normalize( ( flipy_mat * ubo.view_mat_inv * vec4(0.0,0.0,0.0,1.0) ).xyz - in_position_wcc ) ;
        return normalize( ( ubo.view_mat_inv * vec4(0.0,0.0,0.0,1.0) ).xyz - in_position_wcc ) ;
    }

    // ---------------- 
    // Evaluates the illumination at the current fragment, using the Blinn-Phong reflection model, with multiple lights and materials.

    vec3 EvalIllumination( const vec3 base_color )
    {
        vec3  v = ViewVectorWCC() ;
        vec3  n = normalize( in_normal_wcc ) ;
        
        vec3  l0  = normalize( vec3( 0.5, 1.0, 0.5 ) ) ;
        vec3  h0  = normalize( l0 + v ) ;
        vec3  cl0 = vec3( 1.0, 1.0, 1.0 ) ;
        float d0  = max( dot( n, l0 ), 0.0 ) ;
        float h0n = max( dot( n, h0 ), 0.0 ) ;

        vec3  l1  = normalize( vec3( -0.5, -1.0, 0.2 ) ) ;
        vec3  h1  = normalize( l1 + v ) ;
        vec3  cl1 = vec3( 0.6, 0.3, 0.2 ) ;  
        float d1  = max( dot( n, l1 ), 0.0 ) ;
        float h1n = max( dot( n, h1 ), 0.0 ) ;

        vec4 brdf_params = ubo.brdfs_params[ int(pc.brdf_params_index) ] ;
        float ka = brdf_params[0] ; // ambient coefficient
        float kd = brdf_params[1] ; // diffuse coefficient
        float ks = brdf_params[2] ; // specular coefficient
        float exp = brdf_params[3] ; // specular exponent

        vec3 ambient  = vec3(ka,ka,ka) ;
        vec3 diffuse  = kd*( d0*cl0 + d1*cl1 ) ;
        vec3 specular = ks*( pow(h0n,exp)*cl0 + pow(h1n,exp)*cl1 ) ;

        return base_color*(ambient + diffuse) + specular  ; 
    }
    // ----------------

    void main()
    {
        if ( pc.wireframe_mode != 0 )
        {
            out_color = vec4( 1.0, 0.2, 0.2, 1.0 ) ; 
            return ; 
        }
        
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

// -----------------------------------------------------------------------------------

static std::string processShaderSource( const std::string & shader_src ) 
{
    std::string result = shader_src ;
    result =  insert_source( result, "common_inputs_declarations", common_decls ) ;
    return result ;
}
// -----------------------------------------------------------------------------------

static std::string 
    vertShaderSrc_string = processShaderSource( std::string( vert_shader_src ) ),
    fragShaderSrc_string = processShaderSource( std::string( frag_shader_src ) );

// -----------------------------------------------------------------------------------

Pipeline3D::Pipeline3D( VulkanContext & vulkan_context, const bool p_z_buffer_enabled )

:   BasicPipeline( vulkan_context, p_z_buffer_enabled ) 
{
    using namespace std ; 
    cout << "Creating 3D pipeline..." << endl ;
    name = "Pipeline 3D" ;

    // set metadata about  push constants 
    addPushConstant( "model_mat",         sizeof(glm::mat4) ); // model matrix 
    addPushConstant( "model_mat_normals", sizeof(glm::mat4) ); // model matrix for normals
    addPushConstant( "base_color_index",  sizeof(int) ); // base color index, -1 if no base color is active.
    addPushConstant( "texture_index",     sizeof(int) ); // active texture index, -1 if no texture is active.
    addPushConstant( "brdf_params_index", sizeof(int) ); // active material index, -1 if no material is active
    addPushConstant( "eval_illumination", sizeof(int) ); // if not 0, evaluate illumination, if 0, use base color .
    addPushConstant( "wireframe_mode",    sizeof(int) ); // if not 0, we are drawing wireframe, if 0, we are drawing
    
    addUBOUniform( "view_mat",         VType::MAT4x4, 1 ); // view matrix
    addUBOUniform( "view_mat_inv",     VType::MAT4x4, 1 ); // inverse view matrix
    addUBOUniform( "proj_mat",         VType::MAT4x4, 1 ); // projection matrix
    addUBOUniform( "base_colors",      VType::VEC4,   max_num_base_colors ); // array of base colors
    addUBOUniform( "num_base_colors",  VType::INT,    1 ); // current number of entries used in the 'base_colors' array
    addUBOUniform( "brdfs_params",     VType::VEC4,   max_num_materials ); // array of materials parameters
    addUBOUniform( "num_brdfs_params", VType::INT,    1 ); // current number of entries used in the 'material_params' and 'materials_colors' arrays
    addUBOUniform( "lights_dir",       VType::VEC4,   max_num_lights ); // array of light directions
    addUBOUniform( "lights_color",     VType::VEC4,   max_num_lights ); // array of light colors
    addUBOUniform( "num_lights",       VType::INT,    1 ); // current number of entries used in the 'light_dir' and 'light_color' arrays

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

void Pipeline3D::setViewMatrix( const glm::mat4 & new_view_matrix, const glm::mat4 & new_view_matrix_inv ) 
{
    current_view_matrix = new_view_matrix ;
    current_view_matrix_inv = new_view_matrix_inv ;
    setUBOUniform( "view_mat", value_ptr( current_view_matrix ) );
    setUBOUniform( "view_mat_inv", value_ptr( current_view_matrix_inv ) );
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

void Pipeline3D::setBrdfParamsIndex( VkCommandBuffer & vk_cmd, int index ) 
{
    current_brdf_params_index = index ;
    setPushConstant( vk_cmd, "brdf_params_index", &index ); 
    using namespace std ;
    //cout << "Pipeline3D::setBrdfParamsIndex: set brdf params index to " << index << endl ;
}

// ------------------------------------------------------------------------------

void Pipeline3D::setWireframeMode( VkCommandBuffer & vk_cmd, bool new_wireframe_mode )
{ 
    wireframe_mode = new_wireframe_mode ; 
    int wireframe_mode_int = wireframe_mode ? 1 : 0 ;
    setPushConstant( vk_cmd, "wireframe_mode", &wireframe_mode_int ); 
} ;
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
// send all colors to the GPU UBO uniform block

void Pipeline3D::setBaseColorsSet( BaseColorsSet & bcs ) 
{
    num_base_colors = bcs.colors.size() ;
    Assert( num_base_colors <= max_num_base_colors, "Pipeline3D::setBaseColorsSet: number of base colors exceeds maximum allowed." ) ;
    //base_colors_set = &bcs ; // store the pointer to the base colors set (to be able to update it later)
    
    setUBOUniform( "num_base_colors", & num_base_colors ); 
    setUBOUniform( "base_colors", value_ptr( bcs.colors[0] ) );    
}

// ------------------------------------------------------------------------------
// Updates a base color in the base colors set already binded to this pipeline

void Pipeline3D::updateBaseColor( const int index, const glm::vec4 & color ) 
{
    //Assert( base_colors_set != nullptr, "Pipeline3D::setBaseColor: base colors set is not initialized." ) ;
    //Assert( 0<= index && index < (int)base_colors_set->colors.size(), "Pipeline3D::setBaseColor: base color index out of range." ) ;
    //base_colors_set->colors[index] = color ;
    Assert( 0 <= index && index < max_num_base_colors, "Pipeline3D::updateBaseColor: base color index out of range." ) ;
    setUBOUniform( "base_colors", index, value_ptr( color ) );
}

// ------------------------------------------------------------------------------
// send all BRDF parameters to the UBO uniform block

void Pipeline3D::setBrdfParamsSet( BrdfParamsSet & bps ) 
{
    num_brdfs_params = bps.brdfs_params.size() ;
    Assert( num_brdfs_params <= BrdfParamsSet::max_num_brdfs_params, "Pipeline3D::setBrdfParamsSet: number of BRDF params exceeds maximum allowed." ) ;

    //brdf_params_set = &bps ; // store the pointer to the BRDF params set (to be able to update it later)

    setUBOUniform( "num_brdfs_params", & num_brdfs_params );
    setUBOUniform( "brdfs_params", value_ptr( bps.brdfs_params_vec4[0]) );
}
// ------------------------------------------------------------------------------

void Pipeline3D::updateBrdfParams( const int index, const BrdfParams & brdf_params ) 
{
    //Assert( brdf_params_set != nullptr, "Pipeline3D::updateBrdfParams: BRDF params set is not initialized." ) ;
    //uint32_t nb = brdf_params_set->brdfs_params.size() ;
    //Assert( 0 <= index  && index < nb, "Pipeline3D::updateBrdfParams: BRDF params index out of range." ) ;

    Assert( 0 <= index && index < num_brdfs_params, "Pipeline3D::updateBrdfParams: BRDF params index out of range." ) ;
    glm::vec4 params_v4 = glm::vec4( brdf_params.ka, brdf_params.kd, brdf_params.ks, brdf_params.exp ) ;
    setUBOUniform( "brdfs_params", index, value_ptr( params_v4 ) ) ;

    using namespace std ;
    // cout << "---> Pipeline3D::updateBrdfParams: updated BRDF params at index " << index 
    // << ": ka=" << brdf_params.ka << ", kd=" << brdf_params.kd << ", ks=" << brdf_params.ks << ", exp=" << brdf_params.exp 
    // << endl ;
}

// ------------------------------------------------------------------------------

void Pipeline3D::setEvalIllumination( VkCommandBuffer & vk_cmd, bool new_eval_illumination )
{
    eval_illumination = new_eval_illumination ;
    int eval_illumination_int = eval_illumination ? 1 : 0 ;
    setPushConstant( vk_cmd, "eval_illumination", & eval_illumination_int );
}

// ------------------------------------------------------------------------------
//std::vector<glm::vec4> BaseColorsSet::colors{} ; 

// adds a base color to the set, returns its index 
// if the set is already full, an error is raised 

uint32_t BaseColorsSet::add( const glm::vec3 & additional_color ) 
{
    Assert( colors.size() < max_num_base_colors, "Error: cannot add a new base color, size exceeded (increase max_num_base_colors )" );  
    colors.push_back( glm::vec4( additional_color, 1.0f ) ) ;
    return colors.size() - 1 ; // return the index of the added color
}

// ------------------------------------------------------------------------------

BrdfParams::BrdfParams( const float p_ka, const float p_kd, const float p_ks, const float p_exp ) 
{
    ka = p_ka ;
    kd = p_kd ;
    ks = p_ks ;
    exp = p_exp ;
}

// ------------------------------------------------------------------------------
// Class BrdfParamsSet

BrdfParamsSet::BrdfParamsSet(  ) 
{

}

// ------------------------------------------------------------------------------

uint32_t BrdfParamsSet::add( const BrdfParams & brdf ) 
{
    glm::vec4 v4 = { brdf.ka, brdf.kd, brdf.ks, brdf.exp } ;
    using namespace std ;
    cout << "Pipeline3D::BrdfParamsSet::add: adding BRDF params:" << endl ;
    Assert( brdfs_params.size() < max_num_brdfs_params, "Error: cannot add a new BRDF params, size exceeded (increase max_num_brdfs_params )" );

    
    cout << "    vec size == " << brdfs_params.size() << endl ;
    brdfs_params.push_back( brdf ) ;
    brdfs_params_vec4.push_back( v4 ) ;
    cout << "    done." << endl ;

    return static_cast<uint32_t>( brdfs_params.size() - 1 ) ;
}


} // end namespace 'vkhc' 


// Implementation of the class 'BasicPipeline' 
//
// Encapsulates a graphics pipeline, including the Vulkan 
// pipeline object (VkPipeline), the pipeline layout, and the 
// descriptor set layout for UBOs.


#include <pipeline2D_tess.h>
#include <device.h>
#include <render-pass.h>
#include <vulkan-context.h>
#include <textures.h>

// ***********************************************************************************
// Basic shader sources (GLSL)


/// ----------------------------------------------------------------------------------
/// VERTEX SHADER 
/// ----------------------------------------------------------------------------------

static const char* vertShaderSrc = R"glsl(
#version 450

    // Inputs: per vertex attributes:

    layout (location=0) in vec2 in_position;
    layout (location=1) in vec3 in_color;
    layout (location=2) in vec2 in_tex_coords ;

    // Inputs: push constants block:

    layout( push_constant, std430 ) uniform push_constants_block {
        mat4 model_mat ; // model matrix (object to world)
        int  texture_index ; // active texture index, -1 if no texture is active.
        
    } pc ;

    // Inputs: uniform buffer object (WIP):

    layout( set=0, binding=0 ) uniform ubo_block 
    {
        mat4 view_mat; // view matrix (world to camera)
        mat4 proj_mat; // projection matrix (camera to clip)
        float tsc_inner_level ;   // inner tessellation levels
        float tsc_outer_level ;   // outer tessellation levels
    } ubo ;

    // Inputs: array of texture samplers 
    // (we will use the 'texture_index' push constant to index into this array)

    //const int max_textures = 64 ; // must be equal to 'TexturesSet::max_textures'
    //layout( set=1, binding=0 ) uniform sampler2D textures[max_textures]; // array of texture samplers

    // Outputs: to fragment shader (or..)

    layout (location=0) out vec3 color_vs;
    layout (location=1) out vec2 tex_coords_vs;

    void main() 
    {
        gl_Position =  ubo.proj_mat * ubo.view_mat * pc.model_mat * vec4( in_position, 0.0, 1.0);
        color_vs      = in_color ;
        tex_coords_vs = in_tex_coords ;
    }
)glsl";





// --------------------------------------------------------------------------------
// TESSELLATION CONTROL SHADER
// --------------------------------------------------------------------------------

static const char* tescShaderSrc = R"glsl(
#version 450

// size 3 patches (triangles)
layout(vertices = 3) out;

// Inputs: push constants block:
layout( push_constant, std430 ) uniform push_constants_block {
    mat4  model_mat ; // model matrix (object to world)
    int   texture_index ; // active texture index, -1 if no texture is active.
    
} pc ;

// Inputs: uniform buffer object:
layout( set=0, binding=0 ) uniform ubo_block 
{
    mat4 view_mat; // view matrix (world to camera)
    mat4 proj_mat; // projection matrix (camera to clip)
    float tsc_inner_level ; // inner tessellation levels
    float tsc_outer_level ; // outer tessellation levels
} ubo ;

// Inputs: an array of texture samplers (we will use the 'texture_index' push constant to index into this array and select the active texture, in the future when we implement texturing)
const int max_textures = 64 ; // must be equal to 'TexturesSet::max_textures'
layout( set=1, binding=0 ) uniform sampler2D textures[max_textures]; // array of texture samplers

layout (location=0) in vec3 color_vs[];
layout (location=1) in vec2 tex_coords_vs[] ;

layout (location=0) out vec3 color_tsc[] ;
layout (location=1) out vec2 tex_coords_tsc[] ;

void main() 
{
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

    color_tsc[gl_InvocationID] = color_vs[gl_InvocationID] ;
    tex_coords_tsc[gl_InvocationID] = tex_coords_vs[gl_InvocationID] ;

    gl_TessLevelInner[0] = ubo.tsc_inner_level;

    gl_TessLevelOuter[0] = ubo.tsc_outer_level;
    gl_TessLevelOuter[1] = ubo.tsc_outer_level;
    gl_TessLevelOuter[2] = ubo.tsc_outer_level;    
}
)glsl";

// --------------------------------------------------------------------------------
// TESSELLATION EVALUATION SHADER
// --------------------------------------------------------------------------------

const char* teseShaderSrc = R"glsl(
#version 450
layout( triangles, equal_spacing, ccw) in;

// Inputs: push constants block:

layout( push_constant, std430 ) uniform push_constants_block {
    mat4 model_mat ; // model matrix (object to world)
    int  texture_index ; // active texture index, -1 if no texture is active.
    
} pc ;

// Inputs: uniform buffer object:

layout( set=0, binding=0 ) uniform ubo_block {
    mat4 view_mat; // view matrix (world to camera)
    mat4 proj_mat; // projection matrix (camera to clip)
    float tsc_inner_level ; // inner tessellation levels
    float tsc_outer_level ; // outer tessellation levels
} ubo ;

// Inputs: an array of texture samplers (we will use the 'texture_index' push constant to index into this array and select the active texture, in the future when we implement texturing)
const int max_textures = 64 ; // must be equal to 'TexturesSet::max_textures'
layout( set=1, binding=0 ) uniform sampler2D textures[max_textures]; // array of texture samplers


layout (location=0) in vec3 color_tsc[];
layout (location=1) in vec2 tex_coords_tsc[] ;

layout (location=0) out vec3 color_tse ;
layout (location=1) out vec2 tex_coords_tse ;

// float height(vec2 p){
//     return 0.2 * sin(8*p.x) * cos(8*p.y);
// }



vec4 Mix4( vec2 bcc, vec4 v0, vec4 v1, vec4 v2  )
{
    return bcc[0]*v0 + bcc[1]*v1 + (1.0f-bcc[0]-bcc[1])*v2 ;
}

vec3 Mix3( vec2 bcc, vec3 v0, vec3 v1, vec3 v2  )
{
    return bcc[0]*v0 + bcc[1]*v1 + (1.0f-bcc[0]-bcc[1])*v2 ;
}

vec2 Mix2( vec2 bcc, vec2 v0, vec2 v1, vec2 v2  )
{
    return bcc[0]*v0 + bcc[1]*v1 + (1.0f-bcc[0]-bcc[1])*v2 ;
}

void main() {
    vec2 uv = gl_TessCoord.xy;

    vec4 p0 = gl_in[0].gl_Position;
    vec4 p1 = gl_in[1].gl_Position;
    vec4 p2 = gl_in[2].gl_Position;

    gl_Position = Mix4( uv, p0, p1, p2 ) ;

    //pos.z += height(pos.xy);

    color_tse = Mix3( uv, color_tsc[0], color_tsc[1], color_tsc[2] ) ;
    tex_coords_tse = Mix2( uv, tex_coords_tsc[0], tex_coords_tsc[1], tex_coords_tsc[2] ) ;    
}
)glsl";

/// ----------------------------------------------------------------------------------
/// GEOMETRY SHADER
/// ----------------------------------------------------------------------------------

const char *geomShaderSrc = R"glsl(
#version 450
layout( triangles ) in;
layout( triangle_strip, max_vertices = 8 ) out;

// Inputs: push constants block:

layout( push_constant, std430 ) uniform push_constants_block {
    mat4 model_mat ; // model matrix (object to world)
    int  texture_index ; // active texture index, -1 if no texture is active.
    
} pc ;

// Inputs: uniform buffer object:

layout( set=0, binding=0 ) uniform ubo_block {
    mat4 view_mat; // view matrix (world to camera)
    mat4 proj_mat; // projection matrix (camera to clip)
    float tsc_inner_level ; // inner tessellation levels
    float tsc_outer_level ; // outer tessellation levels
} ubo ;

// Inputs: an array of texture samplers (we will use the 'texture_index' push constant to index into this array and select the active texture, in the future when we implement texturing)
const int max_textures = 64 ; // must be equal to 'TexturesSet::max_textures'
layout( set=1, binding=0 ) uniform sampler2D textures[max_textures]; // array of texture samplers

// Inputs from tessellation evaluation shader
layout (location=0) in vec3 color_tse[];
layout (location=1) in vec2 tex_coords_tse[];

// Outputs to fragment shader
layout (location=0) out vec3 color_geo;
layout (location=1) out vec2 tex_coords_geo;

void Passthrough()
{
    for ( int i = 0; i < gl_in.length(); ++i )
    {
        gl_Position = gl_in[i].gl_Position;
        color_geo = color_tse[i];
        tex_coords_geo = tex_coords_tse[i];
        EmitVertex();
    }
    EndPrimitive();
}

// -----------------------------------------------------------------------------------------
// Esta función produce pares de triángulos en cada arista del triángulo de entrada

void EdgesTriangleStrip()
{
    // calcular el centro y los tres vectores desde el centro a los vértices
    vec3 p0 = gl_in[0].gl_Position.xyz ;
    vec3 p1 = gl_in[1].gl_Position.xyz ;
    vec3 p2 = gl_in[2].gl_Position.xyz ;
    vec3 centro = (p0 + p1 + p2) / 3.0 ;
    
    const float g = 0.1 ; // 0.15 ; // grosor relativo de las aristas

    // emitir una tira con 6 triángulos (2 triángulos por arista)
    // para eso se emiten 8 vértices (2 por iteración del bucle)
    //   + primero dos vértices iniciales 
    //   + luego cada uno de los 6 restantes cierra un triángulo
    
    for( int i = 0 ; i < 4 ; i++ )  // en cada iteración se emiten dos vértices
    {
        // j == indice del vértice original en la entrada
        int j = ( i < 3 ) ? i : 0 ;

        vec3 posj = gl_in[j].gl_Position.xyz ;

        // calcular el vértice externo (original) y el interno (nuevo)
        vec4 posic_ext = vec4( posj, 1.0 );
        vec4 posic_int = vec4( centro + (1.0-g)*(posj - centro), 1.0 ); 
        
        
        // emitir vértice interno (nuevo)
        color_geo  = color_tse[j] ;
        tex_coords_geo  = tex_coords_tse[j] ;
        gl_Position = posic_int ; 
        EmitVertex() ;

        // emitir vértice externo (original)
        color_geo  = color_tse[j] ;
        tex_coords_geo  = tex_coords_tse[j] ;
        gl_Position = posic_ext ;
        EmitVertex() ;
    }
    EndPrimitive() ;
}

void main()
{
    //Passthrough();
    EdgesTriangleStrip();
}

)glsl";


/// ----------------------------------------------------------------------------------
/// FRAGMENT SHADER 
/// ----------------------------------------------------------------------------------

const char* fragShaderSrc = R"glsl(
#version 450
    // Inputs: push constants block:

    layout( push_constant, std430 ) uniform push_constants_block {
        mat4 model_mat ; // model matrix (object to world)
        int  texture_index ; // active texture index, -1 if no texture is active.
    } pc ;

    // Inputs: uniform buffer object:

    layout( set=0, binding=0 ) uniform ubo_block {
        mat4 view_mat; // view matrix (world to camera)
        mat4 proj_mat; // projection matrix (camera to clip)
        float tsc_inner_level ; // inner tessellation levels
        float tsc_outer_level ; // outer tessellation levels
    } ubo ;

    // Inputs: an array of texture samplers (we will use the 'texture_index' push constant to index into this array and select the active texture, in the future when we implement texturing)
    const int max_textures = 64 ; // must be equal to 'TexturesSet::max_textures'
    layout( set=1, binding=0 ) uniform sampler2D textures[max_textures]; // array of texture samplers

    // Inputs: varying from vertex shader

    layout (location=0) in vec3 color_geo;
    layout (location=1) in vec2 tex_coords_geo ;

    // Output: fragment color 

    layout (location=0) out vec4 out_color;

    // --------------- 
    // Main function.

    void main()
    {
        if ( pc.texture_index >= 0 ) // if a texture is active, use it to determine the fragment color
             out_color = texture( textures[ pc.texture_index ], tex_coords_geo ) ;
        else // ulse interpolated vertex color
            out_color = vec4( color_geo, 1.0 );
    }
)glsl";

// -----------------------------------------------------------------------------------

namespace vkhc
{

Pipeline2DTess::Pipeline2DTess( VulkanContext & vulkan_context )

:   BasicPipeline( vulkan_context ) 
{
    using namespace std ; 
    cout << "Creating basic 2D pipeline..." << endl ;

    // set metadata about  push constants 
    addPushConstant( "model_mat", sizeof(glm::mat4) ); // model matrix 
    addPushConstant( "texture_index", sizeof(int) ); // active texture index, -1 if no texture is active.
    

    // set metadata about UBO uniforms 
    addUBOUniform( "view_mat", sizeof(glm::mat4) ); // view matrix
    addUBOUniform( "proj_mat", sizeof(glm::mat4) ); // projection matrix
    addUBOUniform( "tsc_inner_level", sizeof(float) ); // inner tessellation levels
    addUBOUniform( "tsc_outer_level", sizeof(float) ); // outer tessellation levels
    
    // set shaders sources 
    shaders_sources = 
    {   .vertex_shader_src       = vertShaderSrc, 
        .tess_control_shader_src = tescShaderSrc,
        .tess_eval_shader_src    = teseShaderSrc,
        .geometry_shader_src     = geomShaderSrc,
        .fragment_shader_src     = fragShaderSrc
    };

    // set attributes formats (must correspond with inputs to the vertex shaders the shaders sources)
    attributes_formats = 
    {   VK_FORMAT_R32G32_SFLOAT,    // position (atrib 0) X,Y
        VK_FORMAT_R32G32B32_SFLOAT, // color (atrib 1) R,G,B
        VK_FORMAT_R32G32_SFLOAT     // texture coords (atrib 2) U,V
    }; // color

    // set default (initial) primitive topology (can be changed dynamically in command buffers)
    //default_primitive_topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST ;
    default_primitive_topology = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST ; // default for tessellation 
    default_vertexes_per_patch = 3 ; // default for tessellation (triangles)


    // initialize the vulkan pipeline  (in the context)
    initialize( ) ; 

    cout << "Basic 2D pipeline created." << endl ;
}
// ------------------------------------------------------------------------------

void Pipeline2DTess::setViewMatrix( const glm::mat4 & view_mat ) 
{
    setUBOUniform( "view_mat", value_ptr( view_mat ) );
}
// ------------------------------------------------------------------------------

void Pipeline2DTess::setProjectionMatrix( const glm::mat4 & proj_mat ) 
{
    setUBOUniform( "proj_mat", value_ptr( proj_mat ) );
}
// ------------------------------------------------------------------------------

void Pipeline2DTess::setTextureIndex( VkCommandBuffer & vk_cmd, int index ) 
{
    setPushConstant( vk_cmd, "texture_index", &index ); 
}
// ------------------------------------------------------------------------------

void Pipeline2DTess::setModelMatrix( VkCommandBuffer & vk_cmd, const glm::mat4 & model_mat ) 
{
    setPushConstant( vk_cmd, "model_mat", value_ptr( model_mat ) ); 
}
// ------------------------------------------------------------------------------


} // end namespace 'vkhc' 


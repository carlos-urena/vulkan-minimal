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
/// Common inputs declarations for all shaders
/// Includes: UBO uniforms, push constants, and texture samplers
/// ----------------------------------------------------------------------------------

static const char* common_decls = R"glsl(

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
    float tsc_outer_level_0 ;   // outer tessellation levels
    float tsc_outer_level_1 ;   // outer tessellation levels
    float tsc_outer_level_2 ;   // outer tessellation levels
} ubo ;

// Inputs: array of texture samplers 
// (we will use the 'texture_index' push constant to index into this array)

const int max_textures = 64 ; // must be equal to 'TexturesSet::max_textures'
layout( set=1, binding=0 ) uniform sampler2D textures[max_textures]; // array of texture samplers

)glsl";



/// ----------------------------------------------------------------------------------
/// VERTEX SHADER 
/// ----------------------------------------------------------------------------------

static const char* vertShaderSrc = R"glsl(
#version 450

//#common_inputs_declarations
    
// Inputs: per vertex attributes from vertex buffers

layout (location=0) in vec2 in_position_occ;
layout (location=1) in vec3 in_color;
layout (location=2) in vec2 in_tex_coords ;

// Outputs: per vertex attributes to next stage 

layout (location=0) out vec3 out_color;
layout (location=1) out vec2 out_tex_coords ;

void main() 
{
    gl_Position =  ubo.proj_mat * ubo.view_mat * pc.model_mat * vec4( in_position_occ, 0.0, 1.0);
    out_color      = in_color ;
    out_tex_coords = in_tex_coords ;
}
)glsl";


// --------------------------------------------------------------------------------
// TESSELLATION CONTROL SHADER
// --------------------------------------------------------------------------------

static const char* tescShaderSrc = R"glsl(
#version 450

// size 3 patches (triangles)
layout(vertices = 3) out;

//#common_inputs_declarations


// Inputs: per vertex attributes from previous stage

layout (location=0) in vec3 in_color[];
layout (location=1) in vec2 in_tex_coords[] ;

// Outputs: per vertex attributes to next stage

layout (location=0) out vec3 out_color[] ;
layout (location=1) out vec2 out_tex_coords[] ;

void main() 
{
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

    out_color[gl_InvocationID] = in_color[gl_InvocationID] ;
    out_tex_coords[gl_InvocationID] = in_tex_coords[gl_InvocationID] ;

    gl_TessLevelInner[0] = ubo.tsc_inner_level;

    gl_TessLevelOuter[0] = ubo.tsc_outer_level_0;
    gl_TessLevelOuter[1] = ubo.tsc_outer_level_1;
    gl_TessLevelOuter[2] = ubo.tsc_outer_level_2;    
}
)glsl";

// --------------------------------------------------------------------------------
// TESSELLATION EVALUATION SHADER
// --------------------------------------------------------------------------------

const char* teseShaderSrc = R"glsl(
#version 450
layout( triangles, equal_spacing, ccw) in;

//#common_inputs_declarations


// Inputs: per vertex attributes from previous stage

layout (location=0) in vec3 in_color[];
layout (location=1) in vec2 in_tex_coords[] ;

// Outputs: per vertex attributes to next stage

layout (location=0) out vec3 out_color ;
layout (location=1) out vec2 out_tex_coords ;

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

    out_color = Mix3( uv, in_color[0], in_color[1], in_color[2] ) ;
    out_tex_coords = Mix2( uv, in_tex_coords[0], in_tex_coords[1], in_tex_coords[2] ) ;    
}
)glsl";

/// ----------------------------------------------------------------------------------
/// GEOMETRY SHADER
/// ----------------------------------------------------------------------------------

const char *geomShaderSrc = R"glsl(
#version 450
layout( triangles ) in;
layout( triangle_strip, max_vertices = 100 ) out;

const int max_vertices = 99 ; // max number of vertices that the geometry shader can emit 
// (must be at least 3 for the triangle itself, plus 6 for the edges, plus n*3 for the discs)
// where 'n' is the number of triangles used to draw each disc


//#common_inputs_declarations


// Inputs: per vertex attributes from previous stage
// (each one is an array with 3 elements, as we are using triangle topology for tessellation)

layout (location=0) in vec3 in_color[];
layout (location=1) in vec2 in_tex_coords[];

// Outputs: per-vertex attributes to next stages

layout (location=0) out vec3 out_color;
layout (location=1) out vec2 out_tex_coords;

// Pass through function (no geometry shader effect, just pass 
// through the vertices from the tessellation evaluation shader 
// to the fragment shader)

// -----------------------------------------------------------------------------------------

void Passthrough()
{
    for ( int i = 0; i < gl_in.length(); ++i )
    {
        gl_Position    = gl_in[i].gl_Position;
        out_color      = in_color[i];
        out_tex_coords = in_tex_coords[i];
        EmitVertex();
    }
    EndPrimitive();
}

// -----------------------------------------------------------------------------------------

const float radio_discos = 0.012f ; // radio de los discos que se dibujan en los vértices
const float grosor_lineas = 0.006f ; // grosor de las líneas

// ----------------------------------------------------------------------------
// Emite un vértice con la posición 'pos_wcc'
//
void NewVertex( vec4 pos, vec4 color )
{
    gl_Position    = pos ;
    out_color      = color.rgb ;
    out_tex_coords = vec2( -2.0f, -2.0f ) ; // do not use texturing for generated geometry
    EmitVertex() ;
}
// ----------------------------------------------------------------------------
// Emite las primitivas (triángulos) que forman un disco de radio w/2 centrado en centro

void EmitDisc( vec4 centro, vec4 color )
{
    // número de triángulos que forman el disco 
    // nt = max_vertices/3 --> numero máximo de triángulos 
    // nt-1
    const int   nt_ed = (max_vertices/3) -1 ; // max number of triangles for edges and discs 
    const int   nt_d  = nt_ed - 2*3 ;          // max number of triangles for discs (6 are for edges)
    const int   num_t  = nt_d/3 ;           // number of triangles for each of the three discs (must be at least 3)
    const float angulo = 2.0f * 3.14159265f / float(num_t) ;
    float       radio  = radio_discos ;

    float f = ubo.proj_mat[0][0] / ubo.proj_mat[1][1] ; 

    vec4 vert_ant  = centro + vec4( f*radio, 0.0f, 0.0f, 0.0f ) ;

    for( int i = 1 ; i <= num_t ; i++ )
    {
        float a        = float(i) * angulo ;
        vec4  vert_nue = centro + vec4( f*radio*cos(a), radio*sin(a), 0.0f, 0.0f ) ;
        
        NewVertex( centro, color );  
        NewVertex( vert_ant, color ); 
        NewVertex( vert_nue, color );
        EndPrimitive();

        vert_ant = vert_nue ;
    }
}

// ----------------------------------------------------------------------------
// Emite la primitiva (una tira de dos triángulos) que forma un segmento de recta de grosor w/2
// desde v0 hasta v1, con colores c0 en v0 y c1 en v1 
// (los colores se interpolan linealmente en el interior de la primitiva) 
//
void EmitSegment( vec4 v0, vec4 v1, vec4 c0, vec4 c1 )
{
    vec4 s  = normalize( v1 - v0 ); // vector director del segmento
    vec4 n  = (grosor_lineas/2.0f)*vec4( -s.y, s.x, 0.0f, 0.0f ); // vector normal al segmento de long w/2

    NewVertex( v0-n, c0 ); NewVertex( v0+n, c0 );  
    NewVertex( v1-n, c1 ); NewVertex( v1+n, c1 );  

    EndPrimitive();
}

// ----------------------------------------------------------------------------
// outputs segments at the edges and discs at the vertexes of the input triangle 

void SegmentsAndDiscs() 
{
    vec4 
        v0 = gl_in[0].gl_Position,
        v1 = gl_in[1].gl_Position,
        v2 = gl_in[2].gl_Position,
        c_seg = vec4( 1.0, 1.0, 1.0, 1.0 ) , //v1_color[0] ,
        c_dis = vec4( 1.0, 0.2, 0.2, 1.0 ) , //v1_color[2] ,
        //c1 = v1_color[1] ,
        dz = vec4( 0.0f, 0.0f, +0.05f, 0.0f ) ; // desplazamiento en z para evitar z-fighting
    
    
    
    EmitSegment( v0+dz, v1+dz, c_seg, c_seg );
    EmitSegment( v1+dz, v2+dz, c_seg, c_seg );
    EmitSegment( v2+dz, v0+dz, c_seg, c_seg );
    
    EmitDisc( v0+2.0*dz, c_dis ); 
    EmitDisc( v1+2.0*dz, c_dis );
    EmitDisc( v2+2.0*dz, c_dis );
}

void main()
{
    Passthrough();
    SegmentsAndDiscs();
}

)glsl";


/// ----------------------------------------------------------------------------------
/// FRAGMENT SHADER 
/// ----------------------------------------------------------------------------------

const char* fragShaderSrc = R"glsl(
#version 450
    
//#common_inputs_declarations

// Inputs: per-vertex attributes from previous stage

layout (location=0) in vec3 in_color;
layout (location=1) in vec2 in_tex_coords;

// Output: fragment color 

layout (location=0) out vec4 out_color;

// --------------- 
// Main function.

void main()
{
    if ( pc.texture_index >= 0 && in_tex_coords.s >= -0.01 ) // if a texture is active, use it to determine the fragment color
            out_color = texture( textures[ pc.texture_index ], in_tex_coords ) ;
    else // ulse interpolated vertex color
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
    vertShaderSrc_full = processShaderSource( vertShaderSrc ),
    tescShaderSrc_full = processShaderSource( tescShaderSrc ),
    teseShaderSrc_full = processShaderSource( teseShaderSrc ),
    geomShaderSrc_full = processShaderSource( geomShaderSrc ),
    fragShaderSrc_full = processShaderSource( fragShaderSrc );

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
    
    addUBOUniform( "tsc_inner_level", sizeof(float) ); // outer tessellation levels
    addUBOUniform( "tsc_outer_level_0", sizeof(float) ); // inner tessellation levels
    addUBOUniform( "tsc_outer_level_1", sizeof(float) ); // inner tessellation levels
    addUBOUniform( "tsc_outer_level_2", sizeof(float) ); // inner tessellation levels


    // set shaders sources 
    shaders_sources = 
    {   .vertex_shader_src       = & vertShaderSrc_full, 
        .tess_control_shader_src = & tescShaderSrc_full,
        .tess_eval_shader_src    = & teseShaderSrc_full,
        .geometry_shader_src     = & geomShaderSrc_full,
        .fragment_shader_src     = & fragShaderSrc_full
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


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
    float tsc_inner_level_0 ;   // inner tessellation levels (0)
    float tsc_inner_level_1 ;   // inner tessellation levels (1, only for quads)
    float tsc_outer_level_0 ;   // outer tessellation levels
    float tsc_outer_level_1 ;   // outer tessellation levels
    float tsc_outer_level_2 ;   // outer tessellation levels
    float tsc_outer_level_3 ;   // outer tessellation levels (only for quads)
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
// TESSELLATION CONTROL SHADER (for triangle patches)
// --------------------------------------------------------------------------------

static const char* tc_shader_triangles_src = R"glsl(
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

    gl_TessLevelInner[0] = ubo.tsc_inner_level_0;

    gl_TessLevelOuter[0] = ubo.tsc_outer_level_0;
    gl_TessLevelOuter[1] = ubo.tsc_outer_level_1;
    gl_TessLevelOuter[2] = ubo.tsc_outer_level_2;    
}
)glsl";

// --------------------------------------------------------------------------------
// TESSELLATION CONTROL SHADER (for quad patches)
// --------------------------------------------------------------------------------

static const char* tc_shader_quads_src = R"glsl(
#version 450

// size 4 patches (quads)
layout(vertices = 4) out;

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

    gl_TessLevelInner[0] = ubo.tsc_inner_level_0;
    gl_TessLevelInner[1] = ubo.tsc_inner_level_1;

    gl_TessLevelOuter[0] = ubo.tsc_outer_level_0;
    gl_TessLevelOuter[1] = ubo.tsc_outer_level_1;
    gl_TessLevelOuter[2] = ubo.tsc_outer_level_2;    
    gl_TessLevelOuter[3] = ubo.tsc_outer_level_3;    
}
)glsl";



// --------------------------------------------------------------------------------
// TESSELLATION EVALUATION SHADER (for triangle patches)
// --------------------------------------------------------------------------------

const char* tev_shader_triangles_src = R"glsl(
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


// --------------------------------------------------------------------------------
// TESSELLATION EVALUATION SHADER (for quads patches)
// --------------------------------------------------------------------------------

const char* tev_shader_quads_src = R"glsl(
#version 450
layout( quads, equal_spacing, ccw) in;

//#common_inputs_declarations


// Inputs: per vertex attributes from previous stage

layout (location=0) in vec3 in_color[];
layout (location=1) in vec2 in_tex_coords[] ;

// Outputs: per vertex attributes to next stage

layout (location=0) out vec3 out_color ;
layout (location=1) out vec2 out_tex_coords ;

vec4 Mix4( vec2 uv, vec4 v00, vec4 v01, vec4 v10, vec4 v11  )
{
    float u = uv[0] ;
    float v = uv[1] ;
    return (1.0-u)*(1.0-v)*v00 + (1.0-u)*v*v01 + u*(1.0-v)*v10 + u*v*v11 ;
}

vec3 Mix3( vec2 uv, vec3 v00, vec3 v01, vec3 v10, vec3 v11  )
{
    float u = uv[0] ;
    float v = uv[1] ;
    return (1.0-u)*(1.0-v)*v00 + (1.0-u)*v*v01 + u*(1.0-v)*v10 + u*v*v11 ;
}

vec2 Mix2( vec2 uv, vec2 v00, vec2 v01, vec2 v10, vec2 v11  )
{
    float u = uv[0] ;
    float v = uv[1] ;
    return (1.0-u)*(1.0-v)*v00 + (1.0-u)*v*v01 + u*(1.0-v)*v10 + u*v*v11 ;
}

void main() {
    vec2 uv = gl_TessCoord.xy;

    vec4 p00 = gl_in[0].gl_Position; // 0,0
    vec4 p10 = gl_in[1].gl_Position; // 1,0
    vec4 p11 = gl_in[2].gl_Position; // 1,1
    vec4 p01 = gl_in[3].gl_Position; // 0,1

    gl_Position = Mix4( uv, p00, p01, p10, p11 ) ;

    //pos.z += height(pos.xy);

    out_color = Mix3( uv, in_color[0], in_color[3], in_color[1], in_color[2] ) ;
    out_tex_coords = Mix2( uv, in_tex_coords[0], in_tex_coords[3], in_tex_coords[1], in_tex_coords[2] ) ;    
}
)glsl";

/// ----------------------------------------------------------------------------------
/// GEOMETRY SHADER
/// ----------------------------------------------------------------------------------

const char *geom_shader_src = R"glsl(
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

const float radio_discos = 0.012f ; // radius of the discs drawn at the vertices
const float grosor_lineas = 0.006f ; // line thickness

// ----------------------------------------------------------------------------
// Emits a vertex at position 'pos'
//
void NewVertex( vec4 pos, vec4 color )
{
    gl_Position    = pos ;
    out_color      = color.rgb ;
    out_tex_coords = vec2( -2.0f, -2.0f ) ; // do not use texturing for generated geometry
    EmitVertex() ;
}
// ----------------------------------------------------------------------------
// Emits the primitives (triangles) that form a disc of radius w/2 centered at 'center'

void EmitDisc( vec4 centro, vec4 color )
{
    // number of triangles that form the disc
    // nt = max_vertices/3 --> maximum number of triangles
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
// Emits the primitive (a strip of two triangles) that forms a line segment with thickness w/2
// from v0 to v1, with color c0 at v0 and c1 at v1
// (the colors are linearly interpolated inside the primitive)
//
void EmitSegment( vec4 v0, vec4 v1, vec4 c0, vec4 c1 )
{
    vec4 s  = normalize( v1 - v0 ); // segment direction vector (normalized)
    vec4 n  = (grosor_lineas/2.0f)*vec4( -s.y, s.x, 0.0f, 0.0f ); // vector perpendicular to the segment, with length equal to half the thickness of the line

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
        dz = vec4( 0.0f, 0.0f, +0.05f, 0.0f ) ; // z offset to avoid z-fighting
    
    
    
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

const char* frag_shader_src = R"glsl(
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
    vert_shader_src_full = processShaderSource( vert_shader_src ),
    tc_shader_triangles_full = processShaderSource( tc_shader_triangles_src ),
    tev_shader_triangles_full = processShaderSource( tev_shader_triangles_src ),
    tc_shader_quads_full = processShaderSource( tc_shader_quads_src ),
    tev_shader_quads_full = processShaderSource( tev_shader_quads_src ),
    geom_shader_src_full = processShaderSource( geom_shader_src ),
    frag_shader_src_full = processShaderSource( frag_shader_src );

Pipeline2DTess::Pipeline2DTess( VulkanContext & vulkan_context,
                                const int p_num_vertexes_per_patch,
                                bool p_depth_test_enabled,
                                bool p_depth_write_enabled,
                                VkCompareOp p_depth_compare_op )

:   BasicPipeline( vulkan_context ) 
{
    using namespace std ; 
    cout << "Creating basic 2D pipeline..." << endl ;

    cout << "num_vertexes_per_patch = " << p_num_vertexes_per_patch << endl ;
    num_vertexes_par_patch = p_num_vertexes_per_patch ;
    Assert( num_vertexes_par_patch == 3 || num_vertexes_par_patch == 4, 
            "num. of vertexes per patch must be 3 for triangles or 4 for quads" ) ;

    // set metadata about  push constants 
    addPushConstant( "model_mat", sizeof(glm::mat4) ); // model matrix 
    addPushConstant( "texture_index", sizeof(int) ); // active texture index, -1 if no texture is active.
    

    // set metadata about UBO uniforms 
    addUBOUniform( "view_mat", sizeof(glm::mat4) ); // view matrix
    addUBOUniform( "proj_mat", sizeof(glm::mat4) ); // projection matrix
    
    addUBOUniform( "tsc_inner_level_0", sizeof(float) ); // inner tessellation levels
    addUBOUniform( "tsc_inner_level_1", sizeof(float) ); // inner tessellation levels

    addUBOUniform( "tsc_outer_level_0", sizeof(float) ); // outer tessellation levels
    addUBOUniform( "tsc_outer_level_1", sizeof(float) ); // outer tessellation levels
    addUBOUniform( "tsc_outer_level_2", sizeof(float) ); // outer tessellation levels
    addUBOUniform( "tsc_outer_level_3", sizeof(float) ); // outer tessellation levels


    // set shaders sources 
    if ( p_num_vertexes_per_patch == 3 )
    {
        shaders_sources = 
        {   .vertex_shader_src       = & vert_shader_src_full, 
            .tess_control_shader_src = & tc_shader_triangles_full,
            .tess_eval_shader_src    = & tev_shader_triangles_full,
            .geometry_shader_src     = & geom_shader_src_full,
            .fragment_shader_src     = & frag_shader_src_full
        };
    }
    else // quads
    {
        shaders_sources = 
        {   .vertex_shader_src       = & vert_shader_src_full, 
            .tess_control_shader_src = & tc_shader_quads_full,
            .tess_eval_shader_src    = & tev_shader_quads_full,
            .geometry_shader_src     = & geom_shader_src_full,
            .fragment_shader_src     = & frag_shader_src_full
        };
    }

    

    // set attributes formats (must correspond with inputs to the vertex shaders the shaders sources)
    attributes_formats = 
    {   VK_FORMAT_R32G32_SFLOAT,    // position (attrib 0) X,Y
        VK_FORMAT_R32G32B32_SFLOAT, // color (attrib 1) R,G,B
        VK_FORMAT_R32G32_SFLOAT     // texture coords (attrib 2) U,V
    }; // color

    // set default (initial) primitive topology (can be changed dynamically in command buffers)
    //default_primitive_topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST ;
    default_primitive_topology = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST ; // default for tessellation 
    default_vertexes_per_patch = num_vertexes_par_patch ; // default for tessellation (triangles)

    // Depth behavior is caller-configurable through constructor params.
    depth_test_enabled = p_depth_test_enabled;
    depth_write_enabled = p_depth_write_enabled;
    depth_compare_op = p_depth_compare_op;


    // initialize the vulkan pipeline  (in the context)
    initialize( ) ; 

    cout << "Pipeline 2D with tessellation created." << endl ;
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


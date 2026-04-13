#include <iostream>
#include <vector>
#include <cmath>
#include <chrono>

#include <vulkan-context.h>
#include <pipeline2D.h>
#include <pipeline2D_tess.h>
#include <vertex-array.h>
#include <imgui-context.h>
#include <textures.h>

// -------------------------------------------------------------------------------  
// class 'Triangle' (a 'vertex-array' like object )

class Triangle : public vkhc::VertexArray 
{
    public: 
    
    inline Triangle( vkhc::VulkanContext & vulkan_context)
    :   VertexArray( vulkan_context, VK_PRIMITIVE_TOPOLOGY_PATCH_LIST ) // VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST )
    {
        using namespace glm ;
        using namespace std ;
        const float 
            r  = 1.0f ,          // triangle radius (distance from the center of the triangle to its vertices)
            a0 = M_PI , // initial angle of the first vertex (in radians), the other vertices will be at angles a0 + 2*pi/3 and a0 + 4*pi/3, so that the triangle is equilateral and one vertex is pointing upwards.
            a  = M_PI*2.0f/3.0f ;  // angle between vertices (in radians), for an equilateral triangle this is 2*pi/3

        addAttribData( vector<vec2>  // location 0: vertex positions
        {   { r*cos( a0 ),          r*sin( a0 ) }, 
            { r*cos( a0 + a ),      r*sin( a0 + a ) }, 
            { r*cos( a0 + 2.0f*a ), r*sin( a0 + 2.0f*a ) },
        });
        // location 1: vertex colors
        addAttribData( vector<vec3>{ {1.0f,0.0f,0.0f}, {0.0f,1.0f,0.0f}, {0.0f,0.0f,1.0f} });
        // location 2: vertex texture coordinates 
        addAttribData( vector<vec2>{ {0.0f,0.0f}, {0.5f,1.0f}, {1.0f,0.0f} });
        // indexes 
        setIndexData( vector<uvec3>{{ 0, 1, 2 }} ); 
    }
} ;

// ----------------------------------------------------------------------------------

using namespace std::chrono ;

 // type for durations in seconds (used for frame time and animation speed)
typedef duration<float,std::ratio<1,1>> seconds_f ;

steady_clock::time_point prev_frame_start ;

void InitFrameStart()
{
    prev_frame_start = steady_clock::now() ;
}

seconds_f NextFrameStart()
{
    // compute delay (in seconds) from previous frame start in 'frame_time_s'
    steady_clock::time_point curr_frame_start = steady_clock::now() ;
    seconds_f frame_time_s = curr_frame_start - prev_frame_start  ;
    prev_frame_start = curr_frame_start ;
    return frame_time_s ;
}

// ----------------------------------------------------------------------------------


float curr_angle_rad = M_PI/2.0f ;  // current angle in radians
float rotation_speed = 0.0f ; // angular speed in cycles per second 
float triangle_scale = 0.8f ;

// tessellation levels (for the pipeline with tessellation shaders)

const int max_tess_level = 20 ;

int tsc_inner_level_int = 4 ;
int tsc_outer_level_int[3] = { 
    tsc_inner_level_int, 
    tsc_inner_level_int, 
    tsc_inner_level_int } ;


float tsc_inner_level = float(tsc_inner_level_int) ;
float tsc_outer_level[3] = { 
    float(tsc_outer_level_int[0]), 
    float(tsc_outer_level_int[1]), 
    float(tsc_outer_level_int[2]) 
} ;

bool close_requested = false ; 

uint32_t  image_index ; // index for image in use (from the swap-chain)
int       texture_index = -1 ;  // active texture index (-1 for none)

// model matrix and its parameters for animation 

glm::mat4 model_mat ;            // model matrix passed to the pipeline via a push constant
glm::mat4 view_mat = glm::mat4(1.0f); // view matrix passed via UBO
glm::mat4 proj_mat = glm::mat4(1.0f) ; // projection matrix passed via UBO

// ----------------------------------------------------------------------------------


void UpdateViewProjMats( vkhc::VulkanContext & context,  seconds_f frame_time_s )
{
    using namespace glm ;

    // sets the model matrix and update angle
    model_mat = scale( vec3( triangle_scale, triangle_scale, 1.0f))*
                rotate( curr_angle_rad, vec3( 0.0f, 0.0f, 1.0f ) );
    curr_angle_rad += rotation_speed * frame_time_s.count() * 2.0f * M_PI ; // increase angle

    // set the projection matrix (so that objects are not deformed whatever the aspect ratio is)
    const uvec2 ra_ext = context.getRenderAreaExtent(); // render area extent (size of the render area left to GUI, in pixels)
    const float ayx    = float(ra_ext.y) / float(ra_ext.x) ; // aspect ratio (height/width) of the render area
    proj_mat = scale( vec3( std::min(1.0f, ayx), std::min(1.0f, 1.0f/ayx), 1.0f ) ) ; 
    
    //pipeline.setViewMatrix( view_mat ) ;
    //pipeline.setProjectionMatrix( proj_mat ) ;
}

// ----------------------------------------------------------------------------------

void DrawIMGUIWidgets( VkCommandBuffer & cmd, vkhc::VulkanContext & context,  vkhc::Pipeline2DTess & pipeline ) 
{
    using namespace ImGui ;
    context.beginIMGUIFrame( cmd ) ;
        if ( Button("Close window" ) ) close_requested = true ;
        if (CollapsingHeader("Triangle controls", ImGuiTreeNodeFlags_DefaultOpen))
        {       
            SliderFloat("Speed", &rotation_speed, 0.0f, 3.0f);
            SliderFloat("Scale", &triangle_scale, 0.2f, 2.0f);
            if ( SliderInt("Tess. inner level", &tsc_inner_level_int, 1, max_tess_level) )
                tsc_inner_level = float(tsc_inner_level_int) ;
            
            for ( int i = 0 ; i < 3 ; i++ )
            {
                const std::string label = "Tess. outer level " + std::to_string(i),
                                    ident = "tsc_outer_level_" + std::to_string(i) ;

                if ( SliderInt( label.c_str(), &tsc_outer_level_int[i], 1, max_tess_level) )
                {   tsc_outer_level[i] = float(tsc_outer_level_int[i]) ;
                    pipeline.setUBOUniform( ident.c_str(), &tsc_outer_level[i] ) ;
                }
            }    
            int texture_combo_index = texture_index + 1 ; // map -1..3 to 0..4 for ImGui combo
            if ( Combo("Texture", &texture_combo_index, "No texture (vert. colors)\0Wood 1\0Wood 2\0Wood 3\0Procedural texture\0") )
                texture_index = texture_combo_index - 1 ;
        }
        Text("FPS: %.1f (%.1f ms)", ImGui::GetIO().Framerate, 1000.0f / ImGui::GetIO().Framerate);
    context.endIMGUIFrame( cmd );
}

//  ------------------------------------------------------------------------------

class ExampleTexturesSet : public vkhc::TexturesSet
{
    public:
    ExampleTexturesSet( vkhc::VulkanContext * p_context ) : TexturesSet( p_context ) 
    {
        add( "../assets/wood-1.png" );
        add( "../assets/wood-2.png" );
        add( "../assets/wood-3.png" );
        add( new vkhc::ProceduralTexture1( context ) ) ;
    }
} ;

// ----------------------------------------------------------------------------------

int main() 
{
    using namespace std ;
    using namespace glm ;
    
    using namespace vkhc ;
    using namespace std::chrono ;

    VulkanContext      context{ 1024, 512, "Vulkan Triangle" } ;
    Pipeline2DTess     pipeline{ context } ;
    Triangle           triangle{ context } ;
    ExampleTexturesSet textures_set{ &context } ;   
    VkClearValue       clear_color{ .color ={ .float32 ={ 0.0f, 0.0f, 0.0f, 1.0f }}};
    VkCommandBuffer    cmd ;
    
    textures_set.bindTo( pipeline ) ; // bind the textures set to the pipeline

    InitFrameStart();

    // enter the main loop
    while ( ! context.windowShouldClose() && ! close_requested )  
    {
        seconds_f frame_time_s = NextFrameStart() ; // compute delay (in seconds) from previous frame start 
        context.pollEvents();  // process pending events 

        UpdateViewProjMats( context, frame_time_s ) ; // updates 'view_mat' and 'proj_mat' 
        pipeline.setViewMatrix( view_mat ) ;
        pipeline.setProjectionMatrix( proj_mat ) ;

        pipeline.setUBOUniform( "tsc_inner_level", &tsc_inner_level ) ;
        pipeline.setUBOUniform( "tsc_outer_level_0", &tsc_outer_level[0] ) ;
        pipeline.setUBOUniform( "tsc_outer_level_1", &tsc_outer_level[1] ) ;
        pipeline.setUBOUniform( "tsc_outer_level_2", &tsc_outer_level[2] ) ;

        // begin frame: acquire an image from the swap chain, and get its corresponding command buffer
        if ( ! context.beginFrame( clear_color, cmd, image_index ) ) 
            continue ; 
        
        // activate the pipeline and sets the viewport
        pipeline.bind( cmd );
        context.setRenderAreaViewport( cmd ) ;

        // give initial value to the push constants 
        pipeline.setModelMatrix( cmd, model_mat ) ;
        pipeline.setTextureIndex( cmd, texture_index ) ;

        // draw the triangle and the widgets 
        triangle.draw( cmd );
        DrawIMGUIWidgets( cmd, context, pipeline ) ;

        // done, now end frame: submit the command queue and present the image.
        context.endFrame( cmd, image_index ) ;
    }
    context.waitDeviceIdle() ; 
    return 0 ;
}
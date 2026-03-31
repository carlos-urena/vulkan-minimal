#include <iostream>
#include <vector>
#include <cmath>
#include <chrono>

#include <vulkan-context.h>
#include <pipeline2D.h>
#include <vertex-array.h>
#include <imgui-context.h>
#include <textures.h>

 // type for durations in seconds (used for frame time and animation speed)
typedef std::chrono::duration<float,std::ratio<1,1>> seconds_f ;

int main() 
{
    using namespace std ;
    using namespace glm ;
    using namespace ImGui ;
    using namespace vkhc ;
    using namespace std::chrono ;

    VulkanContext     context{ 1024, 512, "Vulkan Triangle" } ;
    BasicPipeline2D   pipeline{ context } ; 
    VertexArray       vertex_array{ context, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST } ;
    TexturesSet       textures_set{ &context } ;   
    VkClearValue      clear_color{ .color ={ .float32 ={ 0.0f, 0.0f, 0.0f, 1.0f }}};
    VkCommandBuffer   cmd ;
    uint32_t          image_index ;

    const float r  = 1.0f ;          // triangle radius (distance from the center of the triangle to its vertices)
    const float a0 = M_PI/2.0 ;      // initial angle of the first vertex (in radians), the other vertices will be at angles a0 + 2*pi/3 and a0 + 4*pi/3, so that the triangle is equilateral and one vertex is pointing upwards.
    const float a  = M_PI*2.0/3.0 ;  // angle between vertices (in radians), for an equilateral triangle this is 2*pi/3

    // location 0: vertex positions
    vertex_array.addAttribData( vector<vec2> {  
        { r*cos( a0 ),          r*sin( a0 ) }, 
        { r*cos( a0 + a ),      r*sin( a0 + a ) }, 
        { r*cos( a0 + 2.0f*a ), r*sin( a0 + 2.0f*a ) },
    });

    // location 1: vertex colors
    vertex_array.addAttribData( vector< vec3 > {  
        { 1.0f, 0.0f, 0.0f }, 
        { 0.0f, 1.0f, 0.0f }, 
        { 0.0f, 0.0f, 1.0f }, 
    });

    // location 2: vertex texture coordinates 
    vertex_array.addAttribData( vector<vec2> {  
        { 0.0f, 0.0f }, 
        { 0.5f, 1.0f }, 
        { 1.0f, 0.0f }, 
    });

    // indexes
    vertex_array.setIndexData( std::vector< uvec3 > { 
         { 0, 1, 2 }, 
    });

    // model matrix and its parameters for animation 

    mat4 model_mat ;            // model matrix passed to the pipeline via a push constant
    mat4 view_mat = mat4(1.0f); // view matrix passed via UBO
    mat4 proj_mat = mat4(1.0f) ; // projection matrix passed via UBO

    float curr_angle_rad = M_PI/2.0f ;  // current angle in radians
    float rotation_speed = 1.5f ; // angular speed in cycles per second 
    float triangle_scale = 1.0f ;

    // create texture objects, and bnd the textures to the pipeline
    textures_set.add( "../assets/wood-1.png" );
    textures_set.add( "../assets/wood-2.png" );
    textures_set.add( "../assets/wood-3.png" );
    textures_set.add( new ProceduralTexture1( &context ) ) ;
    
    textures_set.bindTo( pipeline ) ;
    
    // get previous frame start time 
    steady_clock::time_point prev_frame_start = steady_clock::now() ;

    // active texture index (-1 for none)
    int texture_index = -1 ; 

    bool close_requested = false ; 

    // enter the main loop
    while ( ! context.windowShouldClose() && ! close_requested )  
    {
        // compute delay (in seconds) from previous frame start in 'frame_time_s'
        steady_clock::time_point curr_frame_start = steady_clock::now() ;
        seconds_f frame_time_s = curr_frame_start - prev_frame_start  ;
        prev_frame_start = curr_frame_start ;

        // process pending events 
        context.pollEvents(); 

        // sets the model matrix and update angle
        model_mat = scale( vec3( triangle_scale, triangle_scale, 1.0f))*
                    rotate( curr_angle_rad, vec3( 0.0f, 0.0f, 1.0f ) );
        curr_angle_rad += rotation_speed * frame_time_s.count() * 2.0f * M_PI ; // increase angle

        // set the projection matrix (so that objects are not deformed whatever the aspect ratio is)
        const uvec2 ra_ext = context.getRenderAreaExtent(); // render area extent (size of the render area left to GUI, in pixels)
        const float ayx    = float(ra_ext.y) / float(ra_ext.x) ; // aspect ratio (height/width) of the render area
        proj_mat = scale( vec3( std::min(1.0f, ayx), std::min(1.0f, 1.0f/ayx), 1.0f ) ) ; 

        // Gives values to UBO uniforms 
        pipeline.setViewMatrix( view_mat ) ;
        pipeline.setProjectionMatrix( proj_mat ) ;
        
        // begin frame: acquire an image from the swap chain, and get its corresponding command buffer
        if ( ! context.beginFrame( clear_color, cmd, image_index ) ) 
            continue ; 
        
        // activate the pipeline and sets the viewport
        pipeline.bind( cmd );
        context.setRenderAreaViewport( cmd ) ;

        // give initial value to the push constants 
        pipeline.setModelMatrix( cmd, model_mat ) ;
        pipeline.setTextureIndex( cmd, texture_index ) ;
    
        // draw the triangle
        vertex_array.draw( cmd ); 

        // draw IMGUI widgets
        context.beginIMGUIFrame( cmd ) ;
            if ( Button("Close window" ) ) close_requested = true ;
            if (CollapsingHeader("Triangle controls", ImGuiTreeNodeFlags_DefaultOpen))
            {       
                SliderFloat("Speed", &rotation_speed, 0.0f, 3.0f);
                SliderFloat("Scale", &triangle_scale, 0.2f, 2.0f);
                int texture_combo_index = texture_index + 1 ; // map -1..3 to 0..4 for ImGui combo
                if ( Combo("Texture", &texture_combo_index, "No texture (vert. colors)\0Wood 1\0Wood 2\0Wood 3\0Procedural texture\0") )
                    texture_index = texture_combo_index - 1 ;
            }
            Text("FPS: %.1f (%.1f ms)", ImGui::GetIO().Framerate, 1000.0f / ImGui::GetIO().Framerate);
        context.endIMGUIFrame( cmd );
        
        // done, now end frame: submit the command queue and present the image.
        context.endFrame( cmd, image_index ) ;
    }
    context.waitDeviceIdle() ; 
    return 0 ;
}
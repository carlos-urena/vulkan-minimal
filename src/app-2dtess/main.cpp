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

#include <application.h>

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
            r  = 1.0f , // triangle radius (distance from the center of the triangle to its vertices)
            a0 = M_PI , // initial angle of the first vertex (in radians), the other vertices will be at angles a0 + 2*pi/3 and a0 + 4*pi/3, so that the triangle is equilateral and one vertex is pointing upwards.
            a  = M_PI*2.0f/3.0f ;  // angle between vertices (in radians), for an equilateral triangle this is 2*pi/3

        // location 0: vertex positions
        addAttribData( vector<vec2>{ {r*cos(a0),r*sin(a0)}, {r*cos(a0+a),r*sin(a0+a)}, 
                                     {r*cos(a0+2.0f*a),r*sin(a0+2.0f*a)}  });
        // location 1: vertex colors
        addAttribData( vector<vec3>{ {1.0f,0.0f,0.0f}, {0.0f,1.0f,0.0f}, {0.0f,0.0f,1.0f} });
        // location 2: vertex texture coordinates 
        addAttribData( vector<vec2>{ {0.0f,0.0f}, {0.5f,1.0f}, {1.0f,0.0f} });
        // indexes 
        setIndexData( vector<uvec3>{{ 0, 1, 2 }} ); 
    }
} ;

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


class App2DTess : public ilc::Application
{

    private:

    // parameters for the triangle model matrix and animation
    float curr_angle_rad = M_PI/2.0f ;  // current angle in radians
    float rotation_speed = 0.0f ; // angular speed in cycles per second 
    float triangle_scale = 0.8f ;

    // max value for the tessellation levels (for the GUI sliders)
    const int max_tess_level = 20 ;

    // tessellation levels (int for the GUI, and float for the pipeline)
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

    // active texture index (-1 for none)
    int texture_index = -1 ;  

    // model, view and projection matrices
    glm::mat4 model_mat ;            // model matrix passed to the pipeline via a push constant
    glm::mat4 view_mat = glm::mat4(1.0f); // view matrix passed via UBO
    glm::mat4 proj_mat = glm::mat4(1.0f) ; // projection matrix passed via UBO

    // triangle object which is visualized
    Triangle *  triangle = nullptr ; 

    // tessellation pipeline 
    vkhc::Pipeline2DTess * pipeline = nullptr ; 

    // textures set (used for testing textures).
    ExampleTexturesSet * textures_set = nullptr ; 


    // -----------------------------------------------------------------------------
    // Methods:
    
    public:

    App2DTess( ) ;

    // override methods
    void initFrame( const vkhc::seconds_f  time_elapsed ) override ;
    void drawFrame( VkCommandBuffer & cmd ) override ;
    void drawIMGUIWidgets( VkCommandBuffer & cmd ) override ;
    virtual ~App2DTess()  override ; 

    // specific methods for this application (not overrides)
    void updateViewProjMats( vkhc::VulkanContext & context, vkhc::seconds_f frame_time_s ) ;
} ;

// ----------------------------------------------------------------------------------


App2DTess::App2DTess( ) 

:   Application( 1024, 512, "Vulkan Tessellation demo" ) 
{
    using namespace vkhc ; 

    Assert( context != nullptr, "Tess1App constructor: 'context' instance is null !!" );
    
    triangle     = new Triangle( *context ) ;             assert( triangle != nullptr ) ;
    textures_set = new ExampleTexturesSet( context ) ;    assert( textures_set != nullptr ) ;
    pipeline     = new vkhc::Pipeline2DTess( *context ) ; assert( pipeline != nullptr ) ;

    textures_set->bindTo( *pipeline ) ; // bind the textures set to the pipeline, so that its textures can be used in the fragment shader.

} ;

// ----------------------------------------------------------------------------------

App2DTess::~App2DTess() 
{
    //Assert( context != nullptr, "Tess1App destructor: 'context' instance is null !!" );
    delete triangle ; triangle = nullptr ;
    delete pipeline ; pipeline = nullptr ;
    delete textures_set ; textures_set = nullptr ;

    std::cout << "Tess1App deleted" << std::endl ;
}

// ----------------------------------------------------------------------------------


void App2DTess::updateViewProjMats( vkhc::VulkanContext & context,  vkhc::seconds_f frame_time_s )
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
}
// ----------------------------------------------------------------------------------

void App2DTess::drawIMGUIWidgets( VkCommandBuffer & cmd ) 
{
    using namespace ImGui ;
    
    if ( Button("Close window" ) ) close_requested = true ;
    if (CollapsingHeader("Triangle controls", ImGuiTreeNodeFlags_DefaultOpen))
    {       
        SliderFloat("Speed", &rotation_speed, 0.0f, 3.0f);
        SliderFloat("Scale", &triangle_scale, 0.2f, 2.0f);
        if ( SliderInt("Tess. inner level", &tsc_inner_level_int, 1, max_tess_level) )
            tsc_inner_level = float(tsc_inner_level_int) ;
        
        for ( int i = 0 ; i < 3 ; i++ )
        {
            const std::string 
                label = "Tess. outer level " + std::to_string(i),
                ident = "tsc_outer_level_" + std::to_string(i) ;

            if ( SliderInt( label.c_str(), &tsc_outer_level_int[i], 1, max_tess_level) )
            {   tsc_outer_level[i] = float(tsc_outer_level_int[i]) ;
                pipeline->setUBOUniform( ident.c_str(), &tsc_outer_level[i] ) ;
            }
        }    
        int texture_combo_index = texture_index + 1 ; // map -1..3 to 0..4 for ImGui combo
        if ( Combo("Texture", &texture_combo_index, "No texture (vert. colors)\0Wood 1\0Wood 2\0Wood 3\0Procedural texture\0") )
            texture_index = texture_combo_index - 1 ;
    }
    Text("FPS: %.1f (%.1f ms)", ImGui::GetIO().Framerate, 1000.0f / ImGui::GetIO().Framerate);
}
// ----------------------------------------------------------------------------------

void App2DTess::initFrame( const vkhc::seconds_f  time_elapsed )
{
    Assert( context != nullptr, "Tess1App::drawFrame: 'context' instance is null !!" );
    Assert( pipeline != nullptr, "Tess1App::drawFrame: 'pipeline' instance is null !!" );
    Assert( triangle != nullptr, "Tess1App::drawFrame: 'triangle' instance is null !!" );

    // update UBO uniforms in the pipeline
    updateViewProjMats( *context, time_elapsed ) ; // updates 'view_mat' and 'proj_mat' 
    pipeline->setViewMatrix( view_mat ) ;
    pipeline->setProjectionMatrix( proj_mat ) ;

    pipeline->setUBOUniform( "tsc_inner_level", &tsc_inner_level ) ;
    pipeline->setUBOUniform( "tsc_outer_level_0", &tsc_outer_level[0] ) ;
    pipeline->setUBOUniform( "tsc_outer_level_1", &tsc_outer_level[1] ) ;
    pipeline->setUBOUniform( "tsc_outer_level_2", &tsc_outer_level[2] ) ;
    
    
}
// ----------------------------------------------------------------------------------

void App2DTess::drawFrame( VkCommandBuffer & cmd ) 
{
    Assert( context != nullptr, "Tess1App::drawFrame: 'context' instance is null !!" );
    Assert( pipeline != nullptr, "Tess1App::drawFrame: 'pipeline' instance is null !!" );
    Assert( triangle != nullptr, "Tess1App::drawFrame: 'triangle' instance is null !!" );
   
    // activate the pipeline and sets the viewport
    pipeline->bind( cmd );
    // context->setRenderAreaViewport( cmd ) ; /// THIS IS CALLED after 'initFrame' and before 'drawFrame' (does it works?)

    // give initial values to the push constants at the begining of 'cmd'
    pipeline->setModelMatrix( cmd, model_mat ) ;
    pipeline->setTextureIndex( cmd, texture_index ) ;

    // draw the triangle and the widgets 
    triangle->draw( cmd );
}


// end of class 'Tess1App'
// *********************************************************************************

int main() 
{
    App2DTess app{  } ;
    app.run() ;
    return 0 ;
}


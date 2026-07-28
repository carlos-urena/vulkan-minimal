#include <iostream>
#include <vector>
#include <cmath>
#include <chrono>

#include <vulkan-context.h>
#include <pipeline3D.h>
#include <vertex-array.h>
#include <imgui-context.h>
#include <textures.h>
#include <indexed-mesh.h>

#include <application.h>
#include <camara.h>
#include <axes-object.h>
#include <malla-sp.h>

#include <application3D.h>


//  ------------------------------------------------------------------------------

class ExampleTexturesSet : public vkhc::TexturesSet
{
    public:
    ExampleTexturesSet( vkhc::VulkanContext * p_context ) ;
    
} ;

//  ------------------------------------------------------------------------------


ExampleTexturesSet::ExampleTexturesSet( vkhc::VulkanContext * p_context ) 

:   TexturesSet( p_context ) 
{
    using namespace std ;
    cout << "Creating example textures set ..." << endl ;
    add( "../assets/wood-1.png" );
    add( "../assets/wood-2.png" );
    add( "../assets/wood-3.png" );
    add( new vkhc::ProceduralTexture1( context ) ) ;
    cout << "Example textures set created." << endl ;
}

// -------------------------------------------------------------------------------  
// class 'Triangle' (a 'vertex-array' like object )

class Triangle : public DrawableObject
{
    private:
    
    int texture_index = -1 ;
    vkhc::VertexArray * vertex_array = nullptr ;

    public: 
    
    Triangle( vkhc::VulkanContext & vulkan_context) ;
    virtual void drawVK( vkhc::BasicPipeline * pipeline, vkhc::VulkanContext & context, VkCommandBuffer & cmd_vk ) override ;
    void drawIMGUIWidgets() override ;
    virtual ~Triangle()  {} ;   
} ;
// -------------------------------------------------------------------------------  

Triangle::Triangle( vkhc::VulkanContext & vulkan_context)
{
    using namespace glm ;
    using namespace std ;

    cout << "Creating test RGB triangle object ..." << endl ;
    setName( "test RGB triangle" ) ;

    vertex_array = new vkhc::VertexArray( vulkan_context, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 4 ) ; // VK_PRIMITIVE_TOPOLOGY_PATCH_LIST ) 
    Assert( vertex_array != nullptr, "cannot create vertex array for triangle" ) ;

    const vec3  v0 = vec3{ 0.7f, 0.0f, 0.0f } ,
                v1 = vec3{ 0.0f, 0.7f, 0.0f } ,
                v2 = vec3{ 0.0f, 0.0f, 0.7f } ;
    vertex_array->setAttribData( 0, vector<vec3>{ v0, v1, v2 } );
        
    // location 1: vertex colors
    vertex_array->setAttribData( 1, vector<vec3>{ {1.0f,0.0f,0.0f}, {0.0f,1.0f,0.0f}, {0.0f,0.0f,1.0f} });

    // location 2: vertex normals (for testing illumination)
    vec3 normal = normalize( cross( v1-v0, v2-v0 ) ) ;
    vertex_array->setAttribData( 2, vector<vec3>{ normal, normal, normal });

    // location 3: vertex texture coordinates 
    vertex_array->setAttribData( 3, vector<vec2>{ {0.0f,0.0f}, {0.5f,1.0f}, {1.0f,0.0f} });
    
    // indexes 
    vertex_array->setIndexData( vector<uvec3>{{ 0, 1, 2 }} ); 

    cout << "Test RGB triangle object created." << endl ;
}
//  ------------------------------------------------------------------------------


void Triangle::drawVK( vkhc::BasicPipeline * pipeline, vkhc::VulkanContext & context, VkCommandBuffer & cmd_vk )
{
    Assert( vertex_array != nullptr, "cannot draw triangle: vertex array is null" ) ;
    vkhc::Pipeline3D * p = static_cast<vkhc::Pipeline3D *>( pipeline ) ;
    Assert( p != nullptr, "cannot get pipeline 3d" );
    p->setTextureIndex( cmd_vk,texture_index );
    vertex_array->draw( cmd_vk ) ;
}
//  ------------------------------------------------------------------------------

void Triangle::drawIMGUIWidgets() 
{
    using namespace ImGui ;

    if (CollapsingHeader("Triangle object controls", ImGuiTreeNodeFlags_DefaultOpen))
    {       
        
        int texture_combo_index = texture_index + 1 ; // map -1..3 to 0..4 for ImGui combo
        if ( Combo("Texture", &texture_combo_index, "No texture (vert. colors)\0Wood 1\0Wood 2\0Wood 3\0Procedural texture\0") )
        {
            texture_index = texture_combo_index - 1 ;
            std::cout << "Triangle::drawIMGUIWidgets: new texture_index == " << texture_index << std::endl ;
        }
    }
}

// ------------------------------------------------------------------------------
// class 'App3D' (the main application class)

App3D::App3D( ) 

//:   Application( 1024, 512, "Vulkan simple demo" ) 
:   Application( 0, 0, "Vulkan 3D Demo App" ) 
{
    using namespace std ;
    using namespace vkhc ; 
    using namespace ilc ;

    Assert( context != nullptr, "Tess1App constructor: 'context' instance is null !!" );

    // create ot
    axes3D       = new AxesObject();                   assert( axes3D != nullptr ) ;
    textures_set = new ExampleTexturesSet( context );  assert( textures_set != nullptr ) ;
    pipeline     = new Pipeline3D( *context, true );   assert( pipeline != nullptr ) ;
    camera       = new CamaraOrbitalSimple();          assert( camera != nullptr ) ;

    textures_set->bindTo( *pipeline ) ; // bind the textures set to the pipeline, so that its textures can be used in the fragment shader.

    // create drawable objects vector 
    drawable_objects.push_back( new Triangle( *context ) ) ; // index 0
    drawable_objects.push_back( new Cube24(  ) ) ; // index 1
    drawable_objects.push_back( new MallaSupPar( new FPEsfera(), 64, 64, true ) ) ; 
    drawable_objects.push_back( new MallaSupPar( new FPColumna(), 128, 128, true ) ) ;

    current_object_index = 2 ; // start with the sphere in the vector

    Assert( current_object_index < drawable_objects.size(), "App3D constructor: current object index is out of range !!" ) ;
    Assert( drawable_objects[current_object_index] != nullptr, "App3D constructor: current object is null !!" ) ;

    captureEvents( true, true, true, true );
    cout  << "App3D::App3D -- ends" << endl ;
} ;

// ----------------------------------------------------------------------------------
// destructor

App3D::~App3D() 
{
    //Assert( context != nullptr, "Tess1App destructor: 'context' instance is null !!" );
    for (auto* obj : drawable_objects) 
        delete obj;
    drawable_objects.clear();

    delete pipeline ; pipeline = nullptr ;
    delete textures_set ; textures_set = nullptr ;

    std::cout << "Deleted 'App3D' instance" << std::endl ;
}
// ----------------------------------------------------------------------------------

// Called when any mouse button is pressed or released 
void App3D::mouseButtonEventCB( double xpos, double ypos, int button, int action, int mods )
{
    using namespace std ;
    if ( debug_events )
        cout << "App3D::mouseButtonEventCB: xpos=" << xpos << " ypos=" << ypos << " button=" << button << " action=" << action << " mods=" << mods << endl ;
    if ( button == 1 && action == GLFW_PRESS )
    {
        prev_posx = xpos ;
        prev_posy = ypos ;
        if ( debug_events )
        cout << "App3D::mouseButtonEventCB: DRAG STARTxpos=" << xpos << " ypos=" << ypos << " button=" << button << " action=" << action << " mods=" << mods << endl ;
    }
}
// ----------------------------------------------------------------------------------

void App3D::mousePositionEventCB( double xpos, double ypos, int button ) 
{
    using namespace std ;
    if ( button != 1 ) // left button
        return ;

    if ( debug_events )
        cout << "App3D::mousePositionEventCB: xpos=" << xpos << " ypos=" << ypos << " button=" << button << endl ;
    assert( camera != nullptr ) ;
    const float dx = float(xpos - prev_posx) ;
    const float dy = float(ypos - prev_posy) ;
    camera->desplRotarXY( -0.6f*dx, -0.6f*dy ) ;
    prev_posx = xpos ;
    prev_posy = ypos ;
    
}
// ----------------------------------------------------------------------------------

void App3D::scrollEventCB( double xoffset, double yoffset ) 
{
    using namespace std ;
    if ( debug_events )
        cout << "App3D::scrollEventCB: xoffset=" << xoffset << " yoffset=" << yoffset << endl ;
    assert( camera != nullptr ) ;
    
    // zoom factor per scroll unit (depends on platform, because the scroll units are different in Linux and Windows/MacOS)
    #if defined(__linux__)
    constexpr float zoom_factor = -0.7f ; 
    #elif defined(__MACOS__) || defined(_WIN32)
    constexpr float zoom_factor = -0.1f ; // zoom factor per scroll unit
    #else
    constexpr float zoom_factor = -0.1f ; // zoom factor per scroll unit
    #endif

    camera->moverZ( zoom_factor*float(yoffset) ) ; // zoom in/out
}

// ----------------------------------------------------------------------------------


void App3D::updateViewProjMats( vkhc::seconds_f frame_time_s )
{
    using namespace glm ;
    Assert( context != nullptr, "App2D::updateViewProjMats: 'context' instance is null !!" ) ;
    Assert( context->glfw_context != nullptr, "App2D::updateViewProjMats: 'glfw_context' instance is null !!" ) ;
    Assert( camera != nullptr, "App2D::updateViewProjMats: 'camera' instance is null !!" ) ;

    
    // // sets the model matrix and update angle
    model_mat = scale( vec3( triangle_scale, triangle_scale, 1.0f))*
                rotate( curr_angle_rad, vec3( 0.0f, 0.0f, 1.0f ) );
    curr_angle_rad += rotation_speed * frame_time_s.count() * 2.0f * M_PI ; // increase angle

    const uvec2 ra_ext = context->getRenderAreaExtent(); // render area extent (size of the render area left to GUI, in pixels)
    const float ayx    = float(ra_ext.y) / float(ra_ext.x) ; // aspect ratio (height/width) of the render area
    camera->fijarRatioViewport( ayx ) ; // set the camera aspect ratio to the current window aspect ratio
    view_mat = camera->viewMatrix() ;
    proj_mat = camera->projectionMatrix() ;

}
// ----------------------------------------------------------------------------------


char buffer[256] = { 0 } ; // buffer for IMGUI input text widget

void App3D::drawIMGUIWidgets(  ) 
{
    using namespace ImGui ;
    using namespace std ;
    Assert( drawable_objects[current_object_index] != nullptr, "App3D::drawIMGUIWidgets: current object is null !!" ) ;
    //cout << "App3D::drawIMGUIWidgets: drawing IMGUI widgets -- close_requested:" << close_requested << endl ;

    
    
    if ( Button("Close window" ) ) close_requested = true ;
    if (CollapsingHeader("View and render config", ImGuiTreeNodeFlags_DefaultOpen))
    {
        Checkbox("Draw grid", &draw_grid);
        Checkbox("Draw axes", &draw_axes);
        bool draw_wireframe = pipeline->getDrawWireframe() ;
        if ( Checkbox("Draw wireframe", &draw_wireframe) )
            pipeline->setDrawWireframe( draw_wireframe ) ; 
        bool draw_normals = pipeline->getDrawNormals() ;
        if ( Checkbox("Draw normals", &draw_normals) )
            pipeline->setDrawNormals( draw_normals ) ; 
    }
    if (CollapsingHeader("Illumination controls", ImGuiTreeNodeFlags_DefaultOpen))
        Checkbox("Evaluate illumination", &eval_illumination);

    drawIMGUIObjectSelectionCombo() ; // draw the current object selection combo
    drawable_objects[current_object_index]->drawIMGUIWidgets() ; // draw the current object widgets
    
    if ( InputText("Input text (debug)", buffer, IM_ARRAYSIZE(buffer)) ) // debug
    {
        // do something with the input text in 'buffer'
        using namespace std ;
        cout << "Input text: " << buffer << endl ;
    }
    Text("FPS: %.1f (%.1f ms)", ImGui::GetIO().Framerate, 1000.0f / ImGui::GetIO().Framerate);
}
// ----------------------------------------------------------------------------------

void App3D::initFrame( const vkhc::seconds_f  time_elapsed )
{
    Assert( context != nullptr, "Tess1App::drawFrame: 'context' instance is null !!" );
    Assert( pipeline != nullptr, "Tess1App::drawFrame: 'pipeline' instance is null !!" );
    Assert( drawable_objects[current_object_index] != nullptr, "Tess1App::drawFrame: 'current_object' instance is null !!" );

    // update UBO uniforms in the pipeline
    updateViewProjMats( time_elapsed ) ; // updates 'view_mat' and 'proj_mat' 
    pipeline->setViewMatrix( view_mat ) ;
    pipeline->setProjectionMatrix( proj_mat ) ;

}
// ----------------------------------------------------------------------------------

void App3D::drawFrame( VkCommandBuffer & cmd ) 
{
    Assert( context != nullptr, "Tess1App::drawFrame: 'context' instance is null !!" );
    Assert( pipeline != nullptr, "Tess1App::drawFrame: 'pipeline' instance is null !!" );
    Assert( current_object_index < drawable_objects.size(), "Tess1App::drawFrame: 'current_object_index' is out of bounds !!" );
    Assert( drawable_objects[current_object_index] != nullptr, "Tess1App::drawFrame: 'current_object' instance is null !!" );
    Assert( axes3D != nullptr, "Tess1App::drawFrame: 'axes3D' instance is null !!" );
   
    // activate the pipeline and sets the viewport
    pipeline->bind( cmd );
    
    // send the current base colors set to the shaders (via UBO)
    pipeline->setBaseColorsSet() ;

    // give initial values to the push constants at the begining of 'cmd'
    pipeline->resetModelMatrix( cmd ) ; // sets the model matrix to identity and clears the model matrix stack
    pipeline->setTextureIndex( cmd, -1) ;   // no texture by default
    pipeline->setBaseColorIndex( cmd, -1 ) ;
    pipeline->setEvalIllumination( cmd, eval_illumination ) ; // sets the illumination evaluation mode (true or false) in the shaders

    // draw the axes 
    axes3D->setActive( draw_axes, draw_grid );
    axes3D->drawVK( pipeline, *context, cmd ) ;

    // draw the triangle and the widgets 
    drawable_objects[current_object_index]->drawVK( pipeline, *context, cmd );
}


// end of class 'App3D'
// *********************************************************************************



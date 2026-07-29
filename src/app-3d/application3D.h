#include <iostream>
#include <vector>
#include <cmath>
#include <chrono>

#include <vkhc/vulkan-context.h>
#include <vkhc/pipeline3D.h>
#include <vkhc/vertex-array.h>
#include <vkhc/imgui-context.h>
#include <vkhc/textures.h>

#include <ilc/indexed-mesh.h>
#include <ilc/application.h>
#include <ilc/camara.h>
#include <ilc/axes-object.h>
#include <ilc/malla-sp.h>




class ExampleTexturesSet ; // forward declaration

// ------------------------------------------------------------------------------

class App3D : public ilc::Application
{

    private:

    static constexpr bool debug_events = false ; 

    // parameters for the triangle model matrix and animation
    float curr_angle_rad = M_PI/2.0f ;  // current angle in radians
    float rotation_speed = 0.0f ; // angular speed in cycles per second 
    float triangle_scale = 0.8f ;

    // draw grid 
    bool draw_grid = true ;

    // draw axes
    bool draw_axes = true ;

    
    // eval illumination switch (true or false)
    bool eval_illumination = true ; // default value, can be changed dynamically in command
      

    // model, view and projection matrices
    glm::mat4 model_mat ;            // model matrix passed to the pipeline via a push constant
    glm::mat4 view_mat = glm::mat4(1.0f); // view matrix passed via UBO
    glm::mat4 proj_mat = glm::mat4(1.0f) ; // projection matrix passed via UBO

    // drawable 3D object (triangle) with the axes.
    AxesObject * axes3D = nullptr ;

    // array of drawable objects 
    std::vector<DrawableObject*> drawable_objects ;

    // index for current object being displayed (in the 'drawable_objects' vector)
    uint32_t current_object_index = 0 ;

    // basic 2D pipeline 
    vkhc::Pipeline3D * pipeline = nullptr ; 


    // textures set (used for testing textures).
    ExampleTexturesSet * textures_set = nullptr ; 

    // pointer to the current camera 
    ilc::CamaraInteractiva * camera = nullptr ; 

    // mouse position when draggin started 
    double prev_posx = 0.0 ;
    double prev_posy = 0.0 ;

    // Draw IMGUIcurrent drawable object selection combo
    void drawIMGUIObjectSelectionCombo() 
    {
        using namespace ImGui ;
        if (CollapsingHeader("Drawable object selection", ImGuiTreeNodeFlags_DefaultOpen))
        {       
            std::vector<const char*> object_names ;
            for (auto* obj : drawable_objects)
                object_names.push_back( obj->getName().c_str() );
            if ( Combo("Current object", reinterpret_cast<int*>(&current_object_index), object_names.data(), int(object_names.size()) ) )
            {
                std::cout << "App3D::drawIMGUIObjectSelectionCombo: new current_object_index == " << current_object_index << std::endl ;
            }
        }
    }


    // -----------------------------------------------------------------------------
    // Methods:
    
    public:

    App3D( ) ;
    virtual ~App3D()  override ; 

    // override methods
    void initFrame( const vkhc::seconds_f  time_elapsed ) override ;
    void drawFrame( VkCommandBuffer & cmd ) override ;
    void drawIMGUIWidgets(  ) override ;

    // specific methods for this application (not overrides)
    void updateViewProjMats( vkhc::seconds_f frame_time_s ) ;

    // Called when any mouse button is pressed or released 
    virtual void mouseButtonEventCB( double xpos, double ypos, int button, int action, int mods ) override ;

    // Called when mouse moved with any mouse button is pressed
    // button is 0 for the right button and 1 for the left button...
    virtual void mousePositionEventCB( double xpos, double ypos, int button ) override ; 

    // scroll event callback (called when the mouse wheel is scrolled)
    virtual void scrollEventCB( double xoffset, double yoffset ) override ;


    // mouse position event CB 
} ;

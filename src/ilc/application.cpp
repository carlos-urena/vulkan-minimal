// implementation of class 'Application' 
//
// Encapsulates all state data for a single window Vulkan App.


#include <application.h>
#include <common.h>

namespace ilc 
{

// definition of the pointer to the singleton instance 
Application * Application::app_singleton = nullptr ; // initialize the static member of the class


// --------------------------------------------------------------------------------
// Static event callback functions 
// to make event handling compatible with IMGUI, read:
// https://github.com/ocornut/imgui/blob/master/docs/FAQ.md#q-how-can-i-tell-whether-to-dispatch-mousekeyboard-to-dear-imgui-or-my-application
// These functions override IMGUI callbacks, but they forward events both to IMGUI and to the application
// --------------------------------------------------------------------------------

// Keyboard events

void Application::stKeyboardEventCB( GLFWwindow* window, int key, int scancode, int action, int mods ) 
{
    if ( app_singleton == nullptr ) 
    {   std::cerr << "KeyboardEventCB: warning: 'app_singleton' is null !" << std::endl ;
        return ;
    }

    app_singleton->keyboardEventCB( key, scancode, action, mods ) ;
} ; 


// Mouse button events

void Application::stMouseButtonEventCB( GLFWwindow* window, int button, int action, int mods ) 
{
    if ( app_singleton == nullptr ) 
    {   std::cerr << "MouseButtonEventCB: warning: 'app_singleton' is null !" << std::endl ;
        return ;
    }

    // See: https://github.com/ocornut/imgui/blob/master/docs/FAQ.md#q-how-can-i-tell-whether-to-dispatch-mousekeyboard-to-dear-imgui-or-my-application

    // forward the event to IMGUI (so it can process it, and set io.WantCaptureMouse accordingly)
    ImGuiIO& io = ImGui::GetIO();
    io.AddMouseButtonEvent( button, action == GLFW_PRESS); 

    Assert( button < GLFW_MOUSE_BUTTON_LAST, "Mouse button event: button index out of range !!" );
    // update the status of the mouse button (up or down)
    app_singleton->mouse_buttons[button] = (action == GLFW_PRESS) ;

    if ( !io.WantCaptureMouse )
        app_singleton->mouseButtonEventCB( button, action, mods ) ;
    
}

// Mouse position events (they are forwarded to the app only when any mouse button is pressed) 

void Application::stMousePositionEventCB( GLFWwindow* window, double xpos, double ypos ) 
{
    if ( app_singleton == nullptr ) 
    {   std::cerr << "MousePositionEventCB: warning: 'app_singleton' is null !" << std::endl ;
        return ;
    }

    // forward the event to IMGUI (so it can process it, and set io.WantCaptureMouse accordingly)
    ImGuiIO& io = ImGui::GetIO();
    io.AddMousePosEvent( (float)xpos, (float)ypos ); 

    if ( !io.WantCaptureMouse )
    {
        const bool left_or_right_down = app_singleton->mouse_buttons[0] || app_singleton->mouse_buttons[1] ;

        using namespace std ;
        if ( left_or_right_down )
            app_singleton->mousePositionEventCB( xpos, ypos ) ;
           
    }
}

// --------------------------------------------------------------------------------


Application::Application( int nx, int ny, const std::string & title ) 
{
    Assert( app_singleton == nullptr, "Application constructor: an instance of 'Application' already exists !!" );
    app_singleton = this ; // set the singleton instance pointer to this instance
    Assert( context == nullptr, "Application constructor: 'context' instance already exists !!" );
    context = new vkhc::VulkanContext{ nx, ny, title } ;
    Assert( context != nullptr, "Application constructor: failed to create VulkanContext instance !!" );

    // register GLFW event callbacks (static) methods.
    glfwSetKeyCallback( context->glfw_context->glfw_window, stKeyboardEventCB ) ; // keyboard event callback
    glfwSetMouseButtonCallback( context->glfw_context->glfw_window, stMouseButtonEventCB ) ; // mouse button event callback
    glfwSetCursorPosCallback( context->glfw_context->glfw_window, stMousePositionEventCB ) ; // mouse position event callback
}


// --------------------------------------------------------------------------------
// Event callback virtual methods (called by glfwPollEvents() or 
// glfwWaitEvents() when a keyboard event is raised)

void Application::keyboardEventCB( int key, int scancode, int action, int mods ) 
{
    // default implementation does nothing, derived classes can override it to process keyboard events
    using namespace std ;
    cout << "Keyboard event: key=" << key << " scancode=" << scancode << " action=" << action << " mods=" << mods << endl ;
}

void Application::mouseButtonEventCB( int button, int action, int mods ) 
{
    // default implementation does nothing, derived classes can override it to process mouse button events
    using namespace std ;
    cout << "Mouse button event: button=" << button << " action=" << action << " mods=" << mods << endl ;
}

void Application::mousePositionEventCB( double xpos, double ypos ) 
{
    // default implementation does nothing, derived classes can override it to process mouse position events
    using namespace std ;
    cout << "Mouse position (with button down) event: xpos=" << xpos << " ypos=" << ypos << endl ;
}

// --------------------------------------------------------------------------------
// Method which draws IMGUI widgets in each frame

void Application::drawIMGUIWidgets(  ) 
{
    // default implementation does nothing, derived classes can override it to draw IMGUI widgets in each frame
}

// --------------------------------------------------------------------------------

void Application::run() 
{
    using namespace vkhc ;

    Assert( context != nullptr, "Application::run: 'context' instance is null !!" );
    VkCommandBuffer cmd ;
    seconds_f frame_time_s ;
    
    InitFrameStart();

    // enter the main loop
    while ( ! context->windowShouldClose() && ! close_requested )  
    {
        // compute delay (in seconds) from previous frame start 
        frame_time_s = NextFrameStart() ; 

        // process pending events 
        context->pollEvents();  

        // call the virtual function to configure the pipeline before drawing the frame (to be defined in derived classes)
        initFrame( frame_time_s ) ; 

        // get an image_index and a command buffer from the swapchain 
        // (if it is not possible, start over the loop, to try again)
        if ( ! context->beginFrame( clear_color, cmd, image_index ) ) 
            continue ; 

        // draw the frame 
        context->setRenderAreaViewport( cmd ) ; // set the render area to the left of the GUI
        drawFrame( cmd ) ; // draw the frame adding commands to 'cmd'
        
        // draw the widgets 
        context->beginIMGUIFrame( cmd ) ;
        drawIMGUIWidgets(  ) ; // draw IMGUI widgets 
        context->endIMGUIFrame( cmd ) ;

        context->endFrame( cmd, image_index ) ;  // submit 'cmd' and present the image
    }
    // wait for the device to be idle before exiting, (so all resources can be safely released)
    context->waitDeviceIdle() ; 
}

// --------------------------------------------------------------------------------

Application::~Application() 
{
    Assert( context != nullptr, "Application destructor: 'context' instance is null !!" );
    delete context ;
    context = nullptr ; 
    std::cout << "Application deleted" << std::endl ;
}
// --------------------------------------------------------------------------------



} ; // end of namespace 'ilc'

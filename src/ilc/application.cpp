// implementation of class 'Application' 
//
// Encapsulates all state data for a single window Vulkan App.


#include <application.h>
#include <common.h>

#include <backends/imgui_impl_glfw.h>
ImGuiKey ImGui_ImplGlfw_KeyToImGuiKey(int keycode, int scancode); // not in imgui_impl_glfw.h, but it is defined in imgui_impl_glfw.cpp, so we declare it here to be able to use it in Application::stKeyboardEventCB


namespace ilc 
{

constexpr bool debug_events = false ;

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
    using namespace std ;
    if ( debug_events) 
        cout << "Application::stKeyboardEventCB: key=" << key << ", scancode=" << scancode << ", action=" << action << ", mods=" << mods << endl ;
    
    if ( app_singleton == nullptr ) 
    {   std::cerr << "KeyboardEventCB: warning: 'app_singleton' is null !" << std::endl ;
        return ;
    }

    ImGuiIO& io = ImGui::GetIO();
    if ( app_singleton->close_requested )
    {   if ( ( key == GLFW_KEY_ESCAPE || key == GLFW_KEY_N ) && action == GLFW_RELEASE )
        {   app_singleton->close_confirmed = false ;
            app_singleton->close_requested = false ;
        }
        else if ( ( key == GLFW_KEY_ENTER || key == GLFW_KEY_Y ) && action == GLFW_RELEASE )
            app_singleton->close_confirmed = true ;
    } 
    const ImGuiKey imgui_key = ImGui_ImplGlfw_KeyToImGuiKey(key,0);
    io.AddKeyEvent( imgui_key, action == GLFW_PRESS); 

    if ( !io.WantCaptureKeyboard && !io.WantTextInput)
    {
        if ( key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE )
            app_singleton->close_requested = true ;
        else if ( app_singleton->key_events ) 
            app_singleton->keyboardEventCB( key, scancode, action, mods ) ;   
    }
} ; 

// --------------------------------------------------------------------------------
// Mouse button events

void Application::stMouseButtonEventCB( GLFWwindow* window, int button, int action, int mods ) 
{
    using namespace std ;
    if ( debug_events) 
        cout << "Application::stMouseButtonEventCB: button=" << button << ", action=" << action << ", mods=" << mods << endl ;

    if ( app_singleton == nullptr ) 
    {   std::cerr << "MouseButtonEventCB: warning: 'app_singleton' is null !" << std::endl ;
        return ;
    }

    // See: https://github.com/ocornut/imgui/blob/master/docs/FAQ.md#q-how-can-i-tell-whether-to-dispatch-mousekeyboard-to-dear-imgui-or-my-application

    // forward the event to IMGUI (so it can process it, and set io.WantCaptureMouse accordingly)
    ImGuiIO& io = ImGui::GetIO();
    io.AddMouseButtonEvent( button, action == GLFW_PRESS); 

    double mpx, mpy ;
    glfwGetCursorPos( window, &mpx, &mpy ); // get mouse position

    Assert( button < GLFW_MOUSE_BUTTON_LAST, "Mouse button event: button index out of range !!" );
    // update the status of the mouse button (up or down)
    app_singleton->mouse_buttons[button] = (action == GLFW_PRESS) ;

    if ( !io.WantCaptureMouse )
        app_singleton->mouseButtonEventCB( mpx, mpy, button, action, mods ) ;
    
}

// --------------------------------------------------------------------------------
// Mouse position events (they are forwarded to the app only when any mouse button is pressed) 

void Application::stMousePositionEventCB( GLFWwindow* window, double xpos, double ypos ) 
{
    using namespace std ;
    if ( debug_events) 
        cout << "Application::stMousePositionEventCB: xpos=" << xpos << ", ypos=" << ypos << endl ;

    if ( app_singleton == nullptr ) 
    {   std::cerr << "MousePositionEventCB: warning: 'app_singleton' is null !" << std::endl ;
        return ;
    }

    // forward the event to IMGUI (so it can process it, and set io.WantCaptureMouse accordingly)
    ImGuiIO& io = ImGui::GetIO();
    io.AddMousePosEvent( (float)xpos, (float)ypos ); 

    if ( !io.WantCaptureMouse )
        if ( app_singleton->mouse_buttons[0] || app_singleton->mouse_buttons[1] )
        {
            const int button = app_singleton->mouse_buttons[0] ? 0 : 1 ; 
            app_singleton->mousePositionEventCB( xpos, ypos, button ) ;
        }
    
}

void Application::stScrollEventCB( GLFWwindow* window, double xoffset, double yoffset ) 
{
    using namespace std ;
    if ( debug_events) 
        cout << "Application::stScrollEventCB: xoffset=" << xoffset << ", yoffset=" << yoffset << endl ;

    if ( app_singleton == nullptr ) 
    {   std::cerr << "ScrollEventCB: warning: 'app_singleton' is null !" << std::endl ;
        return ;
    }

    // forward the event to IMGUI (so it can process it, and set io.WantCaptureMouse accordingly)
    ImGuiIO& io = ImGui::GetIO();
    io.AddMouseWheelEvent( (float)xoffset, (float)yoffset ); 

    if ( !io.WantCaptureMouse && app_singleton->scroll_events )
        app_singleton->scrollEventCB( xoffset, yoffset ) ;
    
}

// --------------------------------------------------------------------------------
// Constructor of the base, Application class 

Application::Application( int nx, int ny, const std::string & title ) 
{
    using namespace std ;
    cout << "Application::Application - starts" << endl ;

    Assert( app_singleton == nullptr, "Application constructor: an instance of 'Application' already exists !!" );
    Assert( context == nullptr, "Application constructor: 'context' instance already exists !!" );

    app_singleton = this ; // set the singleton instance pointer to this instance
    context = new vkhc::VulkanContext{ nx, ny, title } ;
    Assert( context != nullptr, "Application constructor: failed to create VulkanContext instance !!" );

    glfw_context = context->glfw_context ; // set the pointer to the GLFWContext instance
    Assert( glfw_context != nullptr, "Application constructor: 'glfw_context' instance is null !!" );

    cout << "Application::Application - ends" << endl ;
}
// --------------------------------------------------------------------------------

// Called by derived classes constructors to set which callbacks are captured 
void Application::captureEvents( bool p_key_events, bool p_mouse_button_events, bool p_mouse_position_events, bool p_scroll_events )  
{
    // keyboard event callback
    key_events = p_key_events ; // when true, key events are passed to the application 
    glfwSetKeyCallback( glfw_context->glfw_window, stKeyboardEventCB ) ; 
    
    // mouse button event callback
    mouse_button_events = p_mouse_button_events ;
    if ( mouse_button_events )
        glfwSetMouseButtonCallback( glfw_context->glfw_window, stMouseButtonEventCB ) ; 
    
    // mouse position event callback
    mouse_position_events = p_mouse_position_events ;
    if ( mouse_position_events )
        glfwSetCursorPosCallback( glfw_context->glfw_window, stMousePositionEventCB ) ; 
    
    // scroll event callback
    scroll_events = p_scroll_events ;
    if ( scroll_events )
        glfwSetScrollCallback( glfw_context->glfw_window, stScrollEventCB ) ; 
}

// --------------------------------------------------------------------------------
// Event callback virtual methods (called by glfwPollEvents() or 
// glfwWaitEvents() when a keyboard event is raised)

void Application::keyboardEventCB( int key, int scancode, int action, int mods ) 
{
    // default implementation does nothing, derived classes can override it to process keyboard events
    using namespace std ;
    if ( debug_events)
        cout << "Application::keyboardEventCB: key=" << key << " scancode=" << scancode << " action=" << action << " mods=" << mods << endl ;
}

void Application::mouseButtonEventCB( double xpos, double ypos, int button, int action, int mods ) 
{
    // default implementation does nothing, derived classes can override it to process mouse button events
    using namespace std ;
    if ( debug_events)
        cout << "Application::mouseButtonEventCB: xpos=" << xpos << " ypos=" << ypos << " button=" << button << " action=" << action << " mods=" << mods << endl ;
}

void Application::mousePositionEventCB( double xpos, double ypos, int button ) 
{
    // default implementation does nothing, derived classes can override it to process mouse position events
    using namespace std ;
    if ( debug_events)
        cout << "Application::mousePositionEventCB: xpos=" << xpos << " ypos=" << ypos << " button=" << button << endl ;
}


void Application::scrollEventCB( double xoffset, double yoffset ) 
{
    // default implementation does nothing, derived classes can override it to process scroll events
    using namespace std ;
    if ( debug_events)
        cout << "Application::scrollEventCB: xoffset=" << xoffset << " yoffset=" << yoffset << endl ;
}

// --------------------------------------------------------------------------------
// Method which draws IMGUI widgets in each frame

void Application::drawIMGUIWidgets(  ) 
{
    // default implementation does nothing, derived classes can override it to draw IMGUI widgets in each frame
}

// --------------------------------------------------------------------------------
// Draw imgui modal window to confirm closing the application

void Application::confirmClose()
{
    using namespace ImGui ;
    ImGuiViewport* viewport = GetMainViewport();
    SetNextWindowPos(viewport->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    OpenPopup("Close application?");
    if (BeginPopupModal("Close application?", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        Text("Are you sure you want to close the application?");
        float yes_w = CalcTextSize("Yes").x + GetStyle().FramePadding.x * 2.0f;
        float no_w = CalcTextSize("No").x + GetStyle().FramePadding.x * 2.0f;
        float total_buttons_w = yes_w + GetStyle().ItemSpacing.x + no_w;
        float start_x = (GetWindowSize().x - total_buttons_w) * 0.5f;
        if (start_x > GetCursorPosX())
            SetCursorPosX(start_x);

        if (Button("Yes")) 
        {   close_confirmed = true; 
            CloseCurrentPopup(); 
        }
        SameLine();
        if (Button("No")) 
        {   close_requested = false; 
            close_confirmed = false;
            CloseCurrentPopup(); 
        }
        EndPopup(); 
    }
    
}

// --------------------------------------------------------------------------------
// Runs the aplication event + render loop (main loop)

void Application::run() 
{
    using namespace vkhc ;

    Assert( context != nullptr, "Application::run: 'context' instance is null !!" );
    VkCommandBuffer cmd ;
    seconds_f frame_time_s ;
    
    InitFrameStart();

    // enter the main loop
    while ( ! close_confirmed )  
    {
        // raise 'close_requested' flag if the user has clicked the close button of the window
        if ( glfw_context->windowShouldClose() )
            close_requested = true ; 
        
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
        if ( close_requested )
            confirmClose() ;
        if ( !close_confirmed )
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
    delete context ; // deletes everything inside the VulkanContext instance, including the GLFWContext instance
    context = nullptr ; 
    glfw_context = nullptr ;
    std::cout << "Deleted 'Application' instance." << std::endl ;
}
// --------------------------------------------------------------------------------



} ; // end of namespace 'ilc'

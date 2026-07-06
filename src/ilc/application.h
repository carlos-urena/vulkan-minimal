// declaration of class 'Application' 
//
// Encapsulates all state data for a single window Vulkan App.

#pragma once

#include <vulkan-context.h>

namespace ilc 
{

class Application
{
    protected: 

    // singleton instance of the application 
    static Application * app_singleton ; 
    
    // pointer to the VulkanContext instance (created in the constructor)
    vkhc::VulkanContext * context = nullptr ;

    // glfw context pointer (created in the constructor)
    vkhc::GLFWContext * glfw_context = nullptr ;
    
    // true when the users requests to close, written by app-specific 
    // event handlers.
    bool close_requested = false ; 

    // true when close_requested is 'true' and the user has confirmed to 
    // close the application in the confirmation popup (writen by 'confirmClose()' method)
    bool close_confirmed = false ; 

    // clear value for each frame 
    VkClearValue clear_color{ .color ={ .float32 ={ 0.0f, 0.0f, 0.0f, 1.0f }}};

    // index for image in use when drawing a frame(from the swap-chain)
    uint32_t image_index ; 

    // status of mouse buttons (up or down)
    bool mouse_buttons[GLFW_MOUSE_BUTTON_LAST] = { false } ;

    // draw modal popup window to confirm window close 
    void confirmClose() ;

    // true if the application wants to capture each event type: 

    bool key_events = false ; // key down/up 
    bool mouse_button_events = false ; // mouse button down/up
    bool mouse_position_events = false ; // mouse movements (with a button pressed)
    bool scroll_events = false ; // mouse scroll events

    // ---------------------------------------------------------------------------------

    public:

    // Constructor of the base, Application class (creates the VulkanContext instance)
    Application( int nx, int ny, const std::string & title );

    // Destructor of the base, Application class (deletes the VulkanContext instance)
    virtual ~Application() ;

    // Called by derived classes constructors to set which callbacks are captured 
    void captureEvents( bool key_events, bool mouse_button_events, bool mouse_position_events, bool scroll_events ) ;

    // Runs the aplication event + render loop (main loop)
    void run() ;

    // To be defined in derived classes (mandatory):
    // configures the pipeline before starting draw commands on each frame 
    // `time_elapsed` is the time elapsed from the previous frame start (in seconds)
    //  (it is the same value passed to 'drawFrame' for the same frame)
    virtual void initFrame( const vkhc::seconds_f  time_elapsed ) = 0;

    // To be defined in derived classes (mandatory):
    // adds draw commands to 'vk_cmd' to draw the frame, using 'time_ela' as the time elapsed from the previous frame start (in seconds)
    
    virtual void drawFrame( VkCommandBuffer & vk_cmd ) = 0 ;  

    // To be defined in derived classes (optionally) 
    // adds draw commands to 'vk_cmd' to draw IMGUI widgets in each frame (default implementation does nothing)

    virtual void drawIMGUIWidgets(  ) ; // to be defined in derived classes (optionally)

    // // to be defined in derived classes (returns allways true o allways false)
    // // (if any of them  is not redefined, it returns true, so that the application captures all events)
    // virtual bool captureKeyEvents( ) = 0; 
    // virtual bool captureMouseButtonEvents( ) = 0; 
    // virtual bool captureMousePositionEvents( ) = 0; // only received when any mouse button is pressed.
    
    // Event callback virtual methods (called by glfwPollEvents() or 
    // glfwWaitEvents() when a keyboard event is raised)
    // If any derived class wants to capture, it must override the method.

    // Called when a key is pressed or released
    virtual void keyboardEventCB( int key, int scancode, int action, int mods ) ;

    // Called when any mouse button is pressed or released 
    virtual void mouseButtonEventCB( double xpos, double ypos, int button, int action, int mods ) ;
    
    // Called when mouse moved with any mouse button is pressed
    // button is 0 for the right button and 1 for the left button...
    virtual void mousePositionEventCB( double xpos, double ypos, int button ) ; 

    // Called when mouse wheel is scrolled
    virtual void scrollEventCB( double xoffset, double yoffset ) ;

    // static event callback functions
    static void stKeyboardEventCB( GLFWwindow* window, int key, int scancode, int action, int mods ) ;
    static void stMouseButtonEventCB( GLFWwindow* window, int button, int action, int mods ) ;
    static void stMousePositionEventCB( GLFWwindow* window, double xpos, double ypos ) ; 
    static void stScrollEventCB( GLFWwindow* window, double xoffset, double yoffset ) ;
} ; // end of class Application

} // end of namespace ilc

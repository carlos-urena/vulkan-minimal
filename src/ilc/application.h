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
    
    vkhc::VulkanContext * context = nullptr ;
    bool close_requested = false ;
    VkClearValue clear_color{ .color ={ .float32 ={ 0.0f, 0.0f, 0.0f, 1.0f }}};

    uint32_t image_index ; // index for image in use when drawing a frame(from the swap-chain)

    // status of mouse buttons (up or down)
    bool mouse_buttons[GLFW_MOUSE_BUTTON_LAST] = { false } ;

    public:

    Application( int nx, int ny, const std::string & title );
    virtual ~Application() ;
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

    // to be defined in derived classes (returns allways true o allways false)
    // (if any of them  is not redefined, it returns true, so that the application captures all events)
    virtual bool captureKeyEvents( ) ; 
    virtual bool captureMouseButtonEvents( ) ; 
    virtual bool captureMousePositionsEvents( ) ; // only received when any mouse button is pressed.
    
    // Event callback virtual methods (called by glfwPollEvents() or 
    // glfwWaitEvents() when a keyboard event is raised)

    virtual void keyboardEventCB( int key, int scancode, int action, int mods ) ;
    virtual void mouseButtonEventCB( int button, int action, int mods ) ;
    virtual void mousePositionEventCB( double xpos, double ypos ) ;

    // static event callback functions
    static void stKeyboardEventCB( GLFWwindow* window, int key, int scancode, int action, int mods ) ;
    static void stMouseButtonEventCB( GLFWwindow* window, int button, int action, int mods ) ;
    static void stMousePositionEventCB( GLFWwindow* window, double xpos, double ypos ) ;

} ; // end of class Application

} // end of namespace ilc

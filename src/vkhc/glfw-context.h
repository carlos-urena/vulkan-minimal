// Declaration of GLFWContext class 

#pragma once 

#include <vkhc/common.h>

namespace vkhc
{

class GLFWContext
{

    private: 
    static unsigned instance_count ;  

    public:
    
    GLFWwindow* glfw_window = nullptr;
    static void errorFunc( int error_code, const char* description);
    
    // Initializes GLFW and creates the window.
    // When width and height are 0, this functions selects a size that fits the screen and places the window in the monitor with the largest area.
    GLFWContext(int width, int height,const std::string & title ) ;   
    
    // return a vulkan surface created from the vulkan instance
    VkSurfaceKHR* createVkSurface( VkInstance *vk_instance ) ;
    
    // waits for the framebuffer extent to be > 0, and return its extent
    void getCurrentWindowSize( int & width, int & height ) ;
    
    // closes the window and terminates GLFW
    ~GLFWContext() ;

    // returns true after the user clicks the close button of a window 
    // return true only once after the click (the flag is reset on each call)
    bool windowShouldClose() ;

    // computes window position and size based on the resolution and size of the
    // available monitors (places it inside the monitor with the largest area).

    void getWindowPositionAndSize( int & tamx, int & tamy, int & posx, int & posy ) ;
    
} ;

} // end vkhc  namespace 


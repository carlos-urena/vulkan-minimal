// Declaration of GLFWContext class 

#pragma once 

#include <common.h>

namespace vkhc
{

class GLFWContext
{

    private: 
    static unsigned instance_count ;  

    public:
    
    GLFWwindow* glfw_window = nullptr;
    static void errorFunc( int error_code, const char* description);
    

    GLFWContext( int width, int height, const char* title ) ;   
    
    // return a vulkan surface created from the vulkan instance
    VkSurfaceKHR* createVkSurface( VkInstance *vk_instance ) ;
    
    // waits for the framebuffer extent to be > 0, and return its extent
    void getCurrentWindowSize( int & width, int & height ) ;
    
    // closes the window and terminates GLFW
    ~GLFWContext() ;


    // computes window position and size based on the resolution and size of the
    // available monitors (places it inside the monitor with the largest area).

    void getWindowPositionAndSize( int & tamx, int & tamy, int & posx, int & posy ) ;
    
} ;

} // end vkhc  namespace 


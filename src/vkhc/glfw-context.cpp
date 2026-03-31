// Implementation of GLFWContext class

#include <glfw-context.h>

namespace vkhc
{

unsigned GLFWContext::instance_count = 0 ; // initialize static member variable

// -----------------------------------------------------------------------------
// Static GLFW error function 

void GLFWContext::errorFunc( int error_code, const char* description)
{
    using namespace std ;
    cout << "GLFW error: " << endl 
            << "  code        : " << error_code << endl
            << "  description : " << description << endl 
            << endl  ;
    exit(1) ;
}
// -----------------------------------------------------------------------------
// Constructor: initializes GLFW 

GLFWContext::GLFWContext( int width, int height, const char* title ) 
{
    if ( instance_count > 0 ) 
    {   std::cerr << "Warning: more than one instance of GLFWState created. Aborting." << std::endl ;
        std::exit(1) ;
    }
    instance_count++ ;

    glfwSetErrorCallback( errorFunc );
    glfwInit();
    glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API );
    glfw_window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    assert( glfw_window != nullptr );
    glfwSetFramebufferSizeCallback( glfw_window, framebufferResizeCallback );
}

// -----------------------------------------------------------------------------

VkSurfaceKHR* GLFWContext::createVkSurface( VkInstance *vk_instance ) 
{
    assert( vk_instance != nullptr );
    auto * vk_surface = new VkSurfaceKHR ;
    assert( vk_surface != nullptr );
    glfwCreateWindowSurface( *vk_instance, glfw_window, nullptr, vk_surface);
    return vk_surface;
}

// -----------------------------------------------------------------------------
// waits for the framebuffer extent to be > 0, and return its extent

void GLFWContext::getCurrentWindowSize( int & width, int & height )
{
    width = 0;
    height = 0;
    glfwGetFramebufferSize( glfw_window, &width, &height);
    while (width == 0 || height == 0) 
    {
        std::cout << "Window size is 0, waiting for it to be updated ..." << std::endl;
        glfwWaitEvents();
        glfwGetFramebufferSize( glfw_window, &width, &height);
    }
    std::cout << "New window size: " << width << "x" << height << std::endl;
}

// -----------------------------------------------------------------------------
// closes the window and terminates GLFW

GLFWContext::~GLFWContext() 
{
    glfwDestroyWindow( glfw_window );
    glfwTerminate();
    std::cout << "Deleted GLFW state (closed window and GLFW shutdown)" << std::endl ;
}

// -----------------------------------------------------------------------------

} ; // fin del namespace vkhc 


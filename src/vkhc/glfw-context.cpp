// Implementation of GLFWContext class

#include <glfw-context.h>


namespace vkhc
{
    
// ----------------------------------------------------------------------------------
// computes window position and size based on the resolution and size of the
// available monitors (places it inside the monitor with the largest area).

void GLFWContext::getWindowPositionAndSize( int & tamx, int & tamy, int & posx, int & posy )
{

   using namespace std ;

    // read number of monitors and pointer to the monitor array
    int num_monitors ;
    GLFWmonitor ** monitors = glfwGetMonitors( &num_monitors );
    unsigned       ims       = 0 ; // index of the selected monitor
    unsigned long  area_max  = 0 ;

    cout << "Monitors and GLFW window:" << endl ;

    // iterate over monitors and compute in 'ims' the index of the monitor with the largest pixel area
    for( int i = 0 ; i < num_monitors ; i++ )
    {
        const GLFWvidmode   * modo = glfwGetVideoMode( monitors[i] ); 
        const unsigned long   area = (unsigned long) modo->width * (unsigned long) modo->height ; 
        int mxpos, mypos ;

        glfwGetMonitorPos( monitors[i], &mxpos, &mypos );
        
        cout << "   Monitor '" << glfwGetMonitorName( monitors[i] ) << "': " 
            << "position: " << mxpos << " x " << mypos  << ", "
            << "size: " << modo->width << " x " << modo->height << "." 
            << endl ;

        if ( area_max < area )
        {   area_max = area ;
            ims     = i ;
        }
    }
    cout << "   Using monitor: '" << glfwGetMonitorName( monitors[ims] ) << "'." << endl ;
    
        // read monitor size and position
    const GLFWvidmode * modo = glfwGetVideoMode( monitors[ims] ); // current mode of the monitor with the largest area
    int ancho_tot = modo->width,   // total desktop width in the current mode
        alto_tot  = modo->height,  // total desktop height in the current mode
        mxpos,                     // X position of the monitor within the complete virtual desktop
        mypos ;                    // Y position of the monitor within the complete virtual desktop.

    glfwGetMonitorPos( monitors[ims], &mxpos, &mypos );

        // compute position and size

    tamx  = (alto_tot*4)/5 ;     // window width
    tamy  = tamx ;               // window height
    posx  = mxpos+(ancho_tot-tamx)/2 ; // window X position 
    posy  = mypos+(alto_tot-tamy)/2;   // window Y position 

    cout << "   Window: position: " << posx << " x " << posy << ", size: " << tamx << " x " << tamy << "." << endl ;
}




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

GLFWContext::GLFWContext( int width, int height, const std::string & title ) 
{
    using namespace std ;
    
    if ( instance_count > 0 ) 
    {   std::cerr << "Warning: more than one instance of GLFWState created. Aborting." << std::endl ;
        std::exit(1) ;
    }
    instance_count++ ;

    const char * vers = glfwGetVersionString() ;
    std::cout << "GLFW version: " << vers << std::endl ;
    glfwSetErrorCallback( errorFunc );
    glfwInit();


    #if (GLFW_VERSION_MAJOR > 3) || (GLFW_VERSION_MAJOR == 3 && GLFW_VERSION_MINOR >= 4) 
    const int platform = glfwGetPlatform();
    cout << "GLFW platform in use: " ;
    switch ( platform )
    {
        case GLFW_PLATFORM_WAYLAND : cout << "Wayland" << endl ; break ;
        case GLFW_PLATFORM_X11     : cout << "X11" << endl ;     break ;
        case GLFW_PLATFORM_COCOA   : cout << "Cocoa" << endl ;   break ;
        case GLFW_PLATFORM_WIN32   : cout << "Windows" << endl ; break ;
        default: cout << "unknown" << endl ; break ;
    }
    #endif

    int sizex, sizey ;  // requested window size.
    
    #ifdef __linux__
    sizex = width > 0 ? width : 1500 ; // default width if width == 0
    sizey = height > 0 ? height : 750 ; // default height if height == 0
    #else 
    int posx, posy ; // requested window position  
    getWindowPositionAndSize( sizex, sizey, posx, posy ) ; // does not reports correct values on wayland on linux
    #endif

    glfwWindowHint( GLFW_RESIZABLE, GLFW_TRUE );
    glfwWindowHint( GLFW_MAXIMIZED, GLFW_FALSE );
    glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API );

    cout << "About to create window with requested size " << sizex << " x " << sizey << " and title '" << title << "'." << endl ;
    
    glfw_window = glfwCreateWindow( sizex, sizey, title.c_str(), nullptr, nullptr);
    assert( glfw_window != nullptr );
    
    #ifndef __linux__
    glfwSetWindowPos( glfw_window, posx, posy ); // not available in wayland on linux
    #endif
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

} ; // end of namespace vkhc 


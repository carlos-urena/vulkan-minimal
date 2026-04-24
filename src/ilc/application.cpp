// implementation of class 'Application' 
//
// Encapsulates all state data for a single window Vulkan App.


#include <application.h>
#include <common.h>

namespace ilc 
{

Application::Application( int nx, int ny, const std::string & title ) 
{
    Assert( context == nullptr, "Application constructor: 'context' instance already exists !!" );
    context = new vkhc::VulkanContext{ nx, ny, title } ;
    Assert( context != nullptr, "Application constructor: failed to create VulkanContext instance !!" );
}
// --------------------------------------------------------------------------------

void Application::drawIMGUIWidgets( VkCommandBuffer & vk_cmd ) 
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
        drawIMGUIWidgets( cmd ) ; // draw IMGUI widgets onto 'cmd'
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

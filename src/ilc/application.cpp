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
        frame_time_s = NextFrameStart() ; // compute delay (in seconds) from previous frame start 

        context->pollEvents();  // process pending events 

        if ( ! context->beginFrame( clear_color, cmd, image_index ) ) 
            continue ; 

        drawFrame( cmd, frame_time_s ) ; // call the virtual function to draw the frame (to be defined in derived classes)
        
        context->beginIMGUIFrame( cmd ) ;
        drawIMGUIWidgets( cmd ) ; // call the virtual function to draw the IMGUI widgets, if any.
        context->endIMGUIFrame( cmd ) ;

        context->endFrame( cmd, image_index ) ;
    }

    context->waitDeviceIdle() ;

    //device->waitIdle() ; // wait for the device to be idle before exiting, to ensure all resources can be safely released in the destructors of the state objects.
    
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

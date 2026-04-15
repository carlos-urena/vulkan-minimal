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
    
    vkhc::VulkanContext * context = nullptr ;
    bool close_requested = false ;
    VkClearValue clear_color{ .color ={ .float32 ={ 0.0f, 0.0f, 0.0f, 1.0f }}};

    uint32_t image_index ; // index for image in use when drawing a frame(from the swap-chain)

    public:

    Application( int nx, int ny, const std::string & title );
    virtual ~Application() ;
    void run() ;
    
    // To be defined in derived classes:
    // adds draw commands to 'vk_cmd' to draw the frame, using 'time_ela' as the time elapsed from the previous frame start (in seconds)
    // `time_elapsed` is the time elapsed from the previous frame start (in seconds)
    virtual void drawFrame( VkCommandBuffer & vk_cmd, const vkhc::seconds_f  time_elapsed ) = 0 ;  

    // To be defined in derived classes (optionally) 
    // adds draw commands to 'vk_cmd' to draw IMGUI widgets in each frame (default implementation does nothing)

    virtual void drawIMGUIWidgets( VkCommandBuffer & vk_cmd ) ; // to be defined in derived classes (optionally)
    
} ;

} // end of namespace ilc

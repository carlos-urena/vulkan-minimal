// declaration of class 'Application' 
//
// Encapsulates all state data for a single window Vulkan App.

#pragma once

#include <vulkan-context.h>

namespace ilc 
{

class Application
{
    vkhc::VulkanContext * context = nullptr ;
    bool close_requested = false ;
    VkClearValue clear_color{ .color ={ .float32 ={ 0.0f, 0.0f, 0.0f, 1.0f }}};

    uint32_t image_index ; // index for image in use when drawing a frame(from the swap-chain)

    public:

    Application( int nx, int ny, const std::string & title );
    ~Application() ;
    void run() ;
    virtual void drawFrame( VkCommandBuffer & vk_cmd, const vkhc::seconds_f  time_ela) = 0 ;  // to be defined in derived classes 
    virtual void drawIMGUIWidgets( VkCommandBuffer & vk_cmd ) ; // to be defined in derived classes (optionally)
    
} ;

} // end of namespace ilc

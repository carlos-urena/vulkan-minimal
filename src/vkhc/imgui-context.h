// Declaration of class 'IMGUIContext' 
//
// Encapsulates the state of the IMGUI library, including 
// style settings and Vulkan initialization info.

#pragma once 

#include <common.h>



namespace vkhc
{

class IMGUIContext
{
    public:

    ImGui_ImplVulkan_InitInfo init_info{};

    VkDescriptorPool imguiDescriptorPool ;
    VkDescriptorPoolCreateInfo dpci{ VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };

    bool resetImguiWindowPos = true ; 

    Device * device = nullptr ;
    Surface * surface = nullptr ;

    // ------------------------------------------------------------------------
    // Sets the style for IMGUI widgets
    //
    // interesting code for IMGUIStyle
    // https://www.unknowncheats.me/forum/c-and-c-/189635-imgui-style-settings.html
    // semantics: https://www.unknowncheats.me/forum/general-programming-and-reversing/269953-imgui-theme-color.html

    void setStyle() ;
    
    IMGUIContext( Instance * instance, Device * p_device, Surface * p_surface, RenderPass * render_pass, 
                        SwapChain * swap_chain, GLFWContext * glfw_state ) ;
    
    // called after the window is resized and the swap chain has been recreated 
    void windowResized( SwapChain * swap_chain ) ;
    
    // return the current GUI width, to the right of the window.
    uint32_t getGUIWidth() ;
    
    // called each frame before invoking IMGUI functions to create the interface
    void beginFrame() ;
    
    // called each frame after the IMGUI functions to create the interface, and before rendering the frame
    // adds the commands to vk_cmd_buffer 
    void endFrame( VkCommandBuffer & vk_cmd_buffer )  ;
    
    // destructor
    ~IMGUIContext() ;
} ;


} // vkhc namespace end 


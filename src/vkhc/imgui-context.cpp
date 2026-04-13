// Declaration of class 'IMGUIContext' 
//
// Encapsulates the state of the IMGUI library, including 
// style settings and Vulkan initialization info.


#include <common.h>
#include <imgui-context.h>
#include <instance.h>
#include <device.h>
#include <surface.h>
#include <render-pass.h>
#include <swap-chain.h>

namespace vkhc
{

// Sets the style for IMGUI widgets
//
// interesting code for IMGUIStyle
// https://www.unknowncheats.me/forum/c-and-c-/189635-imgui-style-settings.html
// semantics: https://www.unknowncheats.me/forum/general-programming-and-reversing/269953-imgui-theme-color.html

void IMGUIContext::setStyle()
{
    using namespace ImGui ;

    ImGuiIO &     io    = GetIO(); (void)io;
    
    // Font size and spacing parameters
    constexpr float f         = 0.9 ; // scale factor for all spacings
    constexpr float font_size = 16.0f ; // font size
    constexpr float rounding  = 6.0f ;

    // Color settings
    constexpr float
        // r = 0.20, g = 0.20, b = 0.25 ,  // dark blue-grey
        r = 0.20, g = 0.10, b = 0.20,      // pink-brown
        k  = 0.7,
        k2 = 1.8 ,
        k3 = 3.0 ;

    const ImVec4
        active             = { 3.0*r, g, b, 1.00f },
        window_title       = { 2.0*r, 2.0*g, 2.0*b, 1.0 },
        window_background  = { r,g,b,1.0 } ,
        frame_background   = { k*r, k*g, k*b, 1.0 },
        header             = { 0.0, 0.0, 0.0, 1.0 },
        slider_grab        = { k2*r, k2*g, k2*b, 1.0 },
        slider_grab_active = { 1.0, 1.0, 1.0, 1.0 },
        separator          = { k3*r, k3*g, k3*b, 1.0 };

    // Set font used
    const std::string font_name_ruda  = "../fonts/ruda/Ruda-Bold.ttf" ;
    io.Fonts->AddFontFromFileTTF( font_name_ruda.c_str(), font_size );
    
    // Set the base color theme, from which we modify colors
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();
    //ImGui::StyleColorsLight() ;

    // configure the style object

    ImGuiStyle &  style = GetStyle();

    style.FrameBorderSize   = 0.0 ;  // CUA: revisar
    style.WindowMenuButtonPosition = ImGuiDir_None ;  // disallows windows collapsing
    style.WindowTitleAlign  = ImVec2( 0.5, 0.5 );  // center window title
    style.WindowPadding     = ImVec2(15*f, 15*f);
    style.WindowRounding    = rounding ;
    style.FramePadding      = ImVec2(5*f, 5*f);
    style.FrameRounding     = rounding;
    style.ItemSpacing       = ImVec2(12*f, 8*f);
    style.ItemInnerSpacing  = ImVec2(8*f, 6*f);
    style.IndentSpacing     = 25.0f*f;
    style.ScrollbarSize     = 15.0f*f;
    style.ScrollbarRounding = rounding;
    style.GrabMinSize       = 15.0f*f ;// 5.0f;
    style.GrabRounding      = rounding;

    style.Colors[ImGuiCol_WindowBg]         = window_background ;
    style.Colors[ImGuiCol_ChildBg]          = window_background ;

    style.Colors[ImGuiCol_TitleBg]          = window_title ;
    style.Colors[ImGuiCol_TitleBgCollapsed] = window_title ;
    style.Colors[ImGuiCol_TitleBgActive]    = window_title ;

    style.Colors[ImGuiCol_Separator]        = separator ;

    style.Colors[ImGuiCol_Header]           = header ;
    style.Colors[ImGuiCol_HeaderHovered]    = header ;
    style.Colors[ImGuiCol_HeaderActive]     = active ;

    style.Colors[ImGuiCol_FrameBg]          = frame_background ;
    style.Colors[ImGuiCol_FrameBgHovered]   = frame_background ;
    style.Colors[ImGuiCol_FrameBgActive]    = active ;

    style.Colors[ImGuiCol_Button]           = frame_background ;
    style.Colors[ImGuiCol_ButtonHovered]    = frame_background ;
    style.Colors[ImGuiCol_ButtonActive]     = active ;

    style.Colors[ImGuiCol_SliderGrab]       = slider_grab ;
    style.Colors[ImGuiCol_SliderGrabActive] = slider_grab_active ;
}

// -------------------------------------------------------------------------

IMGUIContext::IMGUIContext( Instance * instance, Device * p_device, Surface * p_surface, RenderPass * render_pass, 
                    SwapChain * swap_chain, GLFWContext * glfw_state ) 
{

    device = p_device ;
    surface = p_surface ;

    assert( instance != nullptr );
    assert( device != nullptr );  
    assert( surface != nullptr );
    assert( render_pass != nullptr );
    assert( swap_chain != nullptr );
    assert( glfw_state != nullptr );

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForVulkan( glfw_state->glfw_window, true);

    
    init_info.ApiVersion = api_version_in_use;
    init_info.Instance = instance->vk_instance;
    init_info.PhysicalDevice = device->vk_gpu;
    init_info.Device = device->vk_device;
    init_info.QueueFamily = 0;
    init_info.Queue = device->vk_queue;
    init_info.MinImageCount = surface->vk_capabilities.minImageCount;
    init_info.ImageCount = swap_chain->imageCount;
    init_info.PipelineInfoMain.RenderPass = render_pass->vk_render_pass ;
    init_info.PipelineInfoMain.Subpass = 0;
    init_info.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

    const VkDescriptorPoolSize imguiPoolSizes[] = {
        {VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
        {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
        {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
        {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}
    };

    dpci.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    dpci.maxSets = 1000 * 11;
    dpci.poolSizeCount = 11;
    dpci.pPoolSizes = imguiPoolSizes;
    vkCreateDescriptorPool( device->vk_device, &dpci, nullptr, &imguiDescriptorPool ) ;
    init_info.DescriptorPool = imguiDescriptorPool;
    ImGui_ImplVulkan_Init( &init_info );

    // fijar el estilo, funciona en Linux, en macOs ??
    setStyle() ; 
    
}
// -------------------------------------------------------------------------

// called after the window is resized and the swap chain has been recreated 
void IMGUIContext::windowResized( SwapChain * swap_chain )
{
    assert( swap_chain != nullptr );
    ImGui_ImplVulkan_SetMinImageCount( swap_chain->min_image_count );
    resetImguiWindowPos = true;
}
// -------------------------------------------------------------------------
// return the current GUI width, to the right of the window.

uint32_t IMGUIContext::getGUIWidth()
{
    // Use ImGui logical display size and scale up to physical pixels for Vulkan.
    // On macOS Retina, surface extent is in physical pixels (2x) but ImGui uses logical pixels.
    const ImVec2 disp  = ImGui::GetIO().DisplaySize;
    const ImVec2 scale = ImGui::GetIO().DisplayFramebufferScale;
    const uint32_t logical_width = std::min( 450u, uint32_t(disp.x * 0.3f) );
    return uint32_t( logical_width * scale.x );
}
// -------------------------------------------------------------------------

// called each frame before invoking IMGUI functions to create the interface
void IMGUIContext::beginFrame()
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // sets the size and position based on the current surface extent  
    assert( surface != nullptr );

    // Use logical (GLFW) display size for ImGui positioning.
    // surface->getCurrentExtent() returns physical pixels (2x on Retina) which
    // would place the window completely off-screen in ImGui's logical coordinate space. 
    //(in a macOs, where physical and logical pixels differ.
    const ImVec2 display_size = ImGui::GetIO().DisplaySize;
    const uint32_t gui_width  = std::min( 450u, uint32_t(display_size.x * 0.3f) );

    ImGui::SetNextWindowPos( ImVec2( display_size.x, 0.0f ), 0, ImVec2( 1.0f, 0.0f ) );
    ImGui::SetNextWindowSize( ImVec2( float(gui_width), display_size.y ) );

    //ImGui::SetNextWindowPos( ImVec2(0.0f, 0.0f), resetImguiWindowPos ? ImGuiCond_Always : ImGuiCond_FirstUseEver );
    resetImguiWindowPos = false; // usado ??

    const ImGuiWindowFlags w_flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse ;
    

    ImGui::Begin("Options", nullptr, w_flags );
}

// -------------------------------------------------------------------------------------

// called each frame after IMGUI functions to create the interface, and before rendering the frame
// adds the commands to vk_cmd_buffer 
void IMGUIContext::endFrame( VkCommandBuffer & vk_cmd_buffer ) 
{
    ImGui::End();
    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), vk_cmd_buffer );
}
// -------------------------------------------------------------------------------------

IMGUIContext::~IMGUIContext()
{
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    vkDestroyDescriptorPool( device->vk_device, imguiDescriptorPool, nullptr);
    std::cout << "Deleted imgui state (IMGUI shutdown)" << std::endl ;
}

} // vkhc namespace end 


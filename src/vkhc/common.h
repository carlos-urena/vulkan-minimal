// Common includes and definitions for all headers and cpps

#pragma once 

#include <stdexcept>
#include <iostream>
#include <vector>
#include <span>
#include <array>
#include <cstring>
#include <algorithm>


#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp> // glm::vec2
#include <glm/gtc/type_ptr.hpp> // glm::value_ptr
#include <glm/gtx/transform.hpp>


#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

#include <shaderc/shaderc.hpp>


// declaration of shader sources for the basic pipeline (move out of here! - must be in the pipeline using them))
extern const char* vertShaderSrc ;
extern const char* fragShaderSrc ;
extern const char* tescShaderSrc ;
extern const char* teseShaderSrc ;

// common declarations inside the vkhc namespace 
namespace vkhc 
{

// -----------------------------------------------------------------------------
// foward class names declarations 

class GLFWContext;
class Instance ;
class Surface ;
class Device ;
class RenderPass ;
class SwapChain ;
class BasicPipeline ;
class CommandPoolAndBuffers ;
class SyncObjects ;
class IMGUIContext ;
class VertexBuffer ;
class VertexArray ;
class VulkanContext ;
class Texture ;

// -------------------------------------------------------------------------------------
// This is called after each time the user resizes the window  

void framebufferResizeCallback( GLFWwindow* window, int nx, int ny ) ;


// ----------------------------------------------------------------------------
// Auxiliary functions and declarations

// api in use
constexpr auto api_version_in_use = VK_API_VERSION_1_3 ;


// get the vendor name from the vendor Id
inline const char* getVendorName( uint32_t vendorId ) 
{
    switch (vendorId) {
        case 0x10DE: return "NVIDIA";
        case 0x1002:
        case 0x1022: return "AMD";
        case 0x8086: return "Intel";
        case 0x13B5: return "ARM";
        case 0x5143: return "Qualcomm";
        case 0x1010: return "Imagination Technologies";
        case 0x106B: return "Apple";
        default: return "Unknown vendor";
    }
}
// -----------------------------------------------------------------------------

} // end of namespace vkhc
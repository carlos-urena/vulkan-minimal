// Common includes and definitions for all headers and cpps

#pragma once 

#include <stdexcept>
#include <iostream>
#include <vector>
#include <span>
#include <array>
#include <cstring>
#include <algorithm>
#include <chrono>


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
const char* getVendorName( uint32_t vendorId ) ;

// assert macro
#define Assert( condition, msg ) vkhc::AssertFunction( condition, msg, __FILE__, __LINE__ )
void ErrorExitFunction( const std::string & msg, const char * file, const int line ) ;

// error exit function
#define ErrorExit( msg ) vkhc::ErrorExitFunction( msg, __FILE__, __LINE__ )
void AssertFunction( bool condition, const std::string & msg, const char * file, const int line )  ;

 // type for durations in seconds (used for frame time and animation speed)
typedef std::chrono::duration<float,std::ratio<1,1>> seconds_f ;


// register the time for first frame start
void InitFrameStart(); 

// returns the time in seconds elapsed between calls or from 'InitFrameStart()' if it is the first call. Also updates the time of the last frame start to the current time.
seconds_f NextFrameStart() ;


} // end of namespace vkhc
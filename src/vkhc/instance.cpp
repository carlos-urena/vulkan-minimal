// Implementation of Instance class

#include "instance.h"

namespace vkhc
{

// -----------------------------------------------------------------------------

Instance::Instance() 
{
    std::cout << "Creating Vulkan instance ..." << std::endl ;
    
    appInfo.apiVersion = api_version_in_use ;

    uint32_t count = 0 ;
    const char** ext = glfwGetRequiredInstanceExtensions( &count );

    if ( count == 0 )
        throw std::runtime_error("Failed to get required instance extensions from GLFW");

    std::cout << "Required instance extensions from GLFW: " << count << std::endl ;
    for ( uint32_t i = 0; i < count; ++i ) 
        std::cout << "  * " << ext[i] << std::endl;

    ci.pApplicationInfo        = &appInfo;
    ci.enabledExtensionCount   = count;
    ci.ppEnabledExtensionNames = ext;

    vkCreateInstance( &ci, nullptr, &vk_instance );
    std::cout << "Vulkan instance created." << std::endl ;
}

// -----------------------------------------------------------------------------

Instance::~Instance() 
{
    vkDestroyInstance( vk_instance , nullptr );
    std::cout << "Deleted instance." << std::endl ;
}

}; // end namespace vkhc

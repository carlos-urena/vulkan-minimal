// Declaration of Instance class

#pragma once 
#include <common.h>

namespace vkhc
{


class Instance 
{
    public:
    
    VkInstanceCreateInfo ci{ VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO } ;
    VkApplicationInfo    appInfo{ VK_STRUCTURE_TYPE_APPLICATION_INFO };
    VkInstance           vk_instance ;

    Instance() ;
    ~Instance() ;
} ;
} // end namespace vkhc
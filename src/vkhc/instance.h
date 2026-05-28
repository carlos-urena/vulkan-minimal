// Declaration of Instance class

#pragma once 
#include <common.h>

namespace vkhc
{


class Instance 
{
    public:
    
    VkInstanceCreateInfo ci{ } ;
    VkApplicationInfo    appInfo{  };
    VkInstance           vk_instance ;

    Instance() ;
    ~Instance() ;
} ;
} // end namespace vkhc
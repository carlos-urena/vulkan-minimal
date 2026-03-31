#pragma once 

#include <common.h>

namespace vkhc
{


class SyncObjects 
{
    public:

    VkSemaphore  imageAvailableSemaphore;
    VkSemaphore  renderFinishedSemaphore;
    VkFence      inFlightFence;

    VkSemaphoreCreateInfo sci1{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
    VkFenceCreateInfo fci{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };

    SyncObjects( Device * device ) ;    
} ;


} // fin del namespace vkhc 


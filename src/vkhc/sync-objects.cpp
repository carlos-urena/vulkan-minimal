// Implementation for class SyncObjects 

#include <common.h>
#include <sync-objects.h>
#include <device.h>

namespace vkhc
{

SyncObjects::SyncObjects( Device * device ) 
{
    assert( device != nullptr );
    VkDevice & vk_device = device->vk_device ;

    vkCreateSemaphore( vk_device, &sci1, nullptr, &imageAvailableSemaphore);
    vkCreateSemaphore( vk_device, &sci1, nullptr, &renderFinishedSemaphore);
    
    fci.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    vkCreateFence( vk_device, &fci, nullptr, &inFlightFence);
}


} // end vkhc namespace 


// Implementation for class SyncObjects 

#include <vkhc/common.h>
#include <vkhc/sync-objects.h>
#include <vkhc/device.h>

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


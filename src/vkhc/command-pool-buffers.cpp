// Declaration of class 'CommandPoolAndBuffers', 
//
// Encapsulates a command pool and a vector of command buffers, one for each swap chain image. 


#include <command-pool-buffers.h>
#include <device.h>
#include <swap-chain.h>

namespace vkhc
{

CommandPoolAndBuffers::CommandPoolAndBuffers( Device * p_device, SwapChain * p_swap_chain ) 
{
    device = p_device ;
    swap_chain = p_swap_chain ;
    assert( device != nullptr );
    assert( swap_chain != nullptr );

    VkCommandPoolCreateInfo cpci{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
    cpci.queueFamilyIndex = 0;
    cpci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vkCreateCommandPool( device->vk_device, &cpci, nullptr, &vk_pool );

    
    vk_cmdBuffers.resize( swap_chain->imageCount );

    VkCommandBufferAllocateInfo cbai{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
    cbai.commandPool        = vk_pool ;
    cbai.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cbai.commandBufferCount = swap_chain->imageCount ;

    vkAllocateCommandBuffers( device->vk_device, &cbai, vk_cmdBuffers.data());

    std::cout << "Created command pool and allocated " << swap_chain->imageCount << " command buffers" << std::endl ;
    
}
// --------------------------------------------------------------------------------
// recreate command buffers when the swap chain is recreated (e.g. when the window is resized)

void CommandPoolAndBuffers::recreate( SwapChain * swap_chain ) 
{
    assert( swap_chain != nullptr );
    assert( device != nullptr );

    // delete command buffers (moved)
    if ( ! vk_cmdBuffers.empty()) 
    {
        vkFreeCommandBuffers( device->vk_device, vk_pool, 
                            static_cast<uint32_t>( vk_cmdBuffers.size()), vk_cmdBuffers.data());
        vk_cmdBuffers.clear();
    }

    VkCommandBufferAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
    allocInfo.commandPool = vk_pool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = swap_chain->imageCount;

    vk_cmdBuffers.resize(swap_chain->imageCount);
    if (vkAllocateCommandBuffers( device->vk_device, &allocInfo, vk_cmdBuffers.data()) != VK_SUCCESS )
        throw std::runtime_error( "Failed to reallocate command buffers" );
}
// --------------------------------------------------------------------------------

CommandPoolAndBuffers::~CommandPoolAndBuffers() 
{
    if (! vk_cmdBuffers.empty())
        vkFreeCommandBuffers( device->vk_device, vk_pool, static_cast<uint32_t>(vk_cmdBuffers.size()), vk_cmdBuffers.data());
    vkDestroyCommandPool( device->vk_device, vk_pool, nullptr);

    std::cout << "Deleted command buffers and command pool" << std::endl ;
    
}
// --------------------------------------------------------------------------------
// get a command buffer at any given index. 
VkCommandBuffer CommandPoolAndBuffers::resetAndGetCommandBuffer( uint32_t imageIndex ) 
{
    assert( imageIndex < vk_cmdBuffers.size() );
    vkResetCommandBuffer( vk_cmdBuffers[imageIndex], 0 );
    return vk_cmdBuffers[imageIndex];
}


VkCommandBuffer CommandPoolAndBuffers::beginSingleTimeCommands(  ) 
{
    assert( device  != nullptr );
    assert( device->vk_device != VK_NULL_HANDLE );
    assert( vk_pool != VK_NULL_HANDLE );

    

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = vk_pool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers( device->vk_device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void CommandPoolAndBuffers::endSingleTimeCommands( VkCommandBuffer vk_cmd ) 
{
    assert( device  != nullptr );
    assert( device->vk_device != VK_NULL_HANDLE );
    assert( device->vk_queue != VK_NULL_HANDLE );
     
    assert( vk_pool != VK_NULL_HANDLE );
    assert( vk_cmd != VK_NULL_HANDLE );

    vkEndCommandBuffer( vk_cmd );

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &vk_cmd;

    // Submit to the queue and wait
    vkQueueSubmit( device->vk_queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle( device->vk_queue );

    // Free the temporary buffer immediately
    vkFreeCommandBuffers( device->vk_device, vk_pool, 1, &vk_cmd );
}

} // vkhc namespace end 


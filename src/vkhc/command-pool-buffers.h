// Declaration of class 'CommandPoolAndBuffers', 
//
// Encapsulates a command pool and a vector of command buffers, one for each swap chain image.

#pragma once 

#include <common.h>


namespace vkhc
{



class CommandPoolAndBuffers 
{
    public:

    VkCommandPool                vk_pool;
    std::vector<VkCommandBuffer> vk_cmdBuffers ;
    Device *                     device = nullptr ;
    SwapChain *                  swap_chain = nullptr ;

    // Constructor
    CommandPoolAndBuffers( Device * p_device, SwapChain * p_swap_chain ) ;
    
    // Recreate command buffers when the swap chain is recreated (e.g. when the window is resized)
    void recreate( SwapChain * swap_chain ) ;
    
    // Destructor
    ~CommandPoolAndBuffers() ;
    
    // get a command buffer at any given index. 
    VkCommandBuffer resetAndGetCommandBuffer( uint32_t imageIndex ) ;

    // produces a single-use command buffer
    VkCommandBuffer beginSingleTimeCommands(  ) ;

    // submits a single use command buffer
    void endSingleTimeCommands( VkCommandBuffer commandBuffer );
    
} ;




} // vkhc namespace end 


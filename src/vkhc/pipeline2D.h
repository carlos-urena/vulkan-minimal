// Declaration of the class 'BasicPipeline2D' 
//
// Derived from 'BasicPipeline' 
// Encapsulates a simple 2D graphics pipeline, including vertex and 
// fragment shaders, but not tesselation or vertex shaders.

#pragma once 

#include <common.h>
#include <pipeline.h>


namespace vkhc
{

// -------------------------------------------------------------------------------
// Graphics pipeline state.

class BasicPipeline2D : public BasicPipeline
{
    public:
    BasicPipeline2D( VulkanContext & vulkan_context ) ; 

    // state which can be changed before sending commands
    void setViewMatrix( const glm::mat4 & view_mat ) ;
    void setProjectionMatrix( const glm::mat4 & proj_mat ) ;

    // dynamic state (can be changed dynamically in command buffers, without re-creating the pipeline)
    void setModelMatrix( VkCommandBuffer & vk_cmd, const glm::mat4 & model_mat ) ;
    void setTextureIndex( VkCommandBuffer & vk_cmd, int index ) ;
    
} ; // end class 'BasicPipeline2D' 

// ------------------------------------------------------------------------------

} // end namespace 'vkhc' 


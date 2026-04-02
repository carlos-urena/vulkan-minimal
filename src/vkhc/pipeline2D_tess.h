// Declaration of the class 'BasicPipeline2D' 
//
// A type of basic pipeline oriented towards 2D rendering

#pragma once 

#include <common.h>
#include <pipeline.h>


namespace vkhc
{

// -------------------------------------------------------------------------------
// Graphics pipeline state.

class Pipeline2DTess : public BasicPipeline
{
    public:
    Pipeline2DTess( VulkanContext & vulkan_context ) ; 

    void setViewMatrix( const glm::mat4 & view_mat ) ;
    void setProjectionMatrix( const glm::mat4 & proj_mat ) ;

    void setTextureIndex( VkCommandBuffer & vk_cmd, int index ) ;
    void setModelMatrix( VkCommandBuffer & vk_cmd, const glm::mat4 & model_mat ) ;

} ; // end class 'BasicPipeline2D' 

// ------------------------------------------------------------------------------

} // end namespace 'vkhc' 


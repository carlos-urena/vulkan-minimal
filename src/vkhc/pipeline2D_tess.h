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
    Pipeline2DTess( VulkanContext & vulkan_context,
                    const int p_num_vertexes_per_patch,
                    bool p_depth_test_enabled = true,
                    bool p_depth_write_enabled = true,
                    VkCompareOp p_depth_compare_op = VK_COMPARE_OP_LESS ) ; 

    void setViewMatrix( const glm::mat4 & view_mat ) ;
    void setProjectionMatrix( const glm::mat4 & proj_mat ) ;

    void setTextureIndex( VkCommandBuffer & vk_cmd, int index ) ;
    void setModelMatrix( VkCommandBuffer & vk_cmd, const glm::mat4 & model_mat ) ;

    int num_vertexes_par_patch ; // number of vertexes per patch (3 for triangles, 4 for quads)

} ; // end class 'BasicPipeline2D' 

// ------------------------------------------------------------------------------

} // end namespace 'vkhc' 


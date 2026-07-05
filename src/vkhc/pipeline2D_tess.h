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
                    const bool p_z_buffer_enabled  );  

    void setViewMatrix( const glm::mat4 & view_mat ) ;
    void setProjectionMatrix( const glm::mat4 & proj_mat ) ;

    void setTextureIndex( VkCommandBuffer & vk_cmd, int index ) ;
    void setModelMatrix( VkCommandBuffer & vk_cmd, const glm::mat4 & model_mat ) ;

    int num_vertexes_par_patch ; // number of vertexes per patch (3 for triangles, 4 for quads)

} ; // end class 'BasicPipeline2D' 

// ------------------------------------------------------------------------------

} // end namespace 'vkhc' 


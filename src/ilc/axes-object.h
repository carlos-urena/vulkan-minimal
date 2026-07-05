#pragma once


#include <indexed-mesh.h>


// a class for a solid cilinder in 01

class CylinderZ01 ;

class AxesObject : public DrawableObject
{
    private:
    CylinderZ01 * axes_cylinder = nullptr ;  // a solid cilinder in 01, used to visualize the axes of the 3D space.
    
    int 
        red_color_index = -1, 
        green_color_index = -1, 
        blue_color_index = -1 ;  // indices of the base colors used to visualize the axes

    public:
    AxesObject( ) ;
    ~AxesObject() ;
    virtual void drawVK( vkhc::BasicPipeline * pipeline, vkhc::VulkanContext & context, VkCommandBuffer & cmd_vk ) override ;

} ;
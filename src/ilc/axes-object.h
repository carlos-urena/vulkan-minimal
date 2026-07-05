#pragma once


#include <indexed-mesh.h>


// a class for a solid cilinder in 01

class CylinderZ01 ;

class AxesObject : public DrawableObject
{
    private:
    CylinderZ01 * axes_cylinder = nullptr ;  // a solid cilinder in 01, used to visualize the axes of the 3D space.
    
    public:
    AxesObject( ) ;
    ~AxesObject() ;
    virtual void drawVK( vkhc::VulkanContext & context, VkCommandBuffer & cmd_vk ) override ;

} ;
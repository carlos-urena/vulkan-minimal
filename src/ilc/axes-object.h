#pragma once


#include <indexed-mesh.h>


// aux helper classes used to debug

class CylinderZ01 ;
class IMTriangle ;
class IMSphere ;

class AxesObject : public DrawableObject
{
    private:
    CylinderZ01 * axes_cylinder = nullptr ;  // a solid cilinder in 01, used to visualize the axes of the 3D space.
    IMTriangle * xtri = nullptr, * ytri = nullptr, * ztri = nullptr ; // triangles used to visualize the axes of the 3D space.  
    IMSphere * sphere = nullptr ; // a solid sphere, used to test normals, lighting and relative scalings in X, Y, Z
    int 
        red_color_index = -1, 
        green_color_index = -1, 
        blue_color_index = -1 ;  // indices of the base colors used to visualize the axes

    public:
    AxesObject( ) ;
    ~AxesObject() ;
    virtual void drawVK( vkhc::BasicPipeline * pipeline, vkhc::VulkanContext & context, VkCommandBuffer & cmd_vk ) override ;

} ;
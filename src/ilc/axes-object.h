#pragma once


#include <pipeline3D.h>
#include <indexed-mesh.h>


// aux helper classes used to debug

class CylinderZ01 ;
class ConeZ01 ;
//class IMTriangle ;
class IMSphere ;
class Segment ;

class AxesObject : public DrawableObject
{
    private:
    CylinderZ01 
        * axes_cylinder = nullptr ;  // a solid cilinder in 01, used to visualize the axes of the 3D space.
    ConeZ01 
        * axes_cone = nullptr ;  // a solid cone in 01, used to visualize the axes of the 3D space.
    //IMTriangle * xtri = nullptr, * ytri = nullptr, * ztri = nullptr ; // triangles used to visualize the axes of the 3D space.  
    IMSphere 
        * sphere = nullptr ; // a solid sphere, used to test normals, lighting and relative scalings in X, Y, Z
    Segment 
        *line_x = nullptr ,
        *line_y = nullptr , 
        *line_z = nullptr ,
        * line01z = nullptr ;
    int 
        red_color_index = -1, 
        green_color_index = -1, 
        blue_color_index = -1 ;  // indices of the base colors used to visualize the axes

    bool draw_axes = true ;
    bool draw_grid = true ;

    void drawGridVK( vkhc::Pipeline3D * pipeline, vkhc::VulkanContext & context, VkCommandBuffer & cmd_vk ) ;
    void drawAxesVK( vkhc::Pipeline3D * pipeline, vkhc::VulkanContext & context, VkCommandBuffer & cmd_vk ) ;


    public:
    AxesObject( ) ;
    ~AxesObject() ;
    void setActive( bool p_draw_axes, bool p_draw_grid ) ;
    virtual void drawVK( vkhc::BasicPipeline * pipeline, vkhc::VulkanContext & context, VkCommandBuffer & cmd_vk ) override ;

} ;
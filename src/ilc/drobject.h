    

#pragma once

#include <string>
#include <vkhc/common.h>
#include <ilc/materials.h>

namespace ilc 
{

class DrawableObject 
{
    private:
        std::string name_str ;
        

    public:
        virtual void drawVK( vkhc::BasicPipeline * pipeline, vkhc::VulkanContext & context, VkCommandBuffer & cmd_vk ) = 0 ;
        virtual ~DrawableObject() {} ;
        virtual void drawIMGUIWidgets( )  ;
        const std::string & getName() const  ;
        void setName( const std::string & new_name ) ;
      
} ;
// -------------------------------------------------------------------------------------------------------------

class TransformedObject : public DrawableObject
{
    private:

    glm::mat4 tr_mat, tr_mat_normals ; 
    DrawableObject * drawable_object = nullptr ; // pointer to the drawable object to be drawn with the transformation matrix 'tr_mat'

    public:

    TransformedObject( const glm::mat4 & p_tr_mat, DrawableObject * p_drawable_object ) ;
    virtual void drawVK( vkhc::BasicPipeline * pipeline, vkhc::VulkanContext & context, VkCommandBuffer & cmd_vk ) override ;
} ;
// -------------------------------------------------------------------------------------------------------------


class CompositeObject : public DrawableObject
{
    private:
    
    std::vector<DrawableObject *> objects ; // vector of pointers to drawable objects

    public:

    void add( DrawableObject * obj ) ;
    virtual void drawVK( vkhc::BasicPipeline * pipeline, vkhc::VulkanContext & context, VkCommandBuffer & cmd_vk ) override ;
        
} ;

// -------------------------------------------------------------------------------------------------------------


class MaterialObject : public DrawableObject
{
    private:

    int material_index = -1 ; // index of the material in the materials set (once added, in the constructor)
    ilc::MaterialsSet * materials_set = nullptr ; // pointer to the materials set to which the material belongs
    DrawableObject * drawable_object = nullptr ; // pointer to the drawable object to be drawn with the material

    public:
    uint32_t getMaterialIndex() const { return material_index ; } // returns the index of this material in the materials set (or -1 if not yet added to any set)

    // initializes the material object by crating a new material in the materials set, and storing its index in 'material_index'
    MaterialObject( ilc::Material * p_material, ilc::MaterialsSet * p_materials_set, DrawableObject * p_drawable_object ) ;

    // activates the material, draws the object, and then resets the state
    virtual void drawVK( vkhc::BasicPipeline * pipeline, vkhc::VulkanContext & context, VkCommandBuffer & cmd_vk ) override ;
} ;

// -------------------------------------------------------------------------------------------------------------

}  // end namespace ilc 
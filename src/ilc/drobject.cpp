#include <ilc/drobject.h>
#include <vkhc/pipeline3D.h>


namespace ilc 
{

const std::string & DrawableObject::getName() const 
{
    return name_str ;
}

void DrawableObject::setName( const std::string & new_name )
{
    name_str = new_name ;
}

void DrawableObject::drawIMGUIWidgets( ) 
{
    // default implementation does nothing
}

// -------------------------------------------------------------------------------------------------------------

TransformedObject::TransformedObject( const glm::mat4 & p_tr_mat, DrawableObject * p_drawable_object ) 
{
    drawable_object = p_drawable_object ; Assert( drawable_object != nullptr, "TransformedObject constructor: 'drawable_object' instance is null !!" ) ;
    tr_mat = p_tr_mat ;
}

// -------------------------------------------------------------------------------------------------------------

void TransformedObject::drawVK( vkhc::BasicPipeline * pipeline, vkhc::VulkanContext & context, VkCommandBuffer & cmd_vk ) 
{
    using namespace vkhc ;

    Pipeline3D * p = static_cast<Pipeline3D *>( pipeline ) ;
    Assert( p != nullptr, "TransformedObject::drawVK: pipeline is not a Pipeline3D instance !!" ) ;

    p->pushModelMatrix( cmd_vk, tr_mat ) ; // push the transformation matrix for the object to be drawn
    
    drawable_object->drawVK( p, context, cmd_vk ) ; // draw the object with the transformation matrix
    p->popModelMatrix( cmd_vk ) ; // pop the transformation matrix for the object to be drawn
    
}

// -------------------------------------------------------------------------------------------------------------

void CompositeObject::add( DrawableObject * obj ) 
{ 
    Assert( obj != nullptr, "CompositeObject::add: drawable object is null !!" ) ;
    objects.push_back( obj ) ; 
} 

// -------------------------------------------------------------------------------------------------------------

void CompositeObject::drawVK( vkhc::BasicPipeline * pipeline, vkhc::VulkanContext & context, VkCommandBuffer & cmd_vk )
{
    for ( auto obj : objects )
    {
        Assert( obj != nullptr, "CompositeObject::drawVK: drawable object is null !!" ) ;
        obj->drawVK( pipeline, context, cmd_vk ) ;
    }
}

// -------------------------------------------------------------------------------------------------------------


MaterialObject::MaterialObject( ilc::Material * p_material, ilc::MaterialsSet * p_materials_set, DrawableObject * p_drawable_object ) 
{
    Assert( p_material != nullptr, "MaterialObject constructor: 'p_material' instance is null !!" ) ;
    Assert( p_materials_set != nullptr, "MaterialObject constructor: 'p_materials_set' instance is null !!" ) ;
    Assert( p_drawable_object != nullptr, "MaterialObject constructor: 'p_drawable_object' instance is null !!" ) ;

    materials_set   = p_materials_set ;
    material_index  = (int)materials_set->add( *p_material ) ; 
    Assert( material_index >= 0, "MaterialObject constructor: cannot add material to materials set !!" ) ;   
    drawable_object = p_drawable_object ;
}

// -------------------------------------------------------------------------------------------------------------

void MaterialObject::drawVK( vkhc::BasicPipeline * pipeline, vkhc::VulkanContext & context, VkCommandBuffer & cmd_vk )
{
    // get a 3d pipeline or die
    vkhc::Pipeline3D * p = static_cast<vkhc::Pipeline3D *>( pipeline ) ;
    Assert( p != nullptr, "MaterialObject::drawVK: pipeline is not a Pipeline3D instance !!" ) ;

    // get previous state 
    int prev_base_color_index  = p->getBaseColorIndex() ;
    int prev_texture_index     = p->getTextureIndex() ;
    int prev_brdf_params_index = p->getBrdfParamsIndex() ;

    // set the state for this material 
    materials_set->activate( cmd_vk, *p, material_index ) ; // activates the material in the pipeline, by setting the push constants and UBOs in the shaders

    // draw the object with the material
    drawable_object->drawVK( p, context, cmd_vk ) ;

    // restore state 
    p->setBaseColorIndex( cmd_vk, prev_base_color_index ) ;
    p->setTextureIndex( cmd_vk, prev_texture_index ) ;
    p->setBrdfParamsIndex( cmd_vk, prev_brdf_params_index ) ;
}

} // end namespace ilc ;
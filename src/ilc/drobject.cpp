#include <ilc/drobject.h>
#include <vkhc/pipeline3D.h>

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


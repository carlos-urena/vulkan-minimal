#include <imgui.h>

#include <vkhc/textures.h>
#include <ilc/materials.h>



namespace ilc 
{



// ------------------------------------------------------------------------------
// builds a material from the base color and the coefficients of the Blinn-Phong model

Material::Material( const glm::vec3 & p_base_color, const vkhc::BrdfParams & p_brdf_params ) 
{
    base_color     = p_base_color ;
    use_base_color = true ;
    use_texture    = false ;
    brdf_params    = p_brdf_params ;   
}
// ------------------------------------------------------------------------------
// builds a material from a texture and the coefficients of the Blinn-Phong model
    
Material::Material( const std::string & p_texture_path, const vkhc::BrdfParams & p_brdf_params ) 
{
    use_base_color = false ;
    texture_path   = p_texture_path ;
    use_texture    = true ;
    brdf_params    = p_brdf_params ;
}

// ------------------------------------------------------------------------------
// builds a material with no base color (uses vertex colors) and the coefficients of the Blinn-Phong model

Material::Material( const vkhc::BrdfParams & p_brdf_params ) 
{
    use_base_color = false ;
    use_texture    = false ;
    brdf_params    = p_brdf_params ;
}

bool Material::drawIMGUIWidgets( const std::string & title ) 
{
    using namespace ImGui ;

    bool changed = false ;

    if (use_base_color)
    {
        ColorEdit3("Base color", value_ptr(base_color));
        using namespace std ;
        cout << "Material::drawIMGUIWidgets: base color = " << base_color.r << ", " << base_color.g << ", " << base_color.b << endl ;   
        changed = true ;
    }
    else
    {
        Text("No base color (using vertex colors or texture)");
    }

        // if (use_texture)
        // {
        //     Text("Texture path: %s", texture_path.c_str());
        // }
        // else
        // {
        //     Text("No texture");
        // }

        // SliderFloat("ka (ambient)", &brdf_params.ka, 0.0f, 1.0f);
        // SliderFloat("kd (diffuse)", &brdf_params.kd, 0.0f, 1.0f);
        // SliderFloat("ks (specular)", &brdf_params.ks, 0.0f, 1.0f);
        // SliderFloat("exp (shininess)", &brdf_params.exp, 1.0f, 128.0f);
    
    return changed ;
}
// ------------------------------------------------------------------------------
// Materials sets


MaterialsSet::MaterialsSet( vkhc::VulkanContext * p_context ) 
{
    assert( p_context != nullptr );
    assert( p_context->device != nullptr );
    assert( p_context->device->vk_device != VK_NULL_HANDLE );
    context = p_context ;

    textures_set     = new vkhc::TexturesSet( context ) ; Assert( textures_set != nullptr, "MaterialsSet::MaterialsSet: cannot create a TexturesSet" ) ;
    base_colors_set  = new vkhc::BaseColorsSet( ) ;       Assert( base_colors_set != nullptr, "MaterialsSet::MaterialsSet: cannot create a BaseColorsSet" ) ;
    brdfs_params_set = new vkhc::BrdfParamsSet( ) ;       Assert( brdfs_params_set != nullptr, "MaterialsSet::MaterialsSet: cannot create a BrdfParamsSet" ) ;
}

// ------------------------------------------------------------------------------

uint32_t MaterialsSet::add( const Material & material ) 
{
    using namespace std ;
    cout << "MaterialsSet::add: adding material with context == " << context << endl ;
    Assert( context != nullptr, "MaterialsSet::add: 'context' instance is null !!" );
    Assert( material.materials_set == nullptr, "Material::add: this material is already part of a materials set !!" );

    materials.push_back( material ) ;
    return static_cast<uint32_t>( materials.size() - 1 ) ; // return index of the added material
}
// ------------------------------------------------------------------------------

void MaterialsSet::updateMaterial( const uint32_t material_index, const Material & newm ) 
{
    Assert( newm.materials_set == this, "MaterialsSet::updateMaterial: the new material does not belong to this set !!" ) ; 
    Assert( material_index < materials.size(), "MaterialsSet::updateMaterial: the material index is out of bounds !!" ) ;
    Assert( pipeline3D != nullptr, "MaterialsSet::updateMaterial: the materials set is not bound to a pipeline !!" ) ;
    
    Material & currm = materials[ material_index ] ; // get the current material 

    // if the material uses a base color, update it. 
    if ( newm.use_base_color )
    {
        Assert( base_colors_set != nullptr, "MaterialsSet::updateMaterial: 'base_colors_set' instance is null !!" ) ;
        Assert( currm.color_base_index == newm.color_base_index, "MaterialsSet::updateMaterial: the base color index of the new material does not match the current material !!" ) ;
        Assert( newm.color_base_index >= 0, "MaterialsSet::updateMaterial: the new material has no base color index !!" ) ;
        
        glm::vec4 new_base_color = glm::vec4( newm.base_color, 1.0f ) ;
        base_colors_set->colors[currm.color_base_index] = new_base_color; 
        materials[ material_index ].base_color = newm.base_color ; 
        pipeline3D->updateBaseColor( currm.color_base_index, new_base_color ) ;

        using namespace std ;
        cout << "MaterialsSet::updateMaterial: updated base color at index " << currm.color_base_index << " to new value: " 
             << new_base_color.r << ", " << new_base_color.g << ", " << new_base_color.b << ", " << new_base_color.a << endl ;
    }   
}

// ------------------------------------------------------------------------------

void MaterialsSet::bindTo( vkhc::Pipeline3D * p_pipeline3D ) 
{
    
    Assert( context != nullptr,          "MaterialsSet::bindTo: 'context' instance is null !!" );
    Assert( context->device != nullptr , "MaterialsSet::bindTo: 'context->device' instance is null !!" );
    Assert( pipeline3D == nullptr ,      "MaterialsSet::bindTo: this materials set is already bound to a pipeline !!" );
    Assert( textures_set != nullptr,     "MaterialsSet::bindTo: 'textures_set' instance is null !!" );
    Assert( base_colors_set != nullptr,  "MaterialsSet::bindTo: 'base_colors_set' instance is null !!" );
    Assert( brdfs_params_set != nullptr,  "MaterialsSet::bindTo: 'brdfs_params_set' instance is null !!" );
    Assert( p_pipeline3D != nullptr,     "MaterialsSet::bindTo: 'p_pipeline3D' instance is null !!" );

    pipeline3D = p_pipeline3D ;
    

    // Create the indexes for the base colors and textures for each material in the set.
    
    for( uint32_t i=0; i<materials.size(); i++ )
    {
        Material & m = materials[i] ;
        m.materials_set = this ; // set the pointer to this materials set

        m.color_base_index = -1 ; 
        m.texture_index = -1 ;

        m.brdf_params_index = brdfs_params_set->add( m.brdf_params ) ; // add the BRDF params to the set, and get its index in the set (the index is the last one, which is 'brdfs_params_set->size()-1')

        if ( m.use_base_color )
            m.color_base_index = base_colors_set->add( m.base_color ) ;
        if ( m.use_texture )
            m.texture_index = textures_set->add( m.texture_path ) ;
    }

    // Copy the arrays to the corresponding  UBO uniforms in the pipeline

    textures_set->bindTo( *pipeline3D ) ;
    pipeline3D->setBaseColorsSet( *base_colors_set ) ;
    pipeline3D->setBrdfParamsSet( *brdfs_params_set ) ;
}

void MaterialsSet::activate( VkCommandBuffer & vk_cmd, 
                             vkhc::Pipeline3D & pipeline, 
                             const uint32_t material_index ) 
{
    using namespace std ;

    assert( material_index < materials.size() ) ;
    Material & m = materials[ material_index ] ;
    
    // cout << "Material::activate: activating material with index == " << material_index <<   endl 
    //      << "  base color        " << m.base_color.r << ", " << m.base_color.g << ", " << m.base_color.b << endl
    //      << "  use base color    " << m.use_base_color << endl
    //      << "  texture path      " << m.texture_path << endl
    //      << "  use texture       " << m.use_texture << endl
    //      << "  BRDF params:      ka=" << m.brdf_params.ka << ", kd=" << m.brdf_params.kd << ", ks=" << m.brdf_params.ks << ", exp=" << m.brdf_params.exp << endl
    //      << "  base color index  " << m.color_base_index << endl 
    //      << "  texture index     " << m.texture_index << endl 
    //      << "  BRDF params index " << m.brdf_params_index << endl ;

    pipeline.setBaseColorIndex( vk_cmd, m.color_base_index ) ;
    pipeline.setTextureIndex( vk_cmd, m.texture_index ) ;
    pipeline.setBrdfParamsIndex( vk_cmd, m.brdf_params_index ) ;
}

// ------------------------------------------------------------------------------
} // end namespace ilc
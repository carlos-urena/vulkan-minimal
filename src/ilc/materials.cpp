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
    using namespace std ;

    bool changed = false ;
    PushID(title.c_str()) ; // this is because we may have multiple materials in the same IMGUI window, and we want to avoid ID collisions

    // draw sliders for the BRDF params 
    if ( SliderFloat("Ka", &brdf_params.ka, 0.0f, 1.0f ) ) 
    {
        cout << "Material::drawIMGUIWidgets: new Ka = " << brdf_params.ka << endl ;
        changed = true ;
    }
    if ( SliderFloat("Kd", &brdf_params.kd, 0.00f, 1.0f ) ) 
    {
        cout << "Material::drawIMGUIWidgets: new Kd = " << brdf_params.kd << endl ;
        changed = true ;
    }
    if ( SliderFloat("Ks", &brdf_params.ks, 0.0f, 1.0f ) ) 
    {
        cout << "Material::drawIMGUIWidgets: new Ks = " << brdf_params.ks << endl ;
        changed = true ;
    }
    if ( SliderFloat("Exp", &brdf_params.exp, 1.0f, 128.0f, "%.1f") ) 
    {
        cout << "Material::drawIMGUIWidgets: new Exp = " << brdf_params.exp << endl ;
        changed = true ;
    }

    // draw widget for the base color, if it has one 
    if (use_base_color)
    {
        if ( ColorEdit3("Base color", value_ptr(base_color)) )
        {
            cout << "Material::drawIMGUIWidgets: base color = " << base_color.r << ", " << base_color.g << ", " << base_color.b << endl ;   
            changed = true ;
        }
    }
    

    PopID() ;
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

uint32_t MaterialsSet::add( Material & material ) 
{
    using namespace std ;
    cout << "MaterialsSet::add: adding material with context == " << context << endl ;
    Assert( context != nullptr, "MaterialsSet::add: 'context' instance is null !!" );
    Assert( material.materials_set == nullptr, "Material::add: this material is already part of a materials set !!" );
    Assert( material.material_index == -1, "Material::add: this material already has a valid index in a materials set !!" );

    const uint32_t pos = materials.size() ; 
    material.material_index = pos ; 
    material.materials_set = this ; // set the pointer to this materials set in the material

    materials.push_back( material ) ;
    return pos ; // return index of the added material
}
// ------------------------------------------------------------------------------

void MaterialsSet::updateMaterial( const uint32_t material_index, const Material & newm ) 
{
    using namespace std ;

    Assert( newm.materials_set == this, "MaterialsSet::updateMaterial: the new material does not belong to this set !!" ) ; 
    Assert( material_index < materials.size(), "MaterialsSet::updateMaterial: the material index is out of bounds !!" ) ;
    Assert( pipeline3D != nullptr, "MaterialsSet::updateMaterial: the materials set is not bound to a pipeline !!" ) ;
    
    Material & currm = materials[ material_index ] ; // get the current material 

    // Copy the new material BRDF params to its index in the BRDFs params array.

    Assert( brdfs_params_set != nullptr, 
            "MaterialsSet::updateMaterial: 'brdf_params_set' instance is null !!" ) ;

    Assert( currm.brdf_params_index == newm.brdf_params_index, 
            "MaterialsSet::updateMaterial: the BRDF params index has changed !" ) ;

    Assert( 0 <= currm.brdf_params_index && currm.brdf_params_index < (int) brdfs_params_set->brdfs_params.size(), 
            "MaterialsSet::updateMaterial: the new material BRDF params index is out of range" ) ;

    Assert( currm.use_base_color == newm.use_base_color, 
            "MaterialsSet::updateMaterial: the new material 'use_base_color' flag has changed !" ) ;
    Assert( currm.use_texture == newm.use_texture, 
            "MaterialsSet::updateMaterial: the new material 'use_texture' flag has changed !" ) ;

    // update the BRDF params of the material in the set, and in the pipeline
    brdfs_params_set->brdfs_params[ currm.brdf_params_index ] = newm.brdf_params ; // update the BRDF params in the set
    brdfs_params_set->brdfs_params_vec4[ currm.brdf_params_index ] = glm::vec4( newm.brdf_params.ka, newm.brdf_params.kd, newm.brdf_params.ks, newm.brdf_params.exp ) ; // update the BRDF params in the set
    currm.brdf_params = newm.brdf_params ; // update the BRDF params in the materials vector
    

    cout << "MaterialsSet::updateMaterial: updating BRDF params at index " << currm.brdf_params_index << " to new value: "
         << "ka=" << newm.brdf_params.ka << ", kd=" << newm.brdf_params.kd << ", ks=" << newm.brdf_params.ks << ", exp=" << newm.brdf_params.exp << endl ;

    pipeline3D->updateBrdfParams( currm.brdf_params_index, newm.brdf_params ) ;

    // Handle a material which uses a base color: copy the new base color to the array of base colors

    if ( newm.use_base_color )
    {
        Assert( base_colors_set != nullptr, 
                "MaterialsSet::updateMaterial: 'base_colors_set' instance is null !!" ) ;

        Assert( currm.use_base_color , "MaterialsSet::updateMaterial: the current material does not use a base color, but the new material does !!" ) ;
        Assert( currm.color_base_index == newm.color_base_index, 
                "MaterialsSet::updateMaterial: the base color index of the new material does not match the current material !!" ) ;

        Assert( 0 <= newm.color_base_index && newm.color_base_index < (int) base_colors_set->colors.size(), 
                "MaterialsSet::updateMaterial: the new material base color index is out of range !!" ) ;

        glm::vec4 new_base_color = glm::vec4( newm.base_color, 1.0f ) ;
        base_colors_set->colors[currm.color_base_index] = new_base_color; 
        currm.base_color = newm.base_color ; // update the base color in the materials vector
        

        cout << "MaterialsSet::updateMaterial: updating base color at index " << currm.color_base_index << " to new value: " 
             << new_base_color.r << ", " << new_base_color.g << ", " << new_base_color.b << ", " << new_base_color.a << endl ;

        pipeline3D->updateBaseColor( currm.color_base_index, new_base_color ) ;
    }

    // Handle a material which uses a texture: copy the new textura index to the array of textures
    ///// This isn't quite right designed and less tested !!!!

    // if ( newm.use_texture )
    // {
    //     Assert( textures_set != nullptr, 
    //             "MaterialsSet::updateMaterial: 'textures_set' instance is null !!" ) ;

    //     Assert( currm.use_texture , "MaterialsSet::updateMaterial: the current material does not use a texture, but the new material does !!" ) ;
    //     Assert( 0 <= newm.texture_index && newm.texture_index < (int) textures_set->textures.size(), 
    //             "MaterialsSet::updateMaterial: the new material texture index is out of range !!" ) ;
        
        
    //     currm.texture_path = newm.texture_path ; // update the texture path in the materials vector
    //     currm.texture_index = newm.texture_index ; // update the texture index in the materials vector

    //     cout << "MaterialsSet::updateMaterial: updating texture at index " << currm.texture_index << " to new value: " 
    //          << newm.texture_path << endl ;
    // }
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

// ------------------------------------------------------------------------------

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
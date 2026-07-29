#include <ilc/materials.h>

namespace ilc 
{

// ------------------------------------------------------------------------------

BrdfParams::BrdfParams( const float p_ka, const float p_kd, const float p_ks, const float p_exp ) 
{
    ka = p_ka ;
    kd = p_kd ;
    ks = p_ks ;
    exp = p_exp ;
}

// ------------------------------------------------------------------------------
// builds a material from the base color and the coefficients of the Blinn-Phong model

Material::Material( const glm::vec3 & p_base_color, const BrdfParams & p_brdf_params ) 
{
    base_color     = p_base_color ;
    use_base_color = true ;
    use_texture    = false ;
    brdf_params    = p_brdf_params ;   
}
// ------------------------------------------------------------------------------
// builds a material from a texture and the coefficients of the Blinn-Phong model

Material::Material( const std::string & p_texture_path, const BrdfParams & p_brdf_params ) 
{
    use_base_color = false ;
    texture_path   = p_texture_path ;
    use_texture    = true ;
    brdf_params    = p_brdf_params ;
}

// ------------------------------------------------------------------------------
// builds a material with no base color (uses vertex colors) and the coefficients of the Blinn-Phong model

Material::Material( const BrdfParams & p_brdf_params ) 
{
    use_base_color = false ;
    use_texture    = false ;
    brdf_params    = p_brdf_params ;
}
// ------------------------------------------------------------------------------
// Materials sets


MaterialsSet::MaterialsSet( vkhc::VulkanContext * p_context ) 
{
    assert( p_context != nullptr );
    assert( p_context->device != nullptr );
    assert( p_context->device->vk_device != VK_NULL_HANDLE );
    context = p_context ;
}

// ------------------------------------------------------------------------------
} // end namespace ilc
#pragma once 

#include <string>
#include <glm/glm.hpp>
#include <vulkan-context.h>

namespace ilc 
{

// ----------------------------------------------------------------------------------------------
// BRDF params struct

class BrdfParams
{
    public:
        float ka  = 0.1; // ambient coefficient
        float kd  = 0.8; // diffuse coefficient
        float ks  = 0.5; // specular coefficient
        float exp = 32.0; // specular exponent

        BrdfParams() = default ;
        BrdfParams( const float p_ka, const float p_kd, const float p_ks, const float p_exp ) ;
} ;

// ----------------------------------------------------------------------------------------------
// Materials

class Material
{

    private:
        glm::vec3   base_color;     // base color (RGB)
        bool        use_base_color; // true for using base color, false for not using it (uses either texture or vertex colors).
        std::string texture_path ;  // path to the texture file ("" for no texture)
        bool        use_texture ;   // only whn 'use_base_color = false' : use texture (true) or use vertex colors (false)
        BrdfParams  brdf_params ;   // coefficients of the Blinn-Phong model

        int color_base_index  = -1 ; // index of the base color in the UBO base color array (or -1 if no base color)
        int texture_index     = -1 ; // index of the texture in the shader (or -1 if no texture)
        int brdf_params_index = -1 ; // index of the BRDF parameters in the UBO BRDF parameters array (must be >=0 ??)

    public:
        // builds a material from the base color and the coefficients of the given BRDF params
        Material( const glm::vec3 & p_color_base, const BrdfParams & p_brdf_params ) ;

        // builds a material from a texture and the coefficients of the given BRDF params
        Material( const std::string & p_texture_path, const BrdfParams & p_brdf_params ) ;

        // builds a material with no base color (uses vertex colors) and the coefficients of the given BRDF params
        Material( const BrdfParams & p_brdf_params ) ;
} ;

// ----------------------------------------------------------------------------------------------
// A set of materials data, which is sent to the GPU.

class MaterialsSet
{
    private:
        vkhc::VulkanContext * context = nullptr ;
        std::vector<Material> materials ;

    public:
        MaterialsSet(  vkhc::VulkanContext * p_context )  ;
        uint32_t add( const Material & material ) ;
        const Material & get( uint32_t index ) const ;
} ;

// ----------------------------------------------------------------------------------------------

} // end namespace ilc
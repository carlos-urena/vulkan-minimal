#pragma once 

#include <string>

#include <glm/glm.hpp>

#include <vkhc/vulkan-context.h>
#include <vkhc/pipeline3D.h>

namespace ilc 
{

class MaterialsSet ;



// ----------------------------------------------------------------------------------------------
// Materials

class Material
{

    private:
        glm::vec3        base_color;     // base color (RGB)
        bool             use_base_color; // true for using base color, false for not using it (uses either texture or vertex colors).
        std::string      texture_path ;  // path to the texture file ("" for no texture)
        bool             use_texture ;   // only whn 'use_base_color = false' : use texture (true) or use vertex colors (false)
        vkhc::BrdfParams brdf_params ;   // coefficients of the Blinn-Phong model

        MaterialsSet * materials_set = nullptr ; // pointer to the materials set to which this material belongs (nullptr if still not added to any set)

        int color_base_index  = -1 ; // index of the base color in the UBO base color array (or -1 if no base color)
        int texture_index     = -1 ; // index of the texture in the shader (or -1 if no texture)
        int brdf_params_index = -1 ; // index of the BRDF parameters in the UBO BRDF parameters array (must be >=0 ??)

        friend class MaterialsSet ; // MaterialsSet is a friend class, so it can access the private members of Material

    public:
        // builds a material from the base color and the coefficients of the given BRDF params
        Material( const glm::vec3 & p_color_base, const vkhc::BrdfParams & p_brdf_params ) ;

        // builds a material from a texture and the coefficients of the given BRDF params
        Material( const std::string & p_texture_path, const vkhc::BrdfParams & p_brdf_params ) ;

        // builds a material with no base color (uses vertex colors) and the coefficients of the given BRDF params
        Material( const vkhc::BrdfParams & p_brdf_params ) ;

        //void activate( VkCommandBuffer & vk_cmd, vkhc::Pipeline3D & pipeline ) ; // activates this material in the given pipeline, by setting the push constants and UBOs in the shaders
        
} ;

// ----------------------------------------------------------------------------------------------
// A set of materials data, which is sent to the GPU.

class MaterialsSet
{
    private:

        vkhc::VulkanContext * context = nullptr ;
        std::vector<Material> materials ;
        vkhc::Pipeline3D *    pipeline3D = nullptr ; // pointer to the pipeline to which this materials set is bound (nullptr if not bound to any pipeline)

    public:

        vkhc::BaseColorsSet * base_colors_set = nullptr; // set of base colors, to be sent to the GPU via UBO
        vkhc::TexturesSet *   textures_set = nullptr ;    // set of textures, to be sent to the GPU via UBO
        vkhc::BrdfParamsSet * brdfs_params_set = nullptr ; // set of BRDF parameters, to be sent to the GPU via UBO

        MaterialsSet(  vkhc::VulkanContext * p_context )  ;
        uint32_t add( const Material & material ) ;
        
        // binds this material set to a pipeline, so the materials are sent to the GPU
        // done only once for a single pipeline, before first frame, but 
        // after all the materials have been added to the set.
        void bindTo( vkhc::Pipeline3D * p_pipeline3D ) ;

        // activates a material with the given materials index
        void activate( VkCommandBuffer & vk_cmd, 
                       vkhc::Pipeline3D & pipeline, 
                       const uint32_t material_index ); 
} ;

// ----------------------------------------------------------------------------------------------

} // end namespace ilc
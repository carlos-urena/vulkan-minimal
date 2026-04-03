// Declaration of the class 'BasicPipeline' 
//
// Encapsulates a graphics pipeline, including the Vulkan 
// pipeline object (VkPipeline), the pipeline layout, and the 
// descriptor set layout for UBOs.

#pragma once 

#include <common.h>


namespace vkhc
{

// ******************************************************************************
// A class which encapsulates pointer to shader sources,
// Pointers for optional shaders can be null 

class ShadersSources 
{
    public:

    const char * vertex_shader_src       = nullptr ;
    const char * tess_control_shader_src = nullptr ;
    const char * tess_eval_shader_src    = nullptr ;
    const char * geometry_shader_src     = nullptr ;
    const char * fragment_shader_src     = nullptr ;
} ;

// -------------------------------------------------------------------------------
// Graphics pipeline state.
// Note that this object depends on the number and format of the vertex attributes accepted 
// by the vertex shader, so to create this, one must know that in advance (but this is coded 
// in the shaders sources, so it makes sense).

class BasicPipeline
{
    public:

    Device * device = nullptr ;
    RenderPass * render_pass = nullptr ;

    VkPipeline                    vk_pipeline ;  // vulkan pipeline object 
    VkPipelineLayout              vk_pipeline_layout {  }; // layout in use 
    
    // UBO uniforms related variables (for the UBO bound at set=0, binding=0)
    VkDescriptorSetLayout         vk_ubo_set_layout { VK_NULL_HANDLE }; // set=0 layout for UBOs used by shaders
    VkDescriptorPool              vk_ubo_descriptor_pool { VK_NULL_HANDLE };
    VkDescriptorSet               vk_ubo_descriptor_set { VK_NULL_HANDLE };
    VkBuffer                      vk_view_ubo_buffer { VK_NULL_HANDLE };
    VkDeviceMemory                vk_view_ubo_memory { VK_NULL_HANDLE };
    
    // textures related variables (for the descriptor set bound at set=1, binding=0, with an array of texture samplers)
    VkDescriptorSetLayout         vk_textures_set_layout { VK_NULL_HANDLE }; // set=1 layout for texture samplers array used by fragment shader
    VkDescriptorPool              vk_textures_descriptor_pool { VK_NULL_HANDLE };
    VkDescriptorSet               vk_textures_descriptor_set { VK_NULL_HANDLE };

    VkShaderModule vk_vertex_shader_module       = VK_NULL_HANDLE ; 
    VkShaderModule vk_tess_control_shader_module = VK_NULL_HANDLE ;
    VkShaderModule vk_tess_eval_shader_module    = VK_NULL_HANDLE ;
    VkShaderModule vk_geometry_shader_module     = VK_NULL_HANDLE ;    
    VkShaderModule vk_fragment_shader_module     = VK_NULL_HANDLE ;

    bool has_tessellation_shaders = false ;
    
    // ----- 
    // pipeline configuration variables, should be set before calling 'initialize()' in 
    // derived classes constructors. 

    // (1) Push constans related variables (initialized in 'addPushConstant')

    static constexpr VkShaderStageFlags allStagesFlags = // every push constant will be used in every shader type
        VK_SHADER_STAGE_VERTEX_BIT |
        VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT |
        VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT |
        VK_SHADER_STAGE_GEOMETRY_BIT |
        VK_SHADER_STAGE_FRAGMENT_BIT ;

    static constexpr uint32_t        max_pc_total_size = 128 ; // maximum total size of push constant ranges, in bytes (Vulkan spec guarantees at least 128 bytes)
    std::vector<VkPushConstantRange> vk_pc_ranges{} ;  // vector of push constant ranges (see addPushConstant) 
    std::vector<std::string>         pc_names ;        // names of push constants, used for debugging 
    uint32_t                         pc_total_size = 0 ; // current total size of push constant ranges (see addPushConstant) 

    
    // (2) vectors with offset, sizes and names of uniform variables in the 
    // UBO bound at set=0, binding=0 (see addUBOUniform) )

    std::vector<std::string>  ubou_names ;
    std::vector<uint32_t>     ubou_offsets ;
    std::vector<uint32_t>     ubou_sizes ;
    uint32_t                  ubou_total_size = 0 ; // current total size of UBO variables (see addUBOuniform)
    static constexpr uint32_t ubou_max_total_size = 16*1024 ; // probably more (check at runtme)

    // (3) pointers to sources for each shader stage 
    ShadersSources shaders_sources ;

    // (4) vector with formats for the vertex attributes (determines the number of atributes)
    std::vector<VkFormat> attributes_formats ; 

    // --- end of configurable variables 


    // default type of primitives, can be changed dynamically in a command buffer with 
    // vkCmdSetPrimitiveTopology in Vulkan 1.3, or with 
    // vkCmdSetPrimitiveTopologyEXT if the extension VK_EXT_extended_dynamic_state3 is supported (and enabled)
    VkPrimitiveTopology default_primitive_topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST ; 
    uint32_t default_vertexes_per_patch = 3 ; // only used if tessellation shaders are present, ignored otherwise (Vulkan spec guarantees at least 3 vertexes per patch)

    // shader modules and stages
    std::vector<VkPipelineShaderStageCreateInfo> vk_shader_stages ;

    // true if the 'initialize()' method has been completed once, false otherwise 
    // (used to avoid calling it twice, or to use the pipeline without proper initialization)
    bool initialized = false ;
    
    // ------------------------------------------------------------------------------
    // Methods.
    
    // Create a shader module from SPIR-V code (vector of uint32_t)
    VkShaderModule createModule( std::vector<uint32_t>& spirv_code ) ;
    
    // return a string with the description of a shader kind (vertex, fragment, etc.) for debugging purposes
    const std::string shaderKindDescription( shaderc_shader_kind kind );
    
    // Compiles a shader from it GLSL source and kind. return bytes array with SPIR-V code.
    // if compilation fails, prints an error message and aborts the program.
    std::vector<uint32_t> compileGLSL( const char* src, shaderc_shader_kind kind ) ;
    
    // Get the size in bytes for data corresponding to a given vulkan format for 
    // a vertex attrribute (only 2 and 3 floats allowed, otherwise aborts)
    uint32_t getAttributeFormatSize( const VkFormat format ) ;
    
    // adds a push constant range. The calls order must match the push constant block in shaders 
    // the offser of each push constant is updated . 
    void addPushConstant( const std::string & name, uint32_t p_size ) ;

    // Adds a push constant command to a command buffer, using the push constant name to find the corresponding range
    void setPushConstant( VkCommandBuffer & vk_cmd_buffer, const std::string & name, const void * data_ptr ) ;
   
    // Adds a uniform variable to the UBO bound at set=0, binding=0
    // the calls order must match the order of variables in the UBO in shaders
    void addUBOUniform( const std::string & name, uint32_t size ) ;
    
    // Gives a value to a an UBO uniform (pre-rendering).
    void setUBOUniform( const std::string & name, const void * data_ptr ) ;
    
    // Initializes this pipeline object, starting from an undefined state
    // (should be called from derived classes constructors, after setting the configuration variables)
    void initialize(); 

    // adds the UBO buffer and descriptor set for the UBO bound at set=0, binding=0, and 
    // updates the descriptor set to point to the buffer
    void initializeUBODescriptor() ;

    // adds the descriptor set for the texture samplers array bound at set=1, binding=0, 
    // and updates the descriptor set to point to the texture array
    void initializeTextureSamplersDescriptor();

    // creates the pipeline layout (after  UBO descriptor and samplers descriptors)
    void createPipelineLayout();

    // initializes the shader stages (vk_shader_stages vector)
    void initializeShaderStages() ;

    // creates the vulkan graphics pipeline object (vk_pipeline) with the current configuration and the shader stages
    void createGraphicsPipeline() ;

    // Constructor 
    BasicPipeline( VulkanContext & vulkan_context ) ; 
    
    // Add this pipeline bind command to a command buffer 
    void bind( VkCommandBuffer & vk_cmd_buffer ) ;

    // Destructor
    ~BasicPipeline() ;
    
} ; // end class 'BasicPipeline' 

// ------------------------------------------------------------------------------

} // end namespace 'vkhc' 


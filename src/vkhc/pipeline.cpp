// Implementation of the class 'BasicPipeline' 
//
// Encapsulates a graphics pipeline, including the Vulkan 
// pipeline object (VkPipeline), the pipeline layout, and the 
// descriptor set layout for UBOs.


#include <sstream>
#include <pipeline.h>
#include <device.h>
#include <render-pass.h>
#include <vulkan-context.h>
#include <textures.h>

// ***********************************************************************************
// Basic shader sources (GLSL)



namespace vkhc
{

// --------------------------------------------------------------------------------
// Replaces a line starting with //#keyword with substituion text, returns new text 

std::string insert_source( const std::string & src, const std::string & keyword, const std::string & substitution ) 
{
    using namespace std ;

    istringstream iss(src);
    ostringstream oss;
    string line;

    const string search_str = "//#" + keyword;

    while ( getline( iss, line) ) {
        if ( line.find( search_str ) == 0 ) { // line starts with the //#keyword
            oss << substitution << endl;
        } else {
            oss << line << endl;
        }
    }
    return oss.str();
}

// ------------------------------------------------------------------------------
// create a shader module from SPIR-V code (vector of uint32_t)

VkShaderModule BasicPipeline::createModule( std::vector<uint32_t>& spirv_code ) 
{
    VkShaderModuleCreateInfo smci{ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
    VkShaderModule           vk_module;

    smci.codeSize = spirv_code.size() * sizeof(uint32_t);
    smci.pCode = spirv_code.data();
    
    const VkResult res = vkCreateShaderModule( device->vk_device, &smci, nullptr, &vk_module);
    if ( res != VK_SUCCESS )
        ErrorExit("vkCreateShaderModule failed");
    return vk_module;
};

// ------------------------------------------------------------------------------

const std::string BasicPipeline::shaderKindDescription( shaderc_shader_kind kind )
{
    switch (kind) {
        case shaderc_vertex_shader:          return "vertex shader"; 
        case shaderc_tess_control_shader:    return "tessellation control shader"; 
        case shaderc_tess_evaluation_shader: return "tessellation evaluation shader"; 
        case shaderc_geometry_shader:        return "geometry shader";
        case shaderc_fragment_shader:        return "fragment shader"; 
        default:                             return "unknown shader kind"; 
    }
}

// ------------------------------------------------------------------------------

std::vector<uint32_t> BasicPipeline::compileGLSL( const std::string & src, shaderc_shader_kind kind ) 
{
    shaderc::Compiler       compiler ;
    shaderc::CompileOptions options ;

    // using namespace std ;
    // string src_str { src };
    // string full_src = insert_source( src_str, "common_inputs_declarations", common_decls );
    
    auto result = compiler.CompileGlslToSpv( src, kind, "shader.glsl", options );
    if ( result.GetCompilationStatus() != shaderc_compilation_status_success )
    {
        using namespace std ; 
        cout << "Error compiling " << shaderKindDescription(kind) << ": " << endl ;
        cout << result.GetErrorMessage() ;
        exit( EXIT_FAILURE);
    }
    return {result.cbegin(), result.cend()};
}
// ------------------------------------------------------------------------------
// get the size in bytes for data corresponding to a given vulkan format for 
// a vertex attrribute (only 2 and 3 floats allowed, otherwise aborts)

uint32_t BasicPipeline::getAttributeFormatSize( const VkFormat format ) 
{
    switch( format ) 
    {
        case VK_FORMAT_R32G32_SFLOAT    : return 2 * sizeof(float); // vec2
        case VK_FORMAT_R32G32B32_SFLOAT : return 3 * sizeof(float); // vec3
        default: ErrorExit("Unsupported attribute format (creating pipeline)");
    }
    return 0 ;
}
// -----------------------------------------------------------------------------
// adds a push constant range. The calls order must match the push constant block in shaders 
// the offser of each push constant is updated . 

void BasicPipeline::addPushConstant( const std::string & name, uint32_t p_size )
{
    assert( ! initialized ); 
    assert( p_size > 0 );
    assert( p_size % 4 == 0 );
    assert( pc_total_size + p_size <= max_pc_total_size );

    VkPushConstantRange vk_push_constant_range{};
    
    vk_push_constant_range.offset = pc_total_size ;
    vk_push_constant_range.size = p_size ;
    vk_push_constant_range.stageFlags = allStagesFlags ; 

    vk_pc_ranges.push_back( vk_push_constant_range );
    pc_names.push_back( name );

    std::cout << "Added push constant '" << name << "' with size " << p_size << " bytes, offset " << pc_total_size << std::endl ;

    pc_total_size += p_size ;
}



// -----------------------------------------------------------------------------
// adds a push constant command to a command buffer, using the push constant name to find the corresponding range

void BasicPipeline::setPushConstant( VkCommandBuffer & vk_cmd_buffer, const std::string & name, const void * data_ptr ) 
{
    assert( initialized ); 

    auto it = std::find( pc_names.begin(), pc_names.end(), name );
    if ( it == pc_names.end() )
    {
        std::cerr << "Error: push constant with name '" << name << "' not found in the pipeline" << std::endl ;
        exit(1);
    }

    const uint32_t index = std::distance( pc_names.begin(), it );
    const VkPushConstantRange & range = vk_pc_ranges[ index ];

    vkCmdPushConstants( vk_cmd_buffer, vk_pipeline_layout, range.stageFlags, range.offset, range.size, data_ptr );
}

// -----------------------------------------------------------------------------
// adds a uniform variable to the UBO bound at set=0, binding=0
// the calls order must match the order of variables in the UBO in shaders


void BasicPipeline::addUBOUniform( const std::string & name, uint32_t size ) 
{
    assert( ! initialized ); 
    assert( size > 0 );
    //assert( size % 4 == 0 );
    assert( ubou_total_size + size <= ubou_max_total_size );

    ubou_names.push_back( name );
    ubou_offsets.push_back( ubou_total_size );
    ubou_sizes.push_back( size );

    
    std::cout << "Added UBO uniform '" << name << "' with size " << size << " bytes, offset " << ubou_total_size << std::endl ;

    ubou_total_size += size ;
}

// -----------------------------------------------------------------------------
// gives a value to a an UBO uniform (pre-rendering).

void BasicPipeline::setUBOUniform( const std::string & name, const void * data_ptr ) 
{
    assert( initialized ); 

    auto it = std::find( ubou_names.begin(), ubou_names.end(), name );
    if ( it == ubou_names.end() )
    {
        std::cerr << "Error: UBO uniform with name '" << name << "' not found in the pipeline" << std::endl ;
        exit(1);
    }

    const uint32_t index = std::distance( ubou_names.begin(), it );
    const uint32_t offset = ubou_offsets[ index ];
    const uint32_t size = ubou_sizes[ index ];

    assert( vk_view_ubo_memory != VK_NULL_HANDLE );

    void * mapped = nullptr;
    vkMapMemory( device->vk_device, vk_view_ubo_memory, offset, size, 0, &mapped );
    memcpy( mapped, data_ptr, size );
    vkUnmapMemory( device->vk_device, vk_view_ubo_memory );
}
// -----------------------------------------------------------------------------

void BasicPipeline::initializeUBODescriptor() 
{
    assert( ! initialized ); 
    assert( device != nullptr );
    assert( device->vk_device != VK_NULL_HANDLE );
    assert( ubou_total_size > 0 ); 

    // Create UBO buffer that stores the UBO uniforms
    VkBufferCreateInfo ubo_bci{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    ubo_bci.size = ubou_total_size ; 
    ubo_bci.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    ubo_bci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    if ( vkCreateBuffer( device->vk_device, &ubo_bci, nullptr, &vk_view_ubo_buffer ) != VK_SUCCESS )
        ErrorExit("Failed to create UBO buffer");

    VkMemoryRequirements ubo_mem_req{};
    vkGetBufferMemoryRequirements( device->vk_device, vk_view_ubo_buffer, &ubo_mem_req );

    VkMemoryAllocateInfo ubo_mai{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
    ubo_mai.allocationSize = ubo_mem_req.size;
    ubo_mai.memoryTypeIndex = UINT32_MAX;

    ubo_mai.memoryTypeIndex = device->findMemoryTypeIndex( ubo_mem_req, 
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT );

    if ( vkAllocateMemory( device->vk_device, &ubo_mai, nullptr, &vk_view_ubo_memory ) != VK_SUCCESS )
        ErrorExit("Failed to allocate UBO memory");
    if ( vkBindBufferMemory( device->vk_device, vk_view_ubo_buffer, vk_view_ubo_memory, 0 ) != VK_SUCCESS )
        ErrorExit("Failed to bind UBO memory");
    //setViewMatrix( glm::mat4(1.0f) );

    VkDescriptorSetLayoutBinding ubo_binding{};
    ubo_binding.binding = 0;
    ubo_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ubo_binding.descriptorCount = 1;
    ubo_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT |
                             VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT |
                             VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT |
                             VK_SHADER_STAGE_GEOMETRY_BIT |
                             VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo ubo_set_layout_ci{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
    ubo_set_layout_ci.bindingCount = 1;
    ubo_set_layout_ci.pBindings = &ubo_binding;

    if ( vkCreateDescriptorSetLayout( device->vk_device, &ubo_set_layout_ci, nullptr, &vk_ubo_set_layout ) != VK_SUCCESS )
        ErrorExit("Failed to create UBO descriptor set layout");

    VkDescriptorPoolSize ubo_pool_size{};
    ubo_pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ubo_pool_size.descriptorCount = 1;

    VkDescriptorPoolCreateInfo ubo_dpci{ VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
    ubo_dpci.maxSets = 1;
    ubo_dpci.poolSizeCount = 1;
    ubo_dpci.pPoolSizes = &ubo_pool_size;
    if ( vkCreateDescriptorPool( device->vk_device, &ubo_dpci, nullptr, &vk_ubo_descriptor_pool ) != VK_SUCCESS )
        ErrorExit("Failed to create UBO descriptor pool");

    VkDescriptorSetAllocateInfo ubo_dsai{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
    ubo_dsai.descriptorPool = vk_ubo_descriptor_pool;
    ubo_dsai.descriptorSetCount = 1;
    ubo_dsai.pSetLayouts = &vk_ubo_set_layout;
    if ( vkAllocateDescriptorSets( device->vk_device, &ubo_dsai, &vk_ubo_descriptor_set ) != VK_SUCCESS )
        ErrorExit("Failed to allocate UBO descriptor set");

    VkDescriptorBufferInfo ubo_buffer_info{};
    ubo_buffer_info.buffer = vk_view_ubo_buffer;
    ubo_buffer_info.offset = 0;
    ubo_buffer_info.range = ubou_total_size ;

    VkWriteDescriptorSet ubo_write{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
    ubo_write.dstSet = vk_ubo_descriptor_set;
    ubo_write.dstBinding = 0;
    ubo_write.descriptorCount = 1;
    ubo_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ubo_write.pBufferInfo = &ubo_buffer_info;
    vkUpdateDescriptorSets( device->vk_device, 1, &ubo_write, 0, nullptr );
}
// -----------------------------------------------------------------------------

void BasicPipeline::initializeTextureSamplersDescriptor() 
{
    assert( ! initialized ); 
    constexpr uint32_t max_texture_descriptors = TexturesSet::max_textures ; 

    VkDescriptorSetLayoutBinding textures_binding{};
    textures_binding.binding = 0;
    textures_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    textures_binding.descriptorCount = max_texture_descriptors;
    textures_binding.stageFlags = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT |
                                  VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT |
                                  VK_SHADER_STAGE_GEOMETRY_BIT |
                                  VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo textures_set_layout_ci{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
    textures_set_layout_ci.bindingCount = 1;
    textures_set_layout_ci.pBindings = &textures_binding;
    if ( vkCreateDescriptorSetLayout( device->vk_device, &textures_set_layout_ci, nullptr, &vk_textures_set_layout ) != VK_SUCCESS )
        ErrorExit("Failed to create textures descriptor set layout");

    VkDescriptorPoolSize textures_pool_size{};
    textures_pool_size.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    textures_pool_size.descriptorCount = max_texture_descriptors;

    VkDescriptorPoolCreateInfo textures_dpci{ VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
    textures_dpci.maxSets = 1;
    textures_dpci.poolSizeCount = 1;
    textures_dpci.pPoolSizes = &textures_pool_size;
    if ( vkCreateDescriptorPool( device->vk_device, &textures_dpci, nullptr, &vk_textures_descriptor_pool ) != VK_SUCCESS )
        ErrorExit("Failed to create textures descriptor pool");

    VkDescriptorSetAllocateInfo textures_dsai{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
    textures_dsai.descriptorPool = vk_textures_descriptor_pool;
    textures_dsai.descriptorSetCount = 1;
    textures_dsai.pSetLayouts = &vk_textures_set_layout;
    if ( vkAllocateDescriptorSets( device->vk_device, &textures_dsai, &vk_textures_descriptor_set ) != VK_SUCCESS )
        ErrorExit("Failed to allocate textures descriptor set");
}
// -----------------------------------------------------------------------------

void BasicPipeline::createPipelineLayout() 
{
    assert( ! initialized ); 
    assert( vk_ubo_set_layout != VK_NULL_HANDLE );
    assert( vk_textures_set_layout != VK_NULL_HANDLE );

    VkDescriptorSetLayout set_layouts[] = { vk_ubo_set_layout, vk_textures_set_layout };

    // Create the pipeline layout 
    VkPipelineLayoutCreateInfo    plci{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };

    plci.setLayoutCount = 2;
    plci.pSetLayouts = set_layouts;
    plci.pushConstantRangeCount = 0;

    if ( !vk_pc_ranges.empty() )
    { 
        // Use one contiguous range to avoid overlapping stageFlags across multiple ranges.
        static VkPushConstantRange merged_pc_range{};
        merged_pc_range.offset = 0;
        merged_pc_range.size = pc_total_size;
        merged_pc_range.stageFlags = allStagesFlags;

        plci.pushConstantRangeCount = 1;
        plci.pPushConstantRanges = &merged_pc_range;

        std::cout << "Push constants configuration:" << std::endl ;
        for( uint32_t i = 0 ; i < vk_pc_ranges.size() ; i++ ) 
        {
            auto & vk_pcr = vk_pc_ranges[i] ;
            std::cout << "Push constant '" << pc_names[i] << "': offset = " << vk_pcr.offset << ", size = " << vk_pcr.size << std::endl ;
        }
    }
    else 
        std::cout << "No push constants used in this pipeline." << std::endl ;
    
    // create the pipeline layout 
    if ( vkCreatePipelineLayout( device->vk_device, &plci, nullptr, &vk_pipeline_layout ) != VK_SUCCESS )
        ErrorExit("Failed to create pipeline layout");
}
// ------------------------------------------------------------------------------

void BasicPipeline::initializeShaderStages() 
{
    assert( ! initialized ); 
    // Initialize shader modules (compile, link, etc..)

    const std::string * vs_src = shaders_sources.vertex_shader_src ;      assert( vs_src != nullptr );
    const std::string * fs_src = shaders_sources.fragment_shader_src ;    assert( fs_src != nullptr );

    auto vertSPV = compileGLSL( *vs_src, shaderc_vertex_shader);

    //auto tescSPV = compileGLSL(tescShaderSrc, shaderc_tess_control_shader);
    //auto teseSPV = compileGLSL(teseShaderSrc, shaderc_tess_evaluation_shader);
    auto fragSPV = compileGLSL( *fs_src, shaderc_fragment_shader);

    vk_vertex_shader_module       = createModule( vertSPV ) ; 
    vk_fragment_shader_module     = createModule( fragSPV ) ;
    vk_tess_control_shader_module = VK_NULL_HANDLE ;
    vk_tess_eval_shader_module    = VK_NULL_HANDLE ;

    
    //tescMod = createModule(tescSPV);
    //VkShaderModule teseMod = createModule(teseSPV);

    // todo: use the shaders sources to guess which shaders the user wants to use.
    
    vk_shader_stages.push_back({ 
        .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage  = VK_SHADER_STAGE_VERTEX_BIT, 
        .module = vk_vertex_shader_module,
        .pName  = "main"
    });

    if ( shaders_sources.tess_control_shader_src != nullptr && 
         shaders_sources.tess_eval_shader_src != nullptr ) 
    {
        assert( device->likelyHasTessellation );
        has_tessellation_shaders = true ;

        const std::string * tesc_src = shaders_sources.tess_control_shader_src ;    assert( tesc_src != nullptr );
        const std::string * tese_src = shaders_sources.tess_eval_shader_src ;     assert( tese_src != nullptr );
        
        auto tescSPV = compileGLSL( *tesc_src, shaderc_tess_control_shader);
        auto teseSPV = compileGLSL( *tese_src, shaderc_tess_evaluation_shader);

        vk_tess_control_shader_module = createModule( tescSPV ) ;
        vk_tess_eval_shader_module = createModule( teseSPV ) ;

        vk_shader_stages.push_back({ 
            .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, 
            .stage  = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, 
            .module = vk_tess_control_shader_module, 
            .pName  = "main"
        });

        vk_shader_stages.push_back({ 
            .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, 
            .stage  = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, 
            .module = vk_tess_eval_shader_module, 
            .pName  = "main"
        });
    }

    if ( shaders_sources.geometry_shader_src != nullptr ) 
    {
        assert( device->likelyHasGeometryShader );
        const std::string * geom_src = shaders_sources.geometry_shader_src ;    assert( geom_src != nullptr );
        auto geomSPV = compileGLSL( *geom_src, shaderc_geometry_shader);
        vk_geometry_shader_module = createModule( geomSPV ) ;
        vk_shader_stages.push_back({ 
            .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, 
            .stage  = VK_SHADER_STAGE_GEOMETRY_BIT, 
            .module = vk_geometry_shader_module, 
            .pName  = "main"
        });
    }

    vk_shader_stages.push_back({ 
        .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, 
        .stage  = VK_SHADER_STAGE_FRAGMENT_BIT, 
        .module = vk_fragment_shader_module, 
        .pName  = "main"
    });
    
    if ( has_tessellation_shaders ) 
    {
        default_primitive_topology = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST ;
        std::cout << "Tessellation shaders detected, using VK_PRIMITIVE_TOPOLOGY_PATCH_LIST as default topology." << std::endl ;
    }
}
// ------------------------------------------------------------------------------

void BasicPipeline::createGraphicsPipeline() 
{
    assert( ! initialized ); 
    // Input state (vertex attributes)

    assert( attributes_formats.size() > 0 ); // at least one attribute format should be specified

    std::vector<VkVertexInputBindingDescription>   bindingDescs {};
    std::vector<VkVertexInputAttributeDescription> attrDescs {};

    for( uint32_t i = 0 ; i <  attributes_formats.size(); i++  ) 
    {   
        bindingDescs.push_back({ .binding = i, .stride = getAttributeFormatSize( attributes_formats[i] ), 
                                 .inputRate = VK_VERTEX_INPUT_RATE_VERTEX });
        attrDescs.push_back({ .location = i, .binding = i, .format = attributes_formats[i], .offset = 0});
    }

    VkPipelineVertexInputStateCreateInfo vi{ VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
    vi.vertexBindingDescriptionCount   = static_cast<uint32_t>(bindingDescs.size());
    vi.pVertexBindingDescriptions      = bindingDescs.data();
    vi.vertexAttributeDescriptionCount = static_cast<uint32_t>(attrDescs.size());
    vi.pVertexAttributeDescriptions    = attrDescs.data();

    // other structures used for pipeline creation

    VkPipelineInputAssemblyStateCreateInfo ia{ VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
    ia.topology = default_primitive_topology ;

    VkPipelineMultisampleStateCreateInfo ms{ VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
    ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineRasterizationStateCreateInfo rs{ VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
    //rs.polygonMode = VK_POLYGON_MODE_LINE;  
    rs.polygonMode  = VK_POLYGON_MODE_FILL;
    rs.cullMode     = VK_CULL_MODE_NONE;
    rs.lineWidth    = 1.0f;

    VkPipelineColorBlendAttachmentState blend{};
    blend.colorWriteMask = 0xF;

    VkPipelineColorBlendStateCreateInfo cb{ VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
    cb.attachmentCount = 1;
    cb.pAttachments = &blend;

    VkPipelineViewportStateCreateInfo vp{ VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
    vp.viewportCount = 1;
    vp.scissorCount = 1;

    std::array<VkDynamicState, 3> dynStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
        VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY
    };
    VkPipelineDynamicStateCreateInfo dyn{};

    dyn.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dyn.dynamicStateCount = device->hasDynamicPrimitiveTopology ? 3u : 2u ;
    dyn.pDynamicStates = dynStates.data();

    // make 'vk_tess_info_ptr' point to 'vk_tess_info' if tessellation shaders are used, or leave it as nullptr otherwise (it will be ignored in this case, since the topology is not patch list)

    VkPipelineTessellationStateCreateInfo * vk_tess_info_ptr = nullptr ;
    VkPipelineTessellationStateCreateInfo vk_tess_info{ VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO };

    if ( has_tessellation_shaders ) 
    {   vk_tess_info.patchControlPoints = default_vertexes_per_patch; // it is 3.
        std::cout << "Tessellation shaders detected, using " << default_vertexes_per_patch << " vertexes per patch." << std::endl ;
        vk_tess_info_ptr = &vk_tess_info ;
    }

    VkGraphicsPipelineCreateInfo  gpci{ VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };

    gpci.stageCount          = vk_shader_stages.size() ; 
    gpci.pStages             = vk_shader_stages.data();
    gpci.pVertexInputState   = &vi;
    gpci.pInputAssemblyState = &ia;
    gpci.pTessellationState  = vk_tess_info_ptr;
    gpci.pMultisampleState   = &ms;
    gpci.pRasterizationState = &rs;
    gpci.pColorBlendState    = &cb;
    gpci.pViewportState      = &vp;
    gpci.pDynamicState       = &dyn;
    gpci.layout              = vk_pipeline_layout ;
    gpci.renderPass          = render_pass->vk_render_pass ; 

    // Validation lambda to catch common mistakes in pipeline creation. This is not exhaustive, just a sanity check.
    auto validateGpci = [](const VkGraphicsPipelineCreateInfo& info) {
        if (info.sType != VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO)
            ErrorExit("gpci.sType is invalid");
        if (info.stageCount == 0 || info.pStages == nullptr)
            ErrorExit("gpci has no shader stages");
        if (info.pInputAssemblyState == nullptr)
            ErrorExit("gpci.pInputAssemblyState is null");
        if (info.pRasterizationState == nullptr)
            ErrorExit("gpci.pRasterizationState is null");
        if (info.pViewportState == nullptr)
            ErrorExit("gpci.pViewportState is null");
        if (info.pColorBlendState == nullptr)
            ErrorExit("gpci.pColorBlendState is null");
        if (info.layout == VK_NULL_HANDLE)
            ErrorExit("gpci.layout is null");
        if (info.renderPass == VK_NULL_HANDLE)
            ErrorExit("gpci.renderPass is null");

        for (uint32_t i = 0; i < info.stageCount; ++i) {
            if (info.pStages[i].module == VK_NULL_HANDLE)
                ErrorExit("gpci contains a shader stage with null module");
            if (info.pStages[i].pName == nullptr)
                ErrorExit("gpci contains a shader stage with null entry point");
        }

        if (info.pInputAssemblyState->topology == VK_PRIMITIVE_TOPOLOGY_PATCH_LIST &&
            info.pTessellationState == nullptr)
            ErrorExit("gpci.patch topology requires tessellation state");
    };

    validateGpci(gpci); 
    if ( vkCreateGraphicsPipelines( device->vk_device, VK_NULL_HANDLE, 1, &gpci, nullptr, &vk_pipeline ) != VK_SUCCESS )
        ErrorExit("Failed to create graphics pipeline");

}
// ------------------------------------------------------------------------------

void BasicPipeline::initialize(  ) // Device * p_device, RenderPass * p_render_pass )
{
    if ( initialized )
    {   std::cerr << "Error: pipeline already initialized" << std::endl ;
        exit(1);
    }
        
    initializeUBODescriptor(); // creates 'vk_ubo_set_layout' 
    initializeTextureSamplersDescriptor(); // creates 'vk_textures_set_layout' 
    createPipelineLayout(); // creates 'vk_pipeline_layout' (uses 'vk_ubo_set_layout' and 'vk_textures_set_layout')
    initializeShaderStages();  // creates vector 'vk_shader_stages'
    createGraphicsPipeline() ; // creates 'vk_pipeline' 

    initialized = true ;

    //std::cout << "Graphics pipeline created." << std::endl ;
}
// ------------------------------------------------------------------------------
// constructor, does nothing, 'initialize' must be called after this. 

BasicPipeline::BasicPipeline( VulkanContext & p_vulkan_context ) 
{
    device = p_vulkan_context.device;
    render_pass = p_vulkan_context.render_pass;

    assert( device != nullptr );
    assert( render_pass != nullptr );
}
// -------------------------------------------------------------------------------------------------
// add this pipeline bind command to a command buffer 

void BasicPipeline::bind( VkCommandBuffer & vk_cmd_buffer ) 
{
    assert( initialized ); 
    
    vkCmdBindPipeline( vk_cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_pipeline );
    VkDescriptorSet sets[] = { vk_ubo_descriptor_set, vk_textures_descriptor_set };
    vkCmdBindDescriptorSets( vk_cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_pipeline_layout,
                                0, 2, sets, 0, nullptr );
}

// ---------------------------------------------------------------------------------
// destroys the pipeline

BasicPipeline::~BasicPipeline()
{
    //assert( initialized ); // make sure the pipeline was initialized before destroying it

    if ( ubou_names.size() > 0 ) ubou_names.clear();
    if ( ubou_offsets.size() > 0 ) ubou_offsets.clear();
    if ( ubou_sizes.size() > 0 ) ubou_sizes.clear();
    if ( pc_names.size() > 0 ) pc_names.clear();
    

    if ( vk_pipeline != VK_NULL_HANDLE )
        vkDestroyPipeline( device->vk_device, vk_pipeline, nullptr );
    if ( vk_pipeline_layout != VK_NULL_HANDLE )
        vkDestroyPipelineLayout( device->vk_device, vk_pipeline_layout, nullptr );
    if ( vk_ubo_set_layout != VK_NULL_HANDLE )
        vkDestroyDescriptorSetLayout( device->vk_device, vk_ubo_set_layout, nullptr );
    if ( vk_textures_set_layout != VK_NULL_HANDLE )
        vkDestroyDescriptorSetLayout( device->vk_device, vk_textures_set_layout, nullptr );
    if ( vk_ubo_descriptor_pool != VK_NULL_HANDLE )
        vkDestroyDescriptorPool( device->vk_device, vk_ubo_descriptor_pool, nullptr );
    if ( vk_textures_descriptor_pool != VK_NULL_HANDLE )
        vkDestroyDescriptorPool( device->vk_device, vk_textures_descriptor_pool, nullptr );
    if ( vk_view_ubo_buffer != VK_NULL_HANDLE )
        vkDestroyBuffer( device->vk_device, vk_view_ubo_buffer, nullptr );
    if ( vk_view_ubo_memory != VK_NULL_HANDLE )
        vkFreeMemory( device->vk_device, vk_view_ubo_memory, nullptr );

    std::cout << "Deleted pipeline" << std::endl ;
}
// ------------------------------------------------------------------------------

} // end namespace 'vkhc' 


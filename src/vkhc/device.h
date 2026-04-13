#pragma once 

#include <common.h>
#include <sync-objects.h>


namespace vkhc
{


class Device  // includes physical device, device and queue creation
{
    public:

    Instance * instance = nullptr ;

    std::vector<VkPhysicalDevice>           vk_ava_gpus ;     // available GPUs
    std::vector<VkPhysicalDeviceProperties> vk_ava_gpus_props; // available gpu props

    uint32_t ava_gpus_count = 0 ;  // total number of available GPUs in the system
    uint32_t gpu_index      = 0 ;  // index of selected GPU
    
    VkDevice                      vk_device ;           // logical device created from the physical device
    VkPhysicalDevice              vk_gpu;               // selected GPU (physical device)
    VkPhysicalDeviceProperties    vk_gpu_props{};       // properties for the selected gpu
    
    VkQueue                       vk_queue ;            // queue used to submit command buffers and present to the swap chain
    //VkSampler                     vk_sampler { VK_NULL_HANDLE }; // shared sampler used by texture descriptors
    
    //VkPhysicalDeviceProperties    vk_gpu_props{};       // properties for the selected gpu
    VkPhysicalDeviceFeatures      vk_desired_feat{};    // requested features for physical device creation 
    VkPhysicalDeviceFeatures      vk_available_feat{};  // available features found after physical device creation
    std::vector<VkExtensionProperties> supp_extensions;      // vector with extensions supported by the physical device
    VkPhysicalDeviceMemoryProperties   vk_mem_props{};       // memory properties, used later when a vertex buffer is created..

    
    
    
    uint32_t supp_extensions_count = 0; // total number of extension supported by the physical device
    
    bool likelyHasTessellation = false ;
    bool likelyHasGeometryShader = false ;
    bool likelyHasWireframeRender = false ;
    bool hasDynamicPrimitiveTopology = false ;

    float queuePriority = 1.0f;

    // list of required extensions 
    static constexpr uint32_t    required_extensions_count = 1 ; 
    static constexpr const char* required_extensions_names[ required_extensions_count ] = { 
        VK_KHR_SWAPCHAIN_EXTENSION_NAME, 
        //"VK_EXT_extended_dynamic_state3" // no está soportada en macOs ?? (en Linux sí).
    };

    // ---------------------------------------------------------------------------------------------------

    // enumerate available GPUs and selects one of them .
    // updates 'vk_ava_gpus', 'vk_ava_gpus_props', ava_gpus_count
    void enumerateAvailableGPUs( ) ;

    // select a GPU from available GPUs (after enumeration)
    // updates 'vk_gpu', 'vk_gpu_props', 'vk_mem_props'
    void selectGPU();

    // Get memory properties  and the physical device features 
    // of the selected GPU, and print some of them.
    // Initializes: 'vk_mem_props', 'vk_available_feat'.
    void initializeGPUProperties() ;

    // Get the list of available extensions and check the required extensiones (if any).
    // Uses 'required_extensions_names' and updates 'supp_extensions' and 'supp_extensions_count'.
    void checkAvailableExtensions() ;

    // create the vk_device and vk_queue 
    void createLogicalDeviceAndGetQueue() ;

    // check if an extension is in the list of supported extensions
    bool extensionIsSupported( const char * req_ext_name ) ;
    
    // Constructor
    Device( Instance * instance ) ;
    
    // Submits a command buffer to the device queue 
    void submitCommandBuffer( VkCommandBuffer & vk_cmd_buffer, SyncObjects * sync_objects );
    

    ~Device();

    // Find memory type index with the desired properties from a memory requeriment
    // usually called with desired properties: 
    // VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    uint32_t findMemoryTypeIndex( VkMemoryRequirements & mem_req, VkMemoryPropertyFlags desired_props );

    // Create and VkBuffer and corresponding VkDeviceMemory, and copy data to the GPU 
    void createBufferAndCopyData
    ( 
        VkDeviceSize       vk_total_size_bytes, 
        const void*        src_data, 
        VkBufferUsageFlags usage_flags,
        VkBuffer &         vk_buffer,
        VkDeviceMemory &   vk_memory 
    );

} ; // end of Device class 

} // vkhc namespace end 


// Implementation of Device class 

#include <device.h>
#include <instance.h>
#include <vulkan-context.h>
#include <command-pool-buffers.h>

namespace vkhc
{

// ---------------------------------------------------------------------------------------------------
// Check if an extension is in the list of supported extensions

bool Device::extensionIsSupported( const char * req_ext_name )
{
    bool found = false ;
    for ( const auto &supp_ext : supp_extensions )
    {   found = std::strcmp( req_ext_name, supp_ext.extensionName ) == 0 ;
        if (found) break ;
    }
    std::cout << "Required extension '" << req_ext_name << "': "
                    << (found ? "supported" : "NOT supported") << std::endl;
    return found ;
}

// aux func 

std::string getGPUTypeDescription( VkPhysicalDeviceType type )
{
    switch (type) 
    {
        case VK_PHYSICAL_DEVICE_TYPE_OTHER: return "Other";
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: return "Integrated GPU";
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU: return "Discrete GPU";
        case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU: return "Virtual GPU";
        case VK_PHYSICAL_DEVICE_TYPE_CPU: return "CPU";
        default: return "Unknown type";
    }
}

// ---------------------------------------------------------------------------------------------------
// enumerate available GPUs and selects one of them .
// initialize 'vk_ava_gpus', 'vk_ava_gpus_props', 'ava_gpus_count'

void Device::enumerateAvailableGPUs( ) 
{
    assert( instance != nullptr );
    assert( ava_gpus_count == 0 );
    assert( vk_ava_gpus.empty() ); // check that the constructor is not called twice (this would cause a memory leak, because the destructor is not called for the previous device)
    //VkInstance & vk_instance = instance->vk_instance ; 

    vkEnumeratePhysicalDevices( instance->vk_instance, &ava_gpus_count, nullptr);
    std::cout << "Found " << ava_gpus_count << " Vulkan-capable device(s)" << std::endl ;
    vk_ava_gpus.resize( ava_gpus_count );
    vkEnumeratePhysicalDevices( instance->vk_instance, &ava_gpus_count, vk_ava_gpus.data());

    using namespace std ;
    cout << "Available GPUs:" << endl ;

    // gather physical device properties for all available GPUs, 
    vk_ava_gpus_props.resize( ava_gpus_count );
    for ( uint32_t i = 0; i < ava_gpus_count; ++i ) 
        vkGetPhysicalDeviceProperties( vk_ava_gpus[i], &vk_ava_gpus_props[i] );

    // print available GPUs info
    for( uint32_t i = 0; i < ava_gpus_count; ++i )
    {
        std::string vendor_name = getVendorName( vk_ava_gpus_props[i].vendorID ) ;
        std::cout << "   GPU " << i << ": "
                << "vendor: " << vendor_name << ", "
                //<< " (vendorID == 0x" << std::hex << props.vendorID << std::dec << ")"
                << "type: " << getGPUTypeDescription( vk_ava_gpus_props[i].deviceType) << ", "
                << "name: " << vk_ava_gpus_props[i].deviceName << endl ;
    }
}

// ---------------------------------------------------------------------------------------- 
// select a GPU from available GPUs (after enumeration)
// computes 'gpu_index' , or aborts when no suitable GPU is found.

void Device::selectGPU() 
{
    if ( ava_gpus_count == 1) // just one GPU, select it
        gpu_index = 0 ;
    else  
    {
        // try to find a discrete GPU
        gpu_index = ava_gpus_count ; 
        for ( uint32_t i = 0; i < ava_gpus_count; ++i )
        {   if ( vk_ava_gpus_props[i].deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU )
            {   gpu_index = i;
                break;
            }
        }

        // no discrete GPU found, select the first one which is not of type CPU or VIRTUAL
        if ( gpu_index == ava_gpus_count ) 
        {
            for ( uint32_t i = 0; i < ava_gpus_count; ++i )
            {   if ( vk_ava_gpus_props[i].deviceType != VK_PHYSICAL_DEVICE_TYPE_CPU && vk_ava_gpus_props[i].deviceType != VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU )
                {   gpu_index = i;
                    break;
                }
            }
        }
        // if no suitable GPU found, abort. 
        if ( gpu_index == ava_gpus_count ) 
        {
            std::cerr << "No suitable GPU found. Available GPUs:" << std::endl;
            exit(1);
        }
    }   
}

// ----------------------------------------------------------------------------------------
// Get memory properties  and the physical device features 
// of the selected GPU, and print some of them.
// Initializes: 'vk_mem_props', 'vk_available_feat'.

void Device::initializeGPUProperties() 
{
    vk_gpu = vk_ava_gpus[gpu_index] ;
    vk_gpu_props = vk_ava_gpus_props[gpu_index] ;
    vkGetPhysicalDeviceMemoryProperties( vk_gpu, &vk_mem_props ); // used later for vertex buffer creation

    std::cout << "Selected GPU " << gpu_index << ": " << vk_gpu_props.deviceName << std::endl ;

    vkGetPhysicalDeviceFeatures( vk_gpu, &vk_available_feat );
    likelyHasTessellation = (vk_available_feat.tessellationShader == VK_TRUE);
    likelyHasWireframeRender = (vk_available_feat.fillModeNonSolid == VK_TRUE);
    
    using namespace std ;
    cout
        << "GPU properties:" << endl
        << "  API version supported by the selected GPU    : " 
            << VK_VERSION_MAJOR(vk_gpu_props.apiVersion) << "." 
            << VK_VERSION_MINOR(vk_gpu_props.apiVersion) << "." 
            << VK_VERSION_PATCH(vk_gpu_props.apiVersion) << endl 
        << "  Driver version supported by the selected GPU : " 
            << VK_VERSION_MAJOR(vk_gpu_props.driverVersion) << "." 
            << VK_VERSION_MINOR(vk_gpu_props.driverVersion) << "." 
            << VK_VERSION_PATCH(vk_gpu_props.driverVersion) << endl
        << "  Max size for push constants block            : " << vk_gpu_props.limits.maxPushConstantsSize << " bytes" << endl 
        << "  Tessellation shader capability (guess)       : " << (likelyHasTessellation ? "likely yes" : "likely no") << endl 
        << "  Wireframe rendering capability (guess)       : " << (likelyHasWireframeRender ? "likely yes" : "likely no") << endl ;

}
// ----------------------------------------------------------------------------------------
// Get the list of available extensions and check the required extensiones (if any).
// Uses 'required_extensions_names' and updates 'supp_extensions' and 'supp_extensions_count'.

void Device::checkAvailableExtensions() 
{
    vkEnumerateDeviceExtensionProperties( vk_gpu, nullptr, &supp_extensions_count, nullptr );
    supp_extensions.resize( supp_extensions_count );
    vkEnumerateDeviceExtensionProperties( vk_gpu, nullptr, &supp_extensions_count, supp_extensions.data());
    std::cout << "Selected GPU supports " << supp_extensions_count << " Vulkan device extension(s)" << std::endl;

    // Do not print the whole list, unless when you want to debug it
    //for (const auto &ext : supp_extensions) 
    //    std::cout << "  - " << ext.extensionName << " (specVersion=" << ext.specVersion << ")" << std::endl;

    // check whether each required extension is supported
    
    for( uint32_t i = 0; i < required_extensions_count; ++i)
        if (! extensionIsSupported( required_extensions_names[i] ) )
            throw std::runtime_error("Selected GPU does not support required extension: " + std::string(required_extensions_names[i]));
    
}

// ----------------------------------------------------------------------------------------
// Create logical device and gets its queue

void Device::createLogicalDeviceAndGetQueue()
{
    VkDeviceQueueCreateInfo qci{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
    VkDeviceCreateInfo      dci{ VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };

    qci.queueFamilyIndex = 0 ;
    qci.queueCount = 1 ;
    qci.pQueuePriorities = &queuePriority ;

    dci.pQueueCreateInfos = &qci ;
    dci.queueCreateInfoCount = 1 ;
    dci.pEnabledFeatures = &vk_desired_feat ;    
    dci.enabledExtensionCount = required_extensions_count ;
    dci.ppEnabledExtensionNames = required_extensions_names ;

    if ( vkCreateDevice( vk_gpu, &dci, nullptr, &vk_device) != VK_SUCCESS )
    {   std::cerr << "Failed to create logical device for the selected GPU." << std::endl;
        exit(1);
    }
     
    // get the queue for this device 
    vkGetDeviceQueue( vk_device, 0, 0, &vk_queue ); 
    assert( vk_queue != VK_NULL_HANDLE );
}

// -------------------------------------------------------------------------------------------------
// Constructor

Device::Device( Instance * p_instance )
{
    instance = p_instance ;
    assert( instance != nullptr );
    
    enumerateAvailableGPUs() ;
    selectGPU() ;
    initializeGPUProperties();
    checkAvailableExtensions() ;
    createLogicalDeviceAndGetQueue() ;

    std::cout << "GPU selected, logical device and queue created." << std::endl ;
    
}
// --------------------------------------------------------------------------------------------------
// Submits a command buffer to the device queue 

void Device::submitCommandBuffer( VkCommandBuffer & vk_cmd_buffer, SyncObjects * sync_objects )
{
    assert( sync_objects != nullptr );
    assert( vk_cmd_buffer != VK_NULL_HANDLE );

    VkPipelineStageFlags waitStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSubmitInfo submit{VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submit.waitSemaphoreCount = 1;
    submit.pWaitSemaphores = &(sync_objects->imageAvailableSemaphore);
    submit.pWaitDstStageMask = &waitStages;

    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &vk_cmd_buffer;

    submit.signalSemaphoreCount = 1;
    submit.pSignalSemaphores = &(sync_objects->renderFinishedSemaphore);

    vkQueueSubmit( vk_queue, 1, &submit, sync_objects->inFlightFence);
}
// -------------------------------------------------------------------------------------------
// Find memory type index with the desired properties from a memory requeriment

uint32_t Device::findMemoryTypeIndex( VkMemoryRequirements & mem_req, VkMemoryPropertyFlags desired_props )  
{
    assert( mem_req.memoryTypeBits != 0 ); // check that there is at least one memory type available for the buffer/image
    assert( vk_mem_props.memoryTypeCount > 0 ); // check that there is at least one memory type in the physical device

    uint32_t type_index = UINT32_MAX;

    for ( uint32_t i = 0; i < vk_mem_props.memoryTypeCount; ++i ) 
    {
        const bool typeSupported = (mem_req.memoryTypeBits & (1u << i)) != 0 ;
        const VkMemoryPropertyFlags props = vk_mem_props.memoryTypes[i].propertyFlags;
        
        if ( typeSupported && ((props & desired_props) == desired_props) ) 
        {   type_index = i;
            break;
        }
    }
    if (type_index == UINT32_MAX) {
        throw std::runtime_error("No suitable memory type found with the desired properties");
    }
    return type_index ;
} 

// -------------------------------------------------------------------------------------------
// Create and VkBuffer and corresponding VkDeviceMemory, and copy data to the GPU 

void Device::createBufferAndCopyData( VkDeviceSize vk_total_size_bytes, const void* src_data, VkBufferUsageFlags usage_flags,
                                      VkBuffer & vk_buffer, VkDeviceMemory & vk_memory )
{
    VkBufferCreateInfo bci{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    bci.size = vk_total_size_bytes;
    bci.usage = usage_flags;
    bci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    vkCreateBuffer( vk_device, &bci, nullptr, &vk_buffer );

    VkMemoryRequirements mem_req;
    vkGetBufferMemoryRequirements( vk_device, vk_buffer, &mem_req );

    VkMemoryAllocateInfo mai{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
    mai.allocationSize = mem_req.size;
    mai.memoryTypeIndex = findMemoryTypeIndex( mem_req, 
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT );
    
    vkAllocateMemory( vk_device, &mai, nullptr, &vk_memory );
    vkBindBufferMemory( vk_device, vk_buffer, vk_memory, 0 );

    void* dst_data ;
    vkMapMemory( vk_device, vk_memory, 0, vk_total_size_bytes, 0, &dst_data );
    std::memcpy( dst_data, src_data, static_cast<size_t>(vk_total_size_bytes) );
    vkUnmapMemory( vk_device, vk_memory );
}



// ---------------------------------------------------------------------------
// Destructor 

Device::~Device()
{
    vkDestroyDevice( vk_device, nullptr );
    std::cout << "Deleted device" << std::endl ;
}

}  // fin del namespace vkhc 


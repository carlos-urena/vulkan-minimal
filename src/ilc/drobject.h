

#pragma once

#include <string>
#include <vkhc/common.h>

class DrawableObject 
{
    private:
        std::string name_str ;
        

    public:
        virtual void drawVK( vkhc::BasicPipeline * pipeline, vkhc::VulkanContext & context, VkCommandBuffer & cmd_vk ) = 0 ;
        virtual ~DrawableObject() {} ;
        virtual void drawIMGUIWidgets( )  ;
        const std::string & getName() const  ;
        void setName( const std::string & new_name ) ;
      
} ;
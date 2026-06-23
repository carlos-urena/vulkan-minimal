

#pragma once

#include <string>
#include <common.h>

class DrawableObject 
{
    private:
        std::string name_str ;

    public:
        virtual void drawVK( vkhc::VulkanContext & context, VkCommandBuffer & cmd_vk ) = 0 ;
        const std::string & getName() const  ;
        void setName( const std::string & new_name ) ;
      
} ;


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

class TransformedObject : public DrawableObject
{
    private:
        glm::mat4 tr_mat, tr_mat_normals ; 
        DrawableObject * drawable_object = nullptr ; // pointer to the drawable object to be drawn with the transformation matrix 'tr_mat'

    public:
        TransformedObject( const glm::mat4 & p_tr_mat, DrawableObject * p_drawable_object ) ;
        virtual void drawVK( vkhc::BasicPipeline * pipeline, vkhc::VulkanContext & context, VkCommandBuffer & cmd_vk ) override ;
} ;


class CompositeObject : public DrawableObject
{
    private:
        std::vector<DrawableObject *> objects ; // vector of pointers to drawable objects

    public:void add( DrawableObject * obj ) ;
        virtual void drawVK( vkhc::BasicPipeline * pipeline, vkhc::VulkanContext & context, VkCommandBuffer & cmd_vk ) override ;
        
} ;
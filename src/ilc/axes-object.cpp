#include <axes-object.h>
#include <pipeline3D.h>

// ---------------------------------------------------------------------------
// crea una tabla de índices para una rejilla con topología cilindrica

void CreateCilindricalTopologyTriangles( std::vector<glm::uvec3> & indices, 
                                            const unsigned na, const unsigned nb )
{
   indices.clear();

   for( unsigned i = 0 ; i < na ; i++ )
   for( unsigned j = 0 ; j < nb ; j++ )
   {        
      const unsigned int
         i00 = (i  )*nb + j,
         i10 = (i+1)*nb + j,
         i01 = (i  )*nb + ((j+1) % nb),
         i11 = (i+1)*nb + ((j+1) % nb);
   
      indices.push_back({ i00, i01, i11 });
      indices.push_back({ i00, i11, i10 });
   }
}

void CreateGridTopologyTriangles( std::vector<glm::uvec3> & indices, 
                                            const unsigned na, const unsigned nb )
{
   indices.clear();

   for( unsigned i = 0 ; i < na-1 ; i++ )
   for( unsigned j = 0 ; j < nb-1 ; j++ )
   {        
      const unsigned int
         i00 = (i  )*nb + j,
         i10 = (i+1)*nb + j,
         i01 = (i  )*nb + (j+1),
         i11 = (i+1)*nb + (j+1);
   
      indices.push_back({ i00, i01, i11 });
      indices.push_back({ i00, i11, i10 });
   }
}

// ---------------------------------------------------------------------------------------
// Indexed mesh with a single triangle 

class IMTriangle : public IndexedMesh
{
   public:
      IMTriangle( const std::vector<glm::vec3> & vertices, const glm::vec3 & color) ;   
} ;



IMTriangle::IMTriangle( const std::vector<glm::vec3> & p_vertices, const glm::vec3 & p_color ) 
{
   Assert( p_vertices.size() == 3, "IMTriangle: must have exactly 3 vertices" ) ;

   using namespace glm ;
   vec3 tri_normal = normalize( cross( p_vertices[1]-p_vertices[0], p_vertices[2]-p_vertices[0] ) ) ;
   
   for( unsigned iv = 0 ; iv < 3 ; iv++ )
   {
      vertices.push_back( p_vertices[iv] ) ;
      vert_colors.push_back( p_color ) ;
      vert_normals.push_back( tri_normal ) ;
      vert_tcc.push_back( { 0.0f, 0.0f } ) ; // texture coordinates not used
   }
   triangles.push_back( { 0, 1, 2 } ) ;
}


// ---------------------------------------------------------------------------------------

class CylinderZ01 : public IndexedMesh
{
  public:
    CylinderZ01( const std::string & name, const unsigned num_slices ) ;
} ;

// ---------------------------------------------------------------------------------------

CylinderZ01::CylinderZ01( const std::string & name, const unsigned num_slices )
   
:   IndexedMesh( )
{
    using namespace std ;
    constexpr unsigned int na = 2 ;
    const     unsigned     nb = num_slices ;  // number of rows and columns of vertexes 
    
    setName( name ) ;
    CreateGridTopologyTriangles( triangles, na, nb );  

    for( unsigned iz = 0 ; iz < na  ; iz++ )
    for( unsigned is = 0 ; is < nb ; is++ )
    {   
        const float fz = float(iz)/float(na-1), 
                    fs = float(is)/float(nb-1), 
                    a  = 2.0f*M_PI*fs, 
                    ca = cos(a), 
                    sa = sin(a) ;

        vertices.push_back( { ca, sa, fz } );   
        //vertices.push_back( { fs, +0.5f, fz } );
        vert_colors.push_back( { fs, fz, 0.0f } );
        vert_tcc.push_back( { fs, fz } );
        vert_normals.push_back( { ca, sa, 0.0f } );
    }
}

// ---------------------------------------------------------------------------------------


AxesObject::AxesObject( ) 
{
   using namespace glm ; 
   using namespace vkhc ;

   setName( "axes object" ) ;
   axes_cylinder = new CylinderZ01( "axes cylinder Z 01", 32 ) ; Assert( axes_cylinder != nullptr, "Cannot create axes cylinder" ) ;
   
   red_color_index   = BaseColorsSet::addBaseColor( vec3( 1.0f, 0.0f, 0.0f ) ) ;
   green_color_index = BaseColorsSet::addBaseColor( vec3( 0.0f, 1.0f, 0.0f ) ) ;
   blue_color_index  = BaseColorsSet::addBaseColor( vec3( 0.0f, 0.0f, 1.0f ) ) ;

   constexpr float d = 1.05, h = 0.15 ;

   xtri = new IMTriangle( 
      std::vector<glm::vec3>{  
         vec3( d,   0.0f, -h ), 
         vec3( d,   0.0f, +h ), 
         vec3( d+2*h, 0.0f, 0.0f ) 
      }, 
      vec3( 1.0f, 0.0f, 0.0f ) 
   ) ;   
   Assert( xtri != nullptr, "Cannot create X axis triangle" ) ;

   ytri = new IMTriangle( 
      {  vec3(  -h,  d,   0.0f ), 
         vec3(   h,  d,   0.0f ), 
         vec3( 0.0f, d+2*h, 0.0f ) 
      }, 
      vec3( 0.0f, 1.0f, 0.0f ) 
   ) ;   
   Assert( ytri != nullptr, "Cannot create Y axis triangle" ) ;

   ztri = new IMTriangle( 
      {  vec3(   -h, 0.0f, d ), 
         vec3(    h, 0.0f, d ), 
         vec3( 0.0f, 0.0f, d+2*h ) 
      }, 
      vec3( 0.0f, 0.0f, 1.0f ) 
   ) ;   
   Assert( ztri != nullptr, "Cannot create Z axis triangle" ) ;
}

// ---------------------------------------------------------------------------------------

AxesObject::~AxesObject() 
{
    delete axes_cylinder ; axes_cylinder = nullptr ;
    delete xtri ; xtri = nullptr ;
    delete ytri ; ytri = nullptr ;
    delete ztri ; ztri = nullptr ;
}

// ---------------------------------------------------------------------------------------

void AxesObject::drawVK( vkhc::BasicPipeline * pipeline, vkhc::VulkanContext & context, VkCommandBuffer & cmd_vk ) 
{
   Assert( axes_cylinder != nullptr, "Axes cylinder not initialized" ) ;

   // retrieve a valid 3d pipeline  
   using namespace vkhc ;
   using namespace glm ;
   
   Assert( pipeline != nullptr, "No pipeline binded when drawing axes object" ) ;
   Pipeline3D * pipeline3d = static_cast<Pipeline3D*>( pipeline ) ;
   Assert( pipeline3d != nullptr, "Current binded pipeline is not a 3D pipeline when drawing axes object" ) ;

   constexpr float radius = 0.03f ;
   const mat4 
      scale_mat = glm::scale(glm::vec3( radius, radius, 1.0f)),
      rot_m90_x = glm::rotate( radians(-90.0f), glm::vec3(1.0,0.0,0.0) ),
      rot_90_y  = glm::rotate( radians(90.0f),  glm::vec3(0.0,1.0,0.0) );

   // save previous pipeline state
   int prev_texture_index = pipeline3d->getTextureIndex() ; // save previous texture index
   int prev_base_color_index = pipeline3d->getBaseColorIndex() ; // save previous base color index

   // disable uso of textures
   pipeline3d->setTextureIndex( cmd_vk, -1 ) ; // disable use of textures

   pipeline3d->pushModelMatrix( cmd_vk, scale_mat ) ; 
   pipeline3d->setBaseColorIndex( cmd_vk, blue_color_index ) ;
   axes_cylinder->drawVK( pipeline, context, cmd_vk ) ; // axis Z (blue)
   pipeline3d->popModelMatrix( cmd_vk ) ;

   pipeline3d->pushModelMatrix( cmd_vk, rot_m90_x* scale_mat ) ;
   pipeline3d->setBaseColorIndex( cmd_vk, green_color_index ) ;
   axes_cylinder->drawVK( pipeline, context, cmd_vk ) ; // axis Y (green)
   pipeline3d->popModelMatrix( cmd_vk );

   pipeline3d->pushModelMatrix( cmd_vk, rot_90_y *scale_mat) ;
   pipeline3d->setBaseColorIndex( cmd_vk, red_color_index ) ;
   axes_cylinder->drawVK( pipeline, context, cmd_vk ) ; // axis X (red)
   pipeline3d->popModelMatrix( cmd_vk );

   pipeline3d->setBaseColorIndex( cmd_vk, red_color_index ) ;
   xtri->drawVK( pipeline, context, cmd_vk ) ; // axis X (red)
   pipeline3d->setBaseColorIndex( cmd_vk, green_color_index ) ;
   ytri->drawVK( pipeline, context, cmd_vk ) ; // axis Y (green)
   pipeline3d->setBaseColorIndex( cmd_vk, blue_color_index ) ;
   ztri->drawVK( pipeline, context, cmd_vk ) ; // axis Z (blue)

   // disable base color for next objects to be drawn
   pipeline3d->setBaseColorIndex( cmd_vk,  prev_base_color_index ) ; // restore previous base color index
   pipeline3d->setTextureIndex( cmd_vk, prev_texture_index ) ; // restore previous texture index
}

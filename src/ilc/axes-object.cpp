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
    setName( "axes object" ) ;
    axes_cylinder = new CylinderZ01( "axes cylinder Z 01", 32 ) ; Assert( axes_cylinder != nullptr, "Cannot create axes cylinder" ) ;
}

// ---------------------------------------------------------------------------------------

AxesObject::~AxesObject() 
{
    delete axes_cylinder ; axes_cylinder = nullptr ;
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

   constexpr float radius = 0.05f ;
   const mat4 scale_mat = glm::scale(glm::vec3( radius, radius, 1.0f)) ;

   pipeline3d->pushModelMatrix( cmd_vk, scale_mat ) ; 
   axes_cylinder->drawVK( pipeline, context, cmd_vk ) ; // axis Z
   pipeline3d->popModelMatrix( cmd_vk ) ;

   pipeline3d->pushModelMatrix( cmd_vk, glm::rotate(-float(M_PI/2.0), glm::vec3(1.0,0.0,0.0))* scale_mat ) ;
   axes_cylinder->drawVK( pipeline, context, cmd_vk ) ; // axis Y
   pipeline3d->popModelMatrix( cmd_vk );

   pipeline3d->pushModelMatrix( cmd_vk, glm::rotate( float(M_PI/2.0), glm::vec3(0.0,1.0,0.0)) *scale_mat) ;
   axes_cylinder->drawVK( pipeline, context, cmd_vk ) ; // axis X
   pipeline3d->popModelMatrix( cmd_vk );
}

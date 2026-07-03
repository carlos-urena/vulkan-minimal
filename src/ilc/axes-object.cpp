#include <axes-object.h>

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

   for( unsigned i = 0 ; i < na ; i++ )
   for( unsigned j = 0 ; j < nb ; j++ )
   {        
      const unsigned int
         i00 = (i  )*nb + j,
         i10 = (i+1)*nb + j,
         i01 = (i  )*nb + (j+1),
         i11 = (i+1)*nb + (j+1);
   
      indices.push_back({ i00, i01, i11 });
      indices.push_back({ i00, i11, i10 });

      //indices.push_back({ i00, i11, i01 });
      //indices.push_back({ i00, i10, i11 });
   }
}

// ---------------------------------------------------------------------------------------

CylinderZ01::CylinderZ01( const std::string & name, int num_slices )
   
:   IndexedMesh( )
{
    using namespace std ;
    constexpr unsigned int nz = 64, ns = 64 ;  // number of slices and sectors 
    
    setName( name ) ;
    CreateGridTopologyTriangles( triangles, nz, ns );  

    for( unsigned iz = 0 ; iz < nz ; iz++ )
    for( unsigned is = 0 ; is < ns ; is++ )
    {   
        const float fz = float(iz)/float(nz-1), 
                    fs = float(is)/float(ns-1) ,
                    a  = 2.0f*M_PI*fs, 
                    ca = cos(a), 
                    sa = sin(a) ;

        vertices.push_back( { ca, sa, fz } );
        vert_colors.push_back( { fs, fz, 0.0f } );
        vert_tcc.push_back( { fs, fz } );
        vert_normals.push_back( { ca, sa, 0.0f } );
    }

   //  vertices.push_back( { 0.0f, 0.0f, 0.0f } ); // center of the base
   //  vertices.push_back( { 0.0f, 3.0f, 0.0f } ); // center of the top
   //  vertices.push_back( { 3.0f, 0.0f, 0.0f } ); // point on the base circle
    
   //  vert_colors.push_back( { 1.0f, 0.0f, 0.0f } );
   //  vert_colors.push_back( { 0.0f, 1.0f, 0.0f } );
   //  vert_colors.push_back( { 0.0f, 0.0f, 1.0f } );

   //  vert_tcc.push_back( { 0.0f, 0.0f } );
   //  vert_tcc.push_back( { 1.0f, 0.0f } );
   //  vert_tcc.push_back( { 0.0f, 1.0f } );

   //  vert_normals.push_back( { 0.0f, 1.0f, 0.0f } );
   //  vert_normals.push_back( { 0.0f, 1.0f, 0.0f } );
   //  vert_normals.push_back( { 0.0f, 1.0f, 0.0f } );

    triangles.push_back( { 0, 1, 2 } ); // base triangle
}


CylinderZ01::~CylinderZ01()
{
    // nothing to do
}

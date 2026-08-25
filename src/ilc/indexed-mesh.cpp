
// *********************************************************************

#include <limits>
#include <set>

#include <vkhc/pipeline3D.h>
#include <ilc/indexed-mesh.h> 
#include <ilc/ply-reader.h>

namespace ilc 
{

// *****************************************************************************
// métodos de la clase IndexedMesh.

IndexedMesh::IndexedMesh()
{
   // nombre por defecto
   setName("malla indexada, anónima");
}
// -----------------------------------------------------------------------------

IndexedMesh::IndexedMesh( const std::string & nombreIni )
{
   // 'identificador' puesto a 0 por defecto, 'centro_oc' puesto a (0,0,0)
   setName(nombreIni) ;
}
// -----------------------------------------------------------------------------

IndexedMesh::~IndexedMesh()
{
   using namespace std ;
   //cout << "Invocado desructor de IndexedMesh (borra tablas) '" << leerNombre() << "'" << endl ;

   vertices.clear();
   triangles.clear();
   vert_colors.clear();
   vert_normals.clear();
   tri_normals.clear();
   vert_tcc.clear();
   normals_segments.clear();
   
   delete dvao ;          
   delete normals_va ;
   delete edges_va ;
   
}

//-----------------------------------------------------------------------------
// calcula la tabla de normales de triángulos una sola vez, si no estaba calculada

void IndexedMesh::computeTriangleNormals()
{
   using namespace std ;
   
   // si ya está creada la tabla de normales de triángulos, no es necesario volver a crearla
   const unsigned nt = triangles.size() ;
   assert( 1 <= nt );
   if ( 0 < tri_normals.size() )
   {
      assert( nt == tri_normals.size() );
      return ;
   }

   // Creación de la tabla de normales de triángulos
   tri_normals.resize( nt ) ;
   unsigned long nt_mal = 0 ;

   cout << "IndexedMesh::computeTriangleNormals: computing triangle normals for mesh '" << getName() 
        << "', nt == " << nt << ", nv = " << vertices.size() << " ......." << endl ;

   for( unsigned it = 0 ; it < nt ; it++ )
   {
      uint32_t iv0 = triangles[it][0],
               iv1 = triangles[it][1],
               iv2 = triangles[it][2] ;

      Assert( iv0 < vertices.size(), "IndexedMesh::computeTriangleNormals: vertex index 0 out of range: "+ to_string(iv0) ) ;
      Assert( iv1 < vertices.size(), "IndexedMesh::computeTriangleNormals: vertex index 1 out of range: "+ to_string(iv1) ) ;
      Assert( iv2 < vertices.size(), "IndexedMesh::computeTriangleNormals: vertex index 2 out of range: "+ to_string(iv2) ) ;

      const glm::vec3
         & v0 = vertices[iv0],
         & v1 = vertices[iv1],
         & v2 = vertices[iv2],
         e1   = v1-v0,
         e2   = v2-v0,
         n    = cross(e1,e2) ;
      const float ln = length( n ) ; //sqrt(n.lengthSq());
      if  ( ln > 1e-8 )
         tri_normals[it] = n/ln ;
      else
      {
         tri_normals[it] = glm::vec3(0.0,0.0,0.0);
         nt_mal++ ;
      }
   }
}


// -----------------------------------------------------------------------------
// calcula las dos tablas de normales

void IndexedMesh::computeNormals()
{
   using namespace std ;
   using namespace glm ;

   cout << "IndexedMesh::computeNormals: computing normals for mesh '" << getName() << "' ..." << endl ;
   
   // Calculo de las normales de la malla
   // se debe invocar en primer lugar 'computeTriangleNormals'
   
   if ( tri_normals.size() == 0 )
      computeTriangleNormals();

   assert( tri_normals.size() == triangles.size() );

   if ( vert_normals.size() > 0 )
   {         
      assert( vert_normals.size() == vertices.size() );
      cout << "IndexedMesh::computeNormals: normals already computed for mesh '" << getName() << "' (SHOULD NOT HAPPEN?)." << endl ;
      return ;
   }
   const unsigned nv = vertices.size(),
                  nt = triangles.size() ;

   assert( 2 <= nv );
   assert( 1 <= nt );

   // calcular normales de vértices:
   assert( vert_normals.size() == 0 );
   vert_normals.resize( nv );

   for( unsigned iv = 0 ; iv < nv ; iv++ )
      vert_normals[iv] = glm::vec3(0.0,0.0,0.0);

   for( unsigned it = 0 ; it < nt ; it++ )
   {
      vec3
         & nt  = tri_normals[it],
         & nv0 = vert_normals[triangles[it][0]],
         & nv1 = vert_normals[triangles[it][1]],
         & nv2 = vert_normals[triangles[it][2]] ;

      nv0 = nv0+nt ;
      nv1 = nv1+nt ;
      nv2 = nv2+nt ;
   }

   for( unsigned iv = 0 ; iv < nv ; iv++ )
   {
      const float ln = length( vert_normals[iv] );
      if ( ln > 1e-5 )
         vert_normals[iv] = vert_normals[iv]/ln ;
      else
         vert_normals[iv] = glm::vec3(0.0,1.0,0.0);
   }
   Assert( vert_normals.size() == vertices.size(), "IndexedMesh::computeNormals: error in normals table size" ) ;
   cout << "IndexedMesh::computeNormals: normals computed for mesh '" << getName() << "'." << endl ;
}

// ------------------------------------------------------------------------
// compute edges vertices ('edges_vertices')
void IndexedMesh::computeEdgesVertices() 
{
   if ( edges_vertices.size() > 0 )
      return ; // already computed

   for( const glm::uvec3 & tri : triangles )
   {
      edges_vertices.push_back( vertices[tri[0]] );
      edges_vertices.push_back( vertices[tri[1]] );

      edges_vertices.push_back( vertices[tri[1]] );
      edges_vertices.push_back( vertices[tri[2]] );

      edges_vertices.push_back( vertices[tri[2]] );
      edges_vertices.push_back( vertices[tri[0]] );
   }
}

// ------------------------------------------------------------------------
// compute segments for normals visualization

void IndexedMesh::computeVisibleNormalsVertices() 
{
   if ( normals_segments.size() > 0 )
      return ; // already computed
   
   Assert( vert_normals.size() == vertices.size(), "IndexedMesh::computeVisibleNormalsVertices: vert_normals size does not match vertices size" ) ;

   for( unsigned iv = 0 ; iv < vertices.size() ; iv++ )
   {
      const glm::vec3 & v = vertices[iv] ;
      const glm::vec3 & n = vert_normals[iv] ;

      normals_segments.push_back( v );
      normals_segments.push_back( v + 0.2f*n );
   }
}

// --------------------------------------------------------------------------------------------

void IndexedMesh::drawVK( vkhc::BasicPipeline * pipeline, vkhc::VulkanContext & context, VkCommandBuffer & cmdb_vk )
{
   using namespace std ;

   // Crear el descriptor de VAO, si no está creado
   //  Si el puntero 'dvao' es nulo, crear el descriptor de VAO (se usan las tablas de vértices, triángulos y atributos de la malla)
   //  Si el VAO ya está creado, (dvao no nulo), no hay que hacer nada.
   
   if ( dvao == nullptr ) 
   {
      using namespace std ;
      cout << "IndexedMesh::drawVK: creating VAO for mesh '" << getName() << "' ..." << endl ;
      cout << "IndexedMesh::drawVK: vertices.size() = " << vertices.size() 
           << ", vert_colors.size() = " << vert_colors.size()
           << ", vert_normals.size() = " << vert_normals.size()
           << ", vert_tcc.size() = " << vert_tcc.size()
           << ", triangles.size() = " << triangles.size()
           << endl ;
      dvao = new vkhc::VertexArray( context, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 4 );
      Assert( dvao != nullptr, "cannot create a VAO" );

      dvao->setAttribData( 0, vertices );
      dvao->setAttribData( 1, vert_colors );
      dvao->setAttribData( 2, vert_normals );
      dvao->setAttribData( 3, vert_tcc );

      dvao->setIndexData( triangles );

   }

   // draw the VAO
   dvao->draw( cmdb_vk );


   // Get the 3d pipeline or null 
   vkhc::Pipeline3D * pipeline3d = static_cast<vkhc::Pipeline3D *>(pipeline) ;
   
   // Draw edges if requested and if it is a 3D pipeline
   if ( pipeline3d != nullptr )  
   if ( pipeline3d->getDrawWireframe() )
   {
      if ( edges_vertices.size() == 0 )
         computeEdgesVertices();

      Assert( edges_vertices.size() > 0, "IndexedMesh::drawVK: cannot draw edges, edges_vertices is empty" ) ;

      if ( edges_va == nullptr )
      {
         edges_va = new vkhc::VertexArray( context, VK_PRIMITIVE_TOPOLOGY_LINE_LIST, 1 );
         edges_va->setAttribData( 0, edges_vertices );
      }

      // save previous state
      const bool ilum     = pipeline3d->getEvalIllumination() ;
      const int  itext    = pipeline3d->getTextureIndex()  ;
      const int  colorind = pipeline3d->getBaseColorIndex() ;
      const bool wiremode = pipeline3d->getWireframeMode() ;

      // set state
      pipeline3d->setEvalIllumination( cmdb_vk, false );
      pipeline3d->setTextureIndex( cmdb_vk, -1 );
      pipeline3d->setBaseColorIndex( cmdb_vk, 0 ); // black ?? which ??
      pipeline3d->setWireframeMode( cmdb_vk, true ); // draw edges as wireframe

      // draw
      edges_va->draw( cmdb_vk );

      // restore previous state
      pipeline3d->setEvalIllumination( cmdb_vk, ilum );
      pipeline3d->setTextureIndex( cmdb_vk, itext );
      pipeline3d->setBaseColorIndex( cmdb_vk, colorind );
      pipeline3d->setWireframeMode( cmdb_vk, wiremode );
   }

   // Draw the normals if requested and if it is a 3D pipeline
   if ( pipeline3d != nullptr )
   if ( pipeline3d->getDrawNormals() )
   {
     
      if ( normals_segments.size() == 0 )
         computeVisibleNormalsVertices();
         
      Assert( normals_segments.size() > 0, "IndexedMesh::drawVK: cannot draw normals, normals_segments is empty" ) ;

      if ( normals_va == nullptr )
      {
         normals_va = new vkhc::VertexArray( context, VK_PRIMITIVE_TOPOLOGY_LINE_LIST, 1 );
         Assert( normals_va != nullptr, "IndexedMesh::drawVK: cannot create normals VAO" ) ;
         normals_va->setAttribData( 0, normals_segments );
      }

      // save previous state
      const bool ilum     = pipeline3d->getEvalIllumination() ;
      const int  itext    = pipeline3d->getTextureIndex()  ;
      const int  colorind = pipeline3d->getBaseColorIndex() ;
      
      // set state
      pipeline3d->setEvalIllumination( cmdb_vk, false );
      pipeline3d->setTextureIndex( cmdb_vk, -1 );
      pipeline3d->setBaseColorIndex( cmdb_vk, 0 ); // red ??
      

      // draw
      normals_va->draw( cmdb_vk );

      // restore previous state
      pipeline3d->setEvalIllumination( cmdb_vk, ilum );
      pipeline3d->setTextureIndex( cmdb_vk, itext );
      pipeline3d->setBaseColorIndex( cmdb_vk, colorind );
      
   }

}


// ****************************************************************************
// Clase 'PLYMesh'

PLYMesh::PLYMesh( const std::string & nombre_arch )
{
   setName( std::string("PLY mesh in '") + nombre_arch + "'" );
   ilc::ReadPLY( nombre_arch, vertices, triangles );
   computeNormals(); // calcular la tabla de normales

   // add vertex colors and texture coordinates (invent)

   for( glm::vec3 v : vertices )
   {
      vert_colors.push_back( glm::vec3( 0.5f*(v.x+1.0f), 0.5f*(v.y+1.0f), 0.5f*(v.z+1.0f) ) );
      vert_tcc.push_back( glm::vec2( 0.5f*(v.x+1.0f), 0.5f*(v.y+1.0f) ) );
   }

   using namespace std ;
   cout << "PLYMesh::PLYMesh: mesh '" << getName() << "' created." 
        << "nv = " << vertices.size() << endl  
        << "nt = " << triangles.size() << endl ;
}

// ****************************************************************************
// Clase 'Cubo

Cube8::Cube8()
:  IndexedMesh( "8 vertexes cube" )
{

   vertices =
      {  { -1.0, -1.0, -1.0 }, // 0
         { -1.0, -1.0, +1.0 }, // 1
         { -1.0, +1.0, -1.0 }, // 2
         { -1.0, +1.0, +1.0 }, // 3
         { +1.0, -1.0, -1.0 }, // 4
         { +1.0, -1.0, +1.0 }, // 5
         { +1.0, +1.0, -1.0 }, // 6
         { +1.0, +1.0, +1.0 }, // 7
      } ;

   
   for( glm::vec3 v : vertices )
      vert_normals.push_back( normalize(v) );
   

   triangles =
      {  {0,1,3}, {0,3,2}, // X-
         {4,7,5}, {4,6,7}, // X+ (+4)

         {0,5,1}, {0,4,5}, // Y-
         {2,3,7}, {2,7,6}, // Y+ (+2)

         {0,6,4}, {0,2,6}, // Z-
         {1,5,7}, {1,7,3}  // Z+ (+1)
      } ;

}
// -----------------------------------------------------------------------------------------------




// Cubo de lado 2 con centro en el origen, 6x4=24 vértices
// Cubo con normales y coordenadas de textura

Cube24::Cube24()
{
   setName("24 vertexes cube");
   

   // crear el objeto:
   vertices =
      {
         // Cara X-
         { -1.0, -1.0, -1.0 },  // 0
         { -1.0, -1.0, +1.0 },  // 1
         { -1.0, +1.0, -1.0 },  // 2
         { -1.0, +1.0, +1.0 },  // 3

         // Cara X+
         { +1.0, -1.0, -1.0 },  // 4
         { +1.0, -1.0, +1.0 },  // 5
         { +1.0, +1.0, -1.0 },  // 6
         { +1.0, +1.0, +1.0 },  // 7

         // Cara Y-
         { -1.0, -1.0, -1.0 },  // 8
         { -1.0, -1.0, +1.0 },  // 9
         { +1.0, -1.0, -1.0 },  // 10
         { +1.0, -1.0, +1.0 },  // 11

         // Cara Y+
         { -1.0, +1.0, -1.0 },  // 12
         { -1.0, +1.0, +1.0 },  // 13
         { +1.0, +1.0, -1.0 },  // 14
         { +1.0, +1.0, +1.0 },  // 15


         // Cara Z-
         { -1.0, -1.0, -1.0 },  // 16
         { -1.0, +1.0, -1.0 },  // 17
         { +1.0, -1.0, -1.0 },  // 18
         { +1.0, +1.0, -1.0 },  // 19

         // Cara Z+
         { -1.0, -1.0, +1.0 },  // 20
         { -1.0, +1.0, +1.0 },  // 21
         { +1.0, -1.0, +1.0 },  // 22
         { +1.0, +1.0, +1.0 },  // 23

      } ;

   for ( glm::vec3 v : vertices )
      vert_colors.push_back( glm::vec3( 0.5f*(v.x+1.0f), 0.5f*(v.y+1.0f), 0.5f*(v.z+1.0f) ) );

   triangles =
      {
         // Cara X-
         { 1, 3, 2 },
         { 0, 1, 2 },

         // Cara X+
         { 4, 6, 5 },
         { 5, 6, 7 },

         // Cara Y-
         { 8, 10,  9 },
         { 9, 10, 11 },

         // Cara Y+
         { 13, 15, 14 },
         { 12, 13, 14 },

         // Cara Z-
         { 17, 19, 18 },
         { 16, 17, 18 },

         // Cara Z+
         { 20, 22, 21 },
         { 21, 22, 23 }

      } ;

   vert_normals =
      {
         // Cara X-
         { -1.0,  0.0, 0.0 },
         { -1.0,  0.0, 0.0 },
         { -1.0,  0.0, 0.0 },
         { -1.0,  0.0, 0.0 },

         // Cara X+
         { +1.0,  0.0, 0.0 },
         { +1.0,  0.0, 0.0 },
         { +1.0,  0.0, 0.0 },
         { +1.0,  0.0, 0.0 },

         // Cara Y-
         { 0.0, -1.0, 0.0 },
         { 0.0, -1.0, 0.0 },
         { 0.0, -1.0, 0.0 },
         { 0.0, -1.0, 0.0 },

         // Cara Y+
         { 0.0, +1.0, 0.0 },
         { 0.0, +1.0, 0.0 },
         { 0.0, +1.0, 0.0 },
         { 0.0, +1.0, 0.0 },

         // Cara Z-
         { 0.0, 0.0, -1.0 },
         { 0.0, 0.0, -1.0 },
         { 0.0, 0.0, -1.0 },
         { 0.0, 0.0, -1.0 },

         // Cara Z+
         { 0.0, 0.0, +1.0 },
         { 0.0, 0.0, +1.0 },
         { 0.0, 0.0, +1.0 },
         { 0.0, 0.0, +1.0 }

      } ;

   vert_tcc =
      {
         // Cara X- (1)
         { 0.0, 1.0 },
         { 1.0, 1.0 },
         { 0.0, 0.0 },
         { 1.0, 0.0 },

         // Cara X+ (6)
         { 1.0, 1.0 },
         { 0.0, 1.0 },
         { 1.0, 0.0 },
         { 0.0, 0.0 },

         // Cara Y- (2)
         { 0.0, 1.0 },
         { 0.0, 0.0 },
         { 1.0, 1.0 },
         { 1.0, 0.0 },

         // Cara Y+ (5)
         { 0.0, 0.0 },
         { 0.0, 1.0 },
         { 1.0, 0.0 },
         { 1.0, 1.0 },

         // Cara Z- (3)
         { 1.0, 1.0 },
         { 1.0, 0.0 },
         { 0.0, 1.0 },
         { 0.0, 0.0 },

         // Cara Z+  (4)
         { 0.0, 1.0 },
         { 0.0, 0.0 },
         { 1.0, 1.0 },
         { 1.0, 0.0 }

      } ;

   //computeNormals() ; 
}


// *****************************************************************************

} // end of namespace ilc
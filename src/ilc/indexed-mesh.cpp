
// *********************************************************************

#include <limits>
#include <set>

#include <indexed-mesh.h> 

// *****************************************************************************
// funciones auxiliares

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
   
   delete dvao ;          dvao = nullptr ;
   delete dvao_normales ; dvao_normales = nullptr ;
}

//-----------------------------------------------------------------------------
// calcula la tabla de normales de triángulos una sola vez, si no estaba calculada

void IndexedMesh::computeTriangleNormals()
{

   
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

   for( unsigned it = 0 ; it < nt ; it++ )
   {
      const glm::vec3
         & v0 = vertices[triangles[it][0]],
         & v1 = vertices[triangles[it][1]],
         & v2 = vertices[triangles[it][2]],
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
   using namespace glm ;
   
   // Calculo de las normales de la malla
   // se debe invocar en primer lugar 'computeTriangleNormals'
   
   if ( tri_normals.size() == 0 )
      computeTriangleNormals();

   assert( tri_normals.size() == triangles.size() );

   if ( vert_normals.size() > 0 )
   {         
      assert( vert_normals.size() == vertices.size() );
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
      dvao = new vkhc::VertexArray( context, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 4 );
      Assert( dvao != nullptr, "cannot create a VAO" );

      dvao->setAttribData( 0, vertices );
      dvao->setAttribData( 1, vert_colors );
      dvao->setAttribData( 2, vert_normals );
      dvao->setAttribData( 3, vert_tcc );

      dvao->setIndexData( triangles );

   }
   dvao->draw( cmdb_vk );
}


// ****************************************************************************
// Clase 'PLYMesh'

PLYMesh::PLYMesh( const std::string & nombre_arch )
{
   setName( std::string("Malla en archivo PLY (") + nombre_arch + ")" );
   //LeerPLY( nombre_arch, vertices, triangles );
   computeNormals(); // calcular la tabla de normales
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

   computeNormals() ; 
}


// *****************************************************************************


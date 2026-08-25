

#pragma once

#include <vector>       // usar std::vector

#include <vkhc/vulkan-context.h>
#include <vkhc/vertex-array.h>
#include <ilc/drobject.h>


namespace ilc 
{

// ---------------------------------------------------------------------
///
/// @brief  Indexed triangle mesh class 
///
class IndexedMesh : public DrawableObject
{
   
   protected:
      // COMPLETAR: incluir aquí las variables y métodos privados que sean
      // necesarios para una malla indexada (y que no aparezcan ya declarados en esta plantilla)
      // ......

      std::vector<glm::vec3>  vertices ;
      std::vector<glm::uvec3> triangles ;

      std::vector<glm::vec3> vert_colors ;   // colores de los vértices
      std::vector<glm::vec3> vert_normals ;   // normales de vértices
      std::vector<glm::vec3> tri_normals ;   // normales de triángulos
      std::vector<glm::vec2> vert_tcc ; // coordenadas de textura de los vértices
      std::vector<glm::vec3> edges_vertices ; // positions of vertices (2 consecutiver vertex for each edge) for visualizing edges of the mesh
      std::vector<glm::vec3> normals_segments ; // guarda los segmentos de normales

      // descriptor del VAO con los vértices, triángulos y atributos de esta malla indexada
      // (se crea bajo demanda en 'visualizarGL')
      vkhc::VertexArray * dvao = nullptr  ;

      
      // vertex array for normals visualization
      vkhc::VertexArray * normals_va = nullptr ;

      // vertex array for edges visualization
      vkhc::VertexArray * edges_va = nullptr ; // edges vertex array
      

      // normales de triángulos y vértices
      void computeNormals();

      // calculo de las normales de triángulos (solo si no están creadas ya)
      void computeTriangleNormals() ;

      // compute edges vertices ('edges_vertices')
      void computeEdgesVertices() ;

      // compute segments for normals visualization 
      void computeVisibleNormalsVertices() ;
      

   public:
      // crea una malla vacía (nombre: "malla indexada nueva vacía")
      IndexedMesh() ;
      virtual ~IndexedMesh() override ;

      // crea una malla vacía con un nombre concreto:
      IndexedMesh( const std::string & nombreIni );

      // visualizar el objeto con OpenGL, métodos virtuales 

      virtual void drawVK( vkhc::BasicPipeline * pipeline, vkhc::VulkanContext & context, VkCommandBuffer & cmd_vk ) override ;
      
} ;
// ---------------------------------------------------------------------
// Clase para mallas obtenidas de un archivo 'ply'
// es un tipo de malla indexada que define un nuevo constructor
// que recibe el nombre del archivo ply como parámetro

class PLYMesh : public IndexedMesh
{
   public:
      PLYMesh( const std::string & file_name ) ;
} ;

// ---------------------------------------------------------------------

class Cube8 : public IndexedMesh
{
  public:
      Cube8();
} ;

// ---------------------------------------------------------------------

class Cube24 : public IndexedMesh
{
  public:
      Cube24();
} ;

} // end namespace ilc
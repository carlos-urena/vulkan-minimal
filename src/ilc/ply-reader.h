

#pragma once


#include <string>
#include <vector>

#include <glm/glm.hpp>


// **********************************************************************
// **
// ** ply::read
// **
// **  lee un archivo ply y lo carga en 'vertices' y 'caras'
// **
// **   - 'nombre_archivo' nombre del archivo (se le añade .ply si no
// **     acaba en .ply)
// **   - si hay un error, aborta
// **   - elimina cualquier contenido previo en los
// **     vectores 'vertices' y 'caras'
// **   - lee el archivo .ply y lo carga en 'vertices' y 'faces'
// **   - solo admite plys con triángulos,
// **   - no lee colores, coordenadas de textura, ni normales.
// **
// *********************************************************************

void LeerPLY
(
   const std::string &       nombre_archivo_pse, // entrada: nombre de archivo
   std::vector<glm::vec3>  & vertices,           // salida:  vector de coords. de vert.
   std::vector<glm::uvec3> & caras               // salida:  vector de triángulos (índices)
);


// **********************************************************************
// **
// ** ply::read_vertices
// **
// **  lee los vértices de un archivo ply y los carga en 'vertices'
// **
// **   - 'nombre_archivo' nombre del archivo (se le añade .ply si no
// **     acaba en .ply)
// **   - si hay un error, aborta
// **   - elimina cualquier contenido previo en el
// **     vector 'vertices'
// **   - lee el archivo .ply y carga los vértices en 'vertices'
// **   - no lee colores, caras, coordenadas de textura, ni normales.
// **   - se ignora la información de caras
// **
// *********************************************************************

void LeerVerticesPLY
(
   const std::string &      nombre_archivo_pse, // entrada: nombre de archivo
   std::vector<glm::vec3> & vertices            // salida:  vector de coords. de vert.
);




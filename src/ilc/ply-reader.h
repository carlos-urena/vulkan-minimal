#pragma once


#include <string>
#include <vector>

#include <glm/glm.hpp>

namespace ilc 
{

// **********************************************************************
// **
// ** ply::read
// **
// **  reads a ply file and loads it into 'vertices' and 'faces'
// **
// **   - 'nombre_archivo' file name (adds .ply if it does not end
// **     with .ply)
// **   - if there is an error, aborts
// **   - clears any previous contents in the
// **     vectors 'vertices' and 'faces'
// **   - reads the .ply file and loads it into 'vertices' and 'faces'
// **   - only supports triangle ply files,
// **   - does not read colors, texture coordinates, or normals.
// **
// *********************************************************************

void ReadPLY
(
   const std::string &       nombre_archivo_pse, // input: file name
   std::vector<glm::vec3>  & vertices,           // output: vector of vertex coords.
   std::vector<glm::uvec3> & caras               // output: vector of triangles (indices)
);


// **********************************************************************
// **
// ** ply::read_vertices
// **
// **  reads the vertices of a ply file and loads them into 'vertices'
// **
// **   - 'nombre_archivo' file name (adds .ply if it does not end
// **     with .ply)
// **   - if there is an error, aborts
// **   - clears any previous contents in the
// **     vector 'vertices'
// **   - reads the .ply file and loads the vertices into 'vertices'
// **   - does not read colors, faces, texture coordinates, or normals.
// **   - face information is ignored
// **
// *********************************************************************

void ReadPLYVertexes
(
   const std::string &      nombre_archivo_pse, // input: file name
   std::vector<glm::vec3> & vertices            // output: vector of vertex coords.
);



} // end of namespace ilc
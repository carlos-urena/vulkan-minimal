// *********************************************************************
// **
// ** PLY file reader (vertices and faces only)
// ** Implementation
// **
// ** Carlos Ureña - 2012- 2019
// **
// **
// ** This program is free software: you can redistribute it and/or modify
// ** it under the terms of the GNU General Public License as published by
// ** the Free Software Foundation, either version 3 of the License, or
// ** (at your option) any later version.
// **
// ** This program is distributed in the hope that it will be useful,
// ** but WITHOUT ANY WARRANTY; without even the implied warranty of
// ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// ** GNU General Public License for more details.
// **
// ** You should have received a copy of the GNU General Public License
// ** along with this program.  If not, see <http://www.gnu.org/licenses/>.
// **
// *********************************************************************


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <iostream>
#include <fstream>
#include <cmath>
#include <limits>
#include <cassert>


#include <ilc/utils.h>
#include <ilc/ply-reader.h>

namespace ilc 
{

// Example PLY files are available at:
   // http://graphics.stanford.edu/data/3Dscanrep/
   // http://graphics.im.ntu.edu.tw/~robin/courses/cg03/model/
   // http://people.sc.fsu.edu/~jburkardt/data/ply/ply.html

using namespace std ;

//**********************************************************************
// Constants and auxiliary functions (private)

static constexpr streamsize tam_buffer = streamsize(10L)*streamsize(1024L) ;

// Class containing the state of the file parsing process and
// providing several methods for performing that parsing

class PLYReader
{
   public:

   ifstream       src ;                        // input stream
   char           buffer[ (unsigned long)tam_buffer ]; // buffer for reading through the end of a line
   unsigned long  num_linea_actual = 0 ;       // number of the line currently being processed
   std::string    nom_archivo      = "none" ;  // name of the file currently being processed
   unsigned long  num_vertices     = 0;        // number of vertices according to the PLY header
   unsigned long  num_caras        = 0;        // number of faces according to the PLY header

   PLYReader() {}

   void openFile       ( const std::string & p_nombre_archivo ) ;
   void readHeader     ( const bool lee_num_caras ) ;
   void readVertices   ( std::vector<glm::vec3> & vertices  ) ;
   void readFaces      ( std::vector<glm::uvec3> & caras   ) ;
   void readRestOfLine() ;
   void reportError    ( const char *msg_error ) ;
} ;

//**********************************************************************
// Main reading function

void ReadPLY
(
   const std::string &       nombre_archivo_pse, // input: file name
   std::vector<glm::vec3> &  vertices,           // output: vector of vertex coordinates
   std::vector<glm::uvec3> & caras               // output: vector of triangles (indices)
)
{
   using namespace std ;
   PLYReader lector ;

   lector.openFile    ( nombre_archivo_pse ) ;
   lector.readHeader  ( true ) ;
   lector.readVertices( vertices ) ;
   lector.readFaces   ( caras ) ;

   //cout << "PLY file '" << lector.nom_archivo << "' read: number of vertices == " << vertices.size() << ", number of faces == " << caras.size() << endl << flush ;
}

//**********************************************************************

void ReadPLYVertexes
(
   const std::string &  nombre_archivo_pse,
   vector<glm::vec3> &    vertices
)
{
   using namespace std ;
   PLYReader lector  ;

   lector.openFile    ( nombre_archivo_pse ) ;
   lector.readHeader  ( false ) ;
   lector.readVertices( vertices ) ;

   //cout << "PLY file '" << lector.nom_archivo << "' read (vertices only): number of vertices == " << vertices.size() << endl << flush ;
}

//**********************************************************************

void PLYReader::readRestOfLine()
{
   src.getline( buffer, tam_buffer );
   num_linea_actual ++ ;
}

//**********************************************************************

void PLYReader::openFile( const std::string & p_nombre_archivo )
{
   using namespace std ;
   string token ;

   num_vertices     = 0 ;
   num_caras        = 0 ;
   num_linea_actual = 0 ;

   nom_archivo = p_nombre_archivo ;
   if ( nom_archivo.substr( nom_archivo.find_last_of(".")+1 ) != "ply" )
      nom_archivo += ".ply" ;


   // const std::string  
   //    nom_archivo_path_1    = PathCarpetaMateriales() + "/plys/" + nom_archivo ,
   //    nom_archivo_procesado = ProcesarNombreArchivo( nom_archivo_path_1 );

   const std::string nom_archivo_path = SearchFile( "plys/"+nom_archivo, "assets" );

   src.open( nom_archivo_path.c_str() ) ; // open (in read mode?)
   assert( src.is_open());

   src >> token ;

   if ( token != "ply" )
      reportError("input file does not start with 'ply'.");

   readRestOfLine();

   //cout << "reading PLY file '" + nombre_archivo + "'" << endl ;
}

//**********************************************************************

void PLYReader::readHeader( const bool lee_num_caras )
{
   string        token ;
   unsigned      state       = 0; // 0 before reading 'element vertex' (or 'element face'), 1 before reading 'element face', 2 afterward
   bool          en_cabecera = true ;
   long long int nv          = 0,
                 nc          = 0 ;

   // Read header:

   while( en_cabecera )
   {
      if ( src.eof() )
         reportError("premature end of file before end_header");

     src >> token ;

     if ( token == "end_header" )
     {  if ( state != 2 )
           reportError("'element vertex' or 'element face' not found in header");
        readRestOfLine();
        en_cabecera = false ;
     }
     else if ( token == "comment" )
   {  readRestOfLine();
     }
     else if ( token == "format" )
     {  src >> token ;
        if ( token != "ascii" )
      {  string msg = string("the PLY format is not 'ascii'; it is '")+token+"', which cannot be read" ;
           reportError(msg.c_str());
        }
      readRestOfLine();
     }
     else if ( token == "element" )
     {  src >> token ;
        if ( token == "vertex" )
        {  if ( state != 0 )
              reportError("the 'element vertex' line comes after 'element face'");
           src >> nv ;
           //cout << "  number of vertices == " << nv << endl ;
           state = lee_num_caras ? 1 : 2 ;
        }
        else if ( lee_num_caras && token == "face" )
        {  if ( state != 1 )
              reportError("'element vertex' comes after 'element face'");
           src >> nc ;
           //cout << "  number of faces == " << nc << endl ;
           state = 2 ;
        }
        else
      {  //cout << "  element '" + token + "' ignored." << endl ;
        }
      readRestOfLine();
     }
     else if ( token == "property" )
   {  readRestOfLine();
     }
   } // end of while( en_cabecera )

   if ( nv <= 0 )
      reportError("the number of vertices was not found, or it is zero or negative");


   if ( lee_num_caras ) if ( nc <= 0 )
      reportError("the number of faces was not found, or it is zero or negative");

   if ( nv > numeric_limits<int>::max() )
      reportError("the number of vertices exceeds the largest possible 'int' value.");

   if ( lee_num_caras )
   if ( nc > numeric_limits<int>::max() )
      reportError("the number of faces exceeds the largest possible 'int' value.");

   num_vertices = unsigned(nv) ;
   num_caras    = unsigned(nc) ;
}

//**********************************************************************

void PLYReader::readVertices( std::vector<glm::vec3> & vertices  )
{
   using namespace glm ;
   
   string token ;

   vertices.resize( num_vertices );

   for( unsigned long long iv = 0 ; iv < num_vertices ; iv++ )
   {
      if ( src.eof() )
         reportError("premature end of file found in the vertex list.");
      long double x,y,z ;
      src >> x >> y >> z ;
      readRestOfLine();
      vertices[iv] = vec3( float(x), float(y), float(z) );
   }
   //cout << "  end of vertex list" << endl << flush ;
}

//**********************************************************************

void PLYReader::readFaces( std::vector<glm::uvec3> & caras )
{
   using namespace glm ;
   
   string        token ;
   constexpr int nvc = 3 ;

   assert( nvc > 2 ) ; // typically, 3 or 4.

   //cout << "  reading " << num_caras << " faces ...." << endl << flush ;

   caras.resize( num_caras );

   for( unsigned long long ifa = 0 ; ifa < num_caras ; ifa++ )
   {
      if ( src.eof() )
         reportError("premature end of file in the face list");

      unsigned nv ; src >> nv ;
      //cout << "reading face #" << ifa << " with " << nv << " vertexes: " ;

      if ( nv != nvc )
         reportError("a face with a number of vertices other than 3 was found.");

      //long long       iv[nvc] ;
      //const long long base = ifa*nvc ;

      uvec3 cara_leida ; assert( nvc == 3 ); //;-)

      for ( unsigned ivc = 0 ; ivc < nvc ; ivc++ )
      {
         src >> cara_leida[ivc] ;
         if ( num_vertices <= cara_leida[ivc] )
            reportError("a vertex index equal to or greater than the number of vertices was found");
         caras[ ifa ] = cara_leida ;
      }
      readRestOfLine();
   }
   //cout << "  end of face list." << endl ;
}

//**********************************************************************

void PLYReader::reportError( const char *msg_error )
{
   using namespace std ;
      cout << "error reading PLY file '" << msg_error << "' on line " << num_linea_actual << endl
         << "program terminated." << endl
        << flush ;

   exit(1);
}

} // end of namespace ilc
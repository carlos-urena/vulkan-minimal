// *********************************************************************
// **
// ** Rutinas auxiliares:
// ** - Gestión de errores en general
// ** - Comprobación de errores de OpenGL (implementación)
// ** - Inicialización de GLEW,
// ** - Comprobación de la versión de OpenGL
// **
// ** Copyright (C) 2014-2022 Carlos Ureña
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

#include <sys/types.h>  // para buscar carpetas con 'stat'
#include <sys/stat.h>   // para buscar carpetas con 'stat'

#ifndef _WIN32
#include <unistd.h>      // para 'getcwd', stat y otros ...
//#else 
//#include <sys/stat.h> // stat() en msvc
#endif 

#include <iostream>
#include <fstream>
#include <string>

// -----------------------------------------------------------------------------
// Devuelve el path relativo hasta una carpeta con cierto nombre ('carpeta')
// busca la carpeta en el directorio de trabajo, si no está busca en el padre, y 
// así sucesivamente hasta 'n' veces como mucho
// Es decir, busca: ./<carpeta>, ./../<carpeta>, ../../<carpeta>, ../../../<carpeta> y ../../../../<carpeta>
// (funciona tmb en windows)
//
// Basado en:
// https://stackoverflow.com/questions/18100097/portable-way-to-check-if-directory-exists-windows-linux-c
// (respuesta de Adam Parson)

std::string PathCarpeta( const std::string & carpeta, unsigned int n )
{
   using namespace std ;
   string prefijo = "" ; 
   
   for( unsigned i = 0 ; i < n ; i++ )
   {
      const string       path   = prefijo + carpeta ;
      const char * const c_str  = path.c_str();
      struct stat        info ;

      if ( stat( c_str, &info ) == 0 )
         // if ( S_ISDIR( info.st_mode )  ) // no va en windows, S_ISDIR no en 'stat.h' 
         if ( info.st_mode & S_IFDIR ) 
            return path ;
      
      if ( i < n-1 )
         prefijo = prefijo + "../" ;
   }

   cout << "No encuentro el path hasta la carpeta '" << carpeta << "' (aborto)" << endl ;
   exit(1);   
}

// ---------------------------------------------------------------------
// quita el path de un nombre de archivo con path

std::string QuitarPath( const std::string & path_arch )
{
   const size_t pos_barra = path_arch.find_last_of('/') ;

   if ( pos_barra == std::string::npos ) 
      return path_arch ;
   else
      return path_arch.substr( pos_barra+1 );
}

// ---------------------------------------------------------------------------
// busca un archivo dado su nombre (nombre_arch) con extensión, sin path alguno.
//
// - en 1er lugar lo busca en la carpeta de materiales, dentro de la subcarpeta especificada 
// - si no está, lo busca en la carpeta de archivos del alumno
//
// devuelve el nombre del archivo con la ruta completa, listo para ser abierto.
// si el archivo no se encuentra en ninguna de ambas carpetas, se invoca 'error' (aborta)

std::string BuscarArchivo( const std::string & nombre_arch, const std::string & subcarpeta )
{
   using namespace std ;

   // quitar el path y quedarnos simplemente con el basename:

   const std::string basename = QuitarPath( nombre_arch );

   

   // buscar en la carpeta de materiales:

   const string nombre_path_mat = PathCarpeta( subcarpeta, 4 )  + "/" + basename ;
   ifstream     archivo_mat ;

   archivo_mat.open( nombre_path_mat.c_str() ); // intentar abrirlo

   if ( archivo_mat.is_open() ) // si se ha podido abrir, cerrarlo y terminar
   {
      archivo_mat.close();
      return nombre_path_mat ;
   }

   

   // no se ha encontrado: producir mensaje de error y abortar 

   cout << "No se encuentra el archivo: '" << nombre_arch << "'" << endl 
        << "Programa abortado." << endl ; 

   exit(1);
}


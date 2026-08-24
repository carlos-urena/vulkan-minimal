#include <sys/types.h>  // for searching folders with 'stat'
#include <sys/stat.h>   // for searching folders with 'stat'

#ifndef _WIN32
#include <unistd.h>      // for 'getcwd', stat and others ...
//#else 
//#include <sys/stat.h> // stat() in msvc
#endif 

#include <iostream>
#include <fstream>
#include <string>

// -----------------------------------------------------------------------------
// Returns the relative path to a folder with a given name ('folder')
// searches for the folder in the current working directory, then in its parent,
// and so on up to 'n' levels at most.
// In other words, it searches: ./<folder>, ./../<folder>, ../../<folder>, ../../../<folder> and ../../../../<folder>
// (also works on Windows)
//
// Based on:
// https://stackoverflow.com/questions/18100097/portable-way-to-check-if-directory-exists-windows-linux-c
// (answer by Adam Parson)

std::string FolderPath( const std::string & carpeta, unsigned int n )
{
   using namespace std ;
   string prefijo = "" ; 
   
   for( unsigned i = 0 ; i < n ; i++ )
   {
      const string       path   = prefijo + carpeta ;
      const char * const c_str  = path.c_str();
      struct stat        info ;

      if ( stat( c_str, &info ) == 0 )
         // if ( S_ISDIR( info.st_mode )  ) // does not work on Windows, S_ISDIR not in 'stat.h' 
         if ( info.st_mode & S_IFDIR ) 
         {
            cout << "Folder path found for '" << carpeta << "' at level " << i << ": '" << path << "'" << endl ;
            return path ;
         }
      
      if ( i < n-1 )
         prefijo = prefijo + "../" ;
   }

   cout << "Folder path not found for '" << carpeta << "' (aborting)" << endl ;
   exit(1);   
}

// ---------------------------------------------------------------------
// removes the path from a file name that includes a path

std::string RemovePath( const std::string & file_path )
{
   const size_t pos_barra = file_path.find_last_of('/') ;

   if ( pos_barra == std::string::npos ) 
      return file_path ;
   else
      return file_path.substr( pos_barra+1 );
}

// ---------------------------------------------------------------------------
// searches for a file given its name (nombre_arch) with extension, without any path.
//
// - first it searches in the materials folder, inside the specified subfolder
// - if it is not there, it searches in the student's files folder
//
// returns the file name with the full path, ready to be opened.
// if the file is not found in either folder, 'error' is invoked (abort)

std::string SearchFile( const std::string & file_path, const std::string & subfolder )
{
   using namespace std ;

   // remove the path and keep only the basename:

   //const std::string basename = RemovePath( file_path );
   const std::string basename = file_path ; // do not remove path

   // search in the materials folder:

   const string nombre_path_mat = FolderPath( subfolder, 4 )  + "/" + basename ;
   ifstream     archivo_mat ;

   cout << "Trying to open file '" << nombre_path_mat << "' ..." << endl ;

   archivo_mat.open( nombre_path_mat.c_str() ); // try to open it

   if ( archivo_mat.is_open() ) // if it opened successfully, close it and finish
   {
      archivo_mat.close();
      cout << "File found: '" << nombre_path_mat << "'" << endl ;
      return nombre_path_mat ;
   }

   // not found: print an error message and abort

   cout << "File not found: '" << file_path << "'" << endl 
        << "Program aborted." << endl ; 

   exit(1);
}


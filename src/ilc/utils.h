#pragma once

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

std::string FolderPath( const std::string & carpeta, unsigned int n ) ;

// ---------------------------------------------------------------------
// removes the path from a file name that includes a path

std::string RemovePath( const std::string & file_path ) ;

// ---------------------------------------------------------------------------
// searches for a file given its name (nombre_arch) with extension, without any path.
//
// - first it searches in the materials folder, inside the specified subfolder
// - if it is not there, it searches in the student's files folder
//
// returns the file name with the full path, ready to be opened.
// if the file is not found in either folder, 'error' is invoked (abort)

std::string SearchFile( const std::string & file_path, const std::string & subfolder ) ;


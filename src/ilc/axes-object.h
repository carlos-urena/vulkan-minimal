#pragma once


#include <indexed-mesh.h>


// a class for a solid cilinder in 01

class CylinderZ01 : public IndexedMesh
{

    
  public:
    CylinderZ01( const std::string & name, int num_slices ) ;
   ~CylinderZ01() ;
} ;
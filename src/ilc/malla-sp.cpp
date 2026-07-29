

#include <limits>
#include <set>
#include <glm/glm.hpp>

#include <ilc/malla-sp.h>


namespace ilc 
{

// ---------------------------------------------------------------------

MallaSupPar::MallaSupPar( const FuncionParam * p_fp, const unsigned p_ns, const unsigned p_nt,
                         const bool p_promediar_normales_col   )
{
   using namespace glm ;

   assert( 1 < p_nt && 1 < p_ns );
   assert( p_fp != nullptr );

   fp = p_fp ;
   ns = p_ns ;
   nt = p_nt ;

   setName( fp->leerNombre() + " generado como superf. parámetrica (" + std::to_string(ns) + " x " + std::to_string(nt) + ")");
   
   // agregar los vértices y triángulos, por filas
   
   for( unsigned it = 0 ; it < nt ; it++ )  
   for( unsigned is = 0 ; is < ns ; is++ )   
   {
      const vec2 c = vec2( float(is)/float(ns-1), float(it)/float(nt-1) );
      
      const vec3 vcol = vec3( float(it & 1), 1.0f, 1.0f );
      
      vertices.push_back( fp->evaluarPosicion( c ));
      vert_colors.push_back( vcol );
      vert_tcc.push_back( vec2( c.s, 1.0-c.t ));

      if ( is < ns-1 && it < nt-1 )
      {
         const unsigned 
            iv00 = (it+0)*ns + (is+0),
            iv01 = (it+1)*ns + (is+0), 
            iv10 = (it+0)*ns + (is+1),
            iv11 = (it+1)*ns + (is+1);

         triangles.push_back( uvec3( iv00, iv11, iv01 ));
         triangles.push_back( uvec3( iv00, iv10, iv11 ));

         // aquí arriba hay que tener en cuenta que la coordenada T crece de "arriba abajo"
         // y la coordenadas S crece de "izquierda a derecha", así que hay que dar la indices en
         // este orden para que las normales de las caras y vértices esten "hacia fuera" ....
         
      }
   }

   computeNormals();
   if ( p_promediar_normales_col )
      promediarNormalesCol();
}
// ---------------------------------------------------------------------

void MallaSupPar::promediarNormalesCol()
{   
   using namespace glm ;

   for( unsigned it = 0 ; it < nt ; it++ )  
   {
      const unsigned  
         iv0 = it*ns,      // índice del 1er vértice de la fila
         iv1 = iv0+ns-1 ;  // índice del último vértice de la fila.

      const vec3 
         n_promedio = normalize( vert_normals[ iv0 ] + vert_normals[ iv1 ] );

      vert_normals[ iv0 ] = n_promedio ;
      vert_normals[ iv1 ] = n_promedio ;
   }
}
// ---------------------------------------------------------------------
    
MallaSPEsfera::MallaSPEsfera( const unsigned ns, const unsigned nt )

:  MallaSupPar( new FPEsfera(), ns, nt )
{
   promediarNormalesCol();
}
// ---------------------------------------------------------------------


MallaSPCilindro ::MallaSPCilindro( const unsigned ns, const unsigned nt )

:  MallaSupPar( new FPCilindro(), ns, nt )
{      
   promediarNormalesCol();
}
// ---------------------------------------------------------------------

MallaSPCono ::MallaSPCono( const unsigned ns, const unsigned nt )

:  MallaSupPar( new FPCono(), ns, nt )
{      
   promediarNormalesCol();
}
// ---------------------------------------------------------------------

MallaSPColumna::MallaSPColumna( const unsigned ns, const unsigned nt )

:  MallaSupPar( new FPColumna(), ns, nt )
{      
   promediarNormalesCol();
}
// ---------------------------------------------------------------------


} // end of namespace ilc



#include <cmath> 
#include <sup-par.h>

namespace ilc 
{

FuncionParam::FuncionParam( const std::string & nombre_inicial )
{
   fijarNombre( nombre_inicial ) ;
}
// ---------------------------------------------------------------------

void FuncionParam::fijarNombre( const std::string & nuevo_nombre ) 
{
   nombre = nuevo_nombre ;
}
// ---------------------------------------------------------------------
      
const std::string & FuncionParam::leerNombre( ) const 
{
   return nombre ;
}
// ---------------------------------------------------------------------


glm::vec3 FPEsfera::evaluarPosicion( const glm::vec2 & st ) const 
{
   assert( 0.0 <= st.s && st.s <= 1.0 );
   assert( 0.0 <= st.t && st.t <= 1.0 );

   const float
      a  = M_PI * (2.0*st.s),
      b  = M_PI * (st.t-0.5),
      ca = std::cos( a ),
      sa = std::sin( a ),
      cb = std::cos( b ),
      sb = std::sin( b ) ;

   return glm::vec3( sa*cb, sb, ca*cb );
}

// ---------------------------------------------------------------------

glm::vec3 FPCilindro::evaluarPosicion( const glm::vec2 &  st ) const 
{
   assert( 0.0 <= st.s && st.s <= 1.0 );
   assert( 0.0 <= st.t && st.t <= 1.0 );

   const float
      a  = M_PI * (2.0*st.s),
      ca = std::cos( a ),
      sa = std::sin( a ) ;

   return glm::vec3( sa, st.t, ca );
}

// ---------------------------------------------------------------------

glm::vec3 FPCono::evaluarPosicion( const glm::vec2 &  st ) const 
{
   assert( 0.0 <= st.s && st.s <= 1.0 );
   assert( 0.0 <= st.t && st.t <= 1.0 );

   const float
      a  = M_PI * (2.0*st.s),
      ca = std::cos( a ),
      sa = std::sin( a ),
      r  = 1.0-st.t ;

   return glm::vec3( r*sa, st.t, r*ca );
}
// ---------------------------------------------------------------------

glm::vec3 FPColumna::evaluarPosicion( const glm::vec2 & st ) const 
   
{
   assert( 0.0 <= st.s && st.s <= 1.0 );
   assert( 0.0 <= st.t && st.t <= 1.0 );

   const float
      a  = M_PI * (2.0*st.s),
      ca = std::cos( a ),
      sa = std::sin( a ),
      r  = 1.0+0.1*std::sin( 5.0* (a + 2.0*M_PI*st.t) ) ;

   return glm::vec3( r*sa, 10.0*(st.t-0.5), r*ca );
} ;


}
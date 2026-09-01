// *********************************************************************
// **
// ** Course: PCG (Graphics Pipeline Programming).
// ** 
// ** Camera management (implementation)
// ** Copyright (C) 2016-2023 Carlos Ureña
// **
// ** Implementation of class 'Viewport', 'Camara' and classes derived from 'Camara'
// **
// **   + Camara: base class
// **      + CamaraInteractiva: editable cameras
// **          + CamaraOrbitalSimple: orbital camera used in practical sessions
// **            (100% implemented).
// **          + Camara3Modos: camera with three operating modes.
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


#include <cmath>     // std::cos, std::sin
#include <algorithm> // std::min, std::max

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/geometric.hpp> // cross 

#include <ilc/camara.h>

namespace ilc
{

constexpr int X = 0, Y = 1, Z = 2 ; // names for each axe index.

const glm::mat4 flipy_mat = glm::mat4( 1.0,  0.0,  0.0,  0.0,
                                       0.0, -1.0,  0.0,  0.0,
                                       0.0,  0.0,  1.0,  0.0,
                                       0.0,  0.0,  0.0,  1.0 ) ;

// ---------------------------------------------------------------------
// viewport matrix (keeps Z unchanged: between -1 and 1)

glm::mat4 MAT_Viewport( int org_x, int org_y, int ancho, int alto )
{
   using namespace glm ;
   return translate( vec3{ float(org_x), float(org_y), 0.0 } )*
          scale( vec3( float(ancho), float(alto), 1.0 ) )*
          scale( vec3( 0.5, 0.5, 1.0 ))*
          translate( vec3{ 1.0, 1.0, 1.0 }) ;
}
// ---------------------------------------------------------------------
// inverse viewport matrix

glm::mat4 MAT_Viewport_inv( int org_x, int org_y, int ancho, int alto )
{
   using namespace glm ;
   return translate( vec3{ -1.0, -1.0, -1.0 } ) *
          scale( vec3( 2.0, 2.0, 1.0 ))*
          scale( vec3( 1.0/float(ancho), 1.0/float(alto), 1.0 ))*
          translate( vec3{ -float(org_x), -float(org_y), 0.0 } ) ;
}


// ---------------------------------------------------------------------
// view matrix and its inverse

glm::mat4 MAT_Vista( const glm::vec3 eje[3], const glm::vec3 & origen )
{
   using namespace glm ;
   auto rot = mat4(1.0);
   
   for( unsigned i = 0 ; i < 3 ; i++ )
   for( unsigned j = 0 ; j < 3 ; j++ )
      //rot[i][j] = eje[i][j] ;    // CUA: wrong in glm
      rot[i][j] = eje[j][i] ;      // CUA: ok in glm, because rot[i][j] is column 'i', row 'j' of 'rot'

   return rot * translate( -origen ) *flipy_mat ; // flip-Y is needed because in Vulkan Y+ axis in world coordinates points down in NDC space.
}
// ---------------------------------------------------------------------

glm::mat4 MAT_Vista_inv( const glm::vec3 eje[3], const glm::vec3 & origen )
{
   using namespace glm ;
   auto rot_inv = mat4(1.0);
   
   for( unsigned i = 0 ; i < 3 ; i++ )
   for( unsigned j = 0 ; j < 3 ; j++ )
      //rot_inv[i][j] = eje[j][i] ;    // inverse == transpose, but this is wrong in glm
      rot_inv[i][j] = eje[i][j] ;    // inverse == transpose (we do it opposite to MAT_Vista)

   return flipy_mat * translate( origen ) * rot_inv ; 
}

// ----------------------------------------------------------------------------
// converts spherical coordinates (a,b,r) to cartesian (x,y,z)
// ('a' is the rotation angle in radians around the Z axis, in plane Y=0)

glm::vec3 Cartesianas( const glm::vec3 & esfericas )
{
   const float
      sa = std::sin(esfericas[0]), ca = std::cos(esfericas[0]),
      sb = std::sin(esfericas[1]), cb = std::cos(esfericas[1]),
      r  = esfericas[2] ;

   return glm::vec3( r*sa*cb, r*sb, r*ca*cb ) ;
}

// ----------------------------------------------------------------------------
// converts cartesian coordinates (x,y,z) to spherical (a,b,r)

glm::vec3 Esfericas( const glm::vec3 & cartesianas )
{
   const float
      x  = cartesianas[0],
      y  = cartesianas[1],
      z  = cartesianas[2],
      r  = std::sqrt( x*x + y*y + z*z ), // vector length (radius)
      rh = std::sqrt( x*x + z*z );  // length of the horizontal projection

   return glm::vec3 { atan2f(x,z), atan2f(y,rh), r } ;
}
// --------------------------------------------------------------------- 

// *********************************************************************
// class: Viewport, does this serve any purpose?

// ---------------------------------------------------------------------
// creates a 512 x 512 viewport with origin at (0,0)

Viewport::Viewport() 
{
   org_x    = 0 ;
   org_y    = 0 ;
   ancho    = 512 ;
   alto     = 512  ;
   ratio_yx = float(alto)/float(ancho) ;

   matrizVp    = MAT_Viewport( org_x, org_y, ancho, alto );
   matrizVpInv = MAT_Viewport_inv( org_x, org_y, ancho, alto );
}

// ---------------------------------------------------------------------

Viewport::Viewport( int p_org_x, int p_org_y, int p_ancho, int p_alto )
{
   org_x    = p_org_x ;
   org_y    = p_org_y ;
   ancho    = p_ancho ;
   alto     = p_alto ;
   ratio_yx = float(alto)/float(ancho) ;

   matrizVp    = MAT_Viewport( org_x, org_y, ancho, alto );
   matrizVpInv = MAT_Viewport_inv( org_x, org_y, ancho, alto );
}

// *********************************************************************
// class Camara
// base class for cameras

// -------------------------------------------------------------------------------
// set model-view and projection matrices in the pipeline

// void Camara::activar( Cauce3D & cauce )
// {
//    using namespace std ;
//    //cout << endl ;
//    //cout << __FUNCTION__ << ": start " << descripcion() << "'" << endl ;
//    actualizarMatrices();
//    cauce.fijarMatrizVista( matriz_vista );
//    cauce.fijarMatrizProyeccion( matriz_proye );
//    //cout << __FUNCTION__ << ": end " << descripcion() << "'" << endl ;
// }

// -------------------------------------------------------------------------------
// change the value of 'ratio_vp' (viewport height/width)

void Camara::fijarRatioViewport( const float nuevo_ratio )
{
   ratio_vp = nuevo_ratio ;
   matrices_actualizadas = false ;
}

// -----------------------------------------------------------------------------
// update 'matriz_vista' and 'matriz_proye' from the ratio

void Camara::actualizarMatrices()
{
   using namespace glm ;

   if ( matrices_actualizadas )
      return ;

   matriz_vista = mat4(1.0);
   matriz_proye = scale( vec3{ 1.0f,1.0f/ratio_vp, 1.0f }) ;// MAT_Scaling( 1.0f, 1.0f/ratio_vp, 1.0f );
   matrices_actualizadas = true ;
}
// -----------------------------------------------------------------------------
// get the camera description (and probably its state)

std::string Camara::descripcion()
{
   return "camera (base class)" ;
}

// ################################################################################
// class CamaraInteractiva
// base (abstract) class for interactive cameras (manipulable cameras)


// ----------------------------------------------------------------------------
// make it look toward attention point 'paten' and switch to examine mode
// by default prints a warning that the camera does not provide this functionality

void CamaraInteractiva::mirarHacia( const glm::vec3 & paten )
{
   using namespace std ;
   cout << "this camera cannot point to any object." << endl ;
}

// ----------------------------------------------------------------------------
// change the camera mode to the next one or the first one
// by default prints a warning that it does not provide this functionality

void CamaraInteractiva::siguienteModo()
{
   using namespace std ;
   cout << "this camera does not define multiple operating modes." << endl ;
}

// ******************************************************************
// class CamaraOrbitalSimple
//

// class for a simple orbital camera that always keeps the world
// coordinate origin at the center of the image (single mode)

// ----------------------------------------------------------------------------
// changes longitude using 'dx' and latitude using 'dy'

void CamaraOrbitalSimple::desplRotarXY( const float dh, const float dv )
{
   horz_angle_deg += dh ;
   vert_angle_deg += dv ;
   matrices_actualizadas = false ;
}

// ----------------------------------------------------------------------------
// zoom by 'dz' units relative to the origin (without ever crossing it)

void CamaraOrbitalSimple::moverZ( const float dz )
{
   constexpr float
      d_min = 0.2 ,  // minimum distance to the origin
      rc    = 0.04 ; // growth ratio when 'dz=1'

   d = d_min + (d-d_min)*std::pow((1.0f+rc),dz) ;
   matrices_actualizadas = false ;
}

// ----------------------------------------------------------------------------

void CamaraOrbitalSimple::actualizarMatrices()
{
   using namespace std ;
   //cout << "CamaraOrbitalSimple::actualizarMatrices() : start: a == " << a << ", b == " << b << ", d == " << d << endl ;
   using namespace glm ;
   matriz_vista = translate( vec3( 0.0, 0.0, -d) ) *          // MAT_Traslacion( { 0.0, 0.0, -d } ) *
                  rotate( radians(vert_angle_deg),  vec3( 1.0,0.0,0.0 )) * // MAT_Rotation( b,  { 1.0,0.0,0.0} ) *
                  rotate( radians(-horz_angle_deg), vec3( 0.0,1.0,0.0 )) * // MAT_Rotation( -a, { 0.0,1.0,0.0}  ) ;
                  flipy_mat ; // flip-Y is needed because in Vulkan Y+ axis in world coordinates points down in NDC space.

   constexpr float
      fovy_grad = 70.0,
      near      = 0.05,
      far       = near+1000.0 ;

   //MAT_Perspective( fovy_grad, ratio_vp, near, far ); // CUA: ratio_vp is y/x, but this function expects 'aspect', which seems to be x/y
    
   matriz_proye = perspective( radians(fovy_grad), 1.0f/ratio_vp, near, far ); // CUA
   //matriz_proye = scale( vec3{ 1.0f,1.0f/ratio_vp, 1.0f })  ;
   
   matrices_actualizadas = true ;
}
// -----------------------------------------------------------------------------
// get the camera description (and probably its state)

std::string CamaraOrbitalSimple::descripcion()
{
   using namespace std ;
   return string("simple orbital camera, angles: a = ") + to_string(horz_angle_deg) + ", b = " + to_string(vert_angle_deg) ;
}

// ******************************************************************
// class Camara3Modos
// full interactive camera, can work in three modes,
// and can focus on a point (switches to examine mode)
//
// Modes are:
//
//   ** examine mode (or orbital mode), relative to an attention point
//   ** first-person mode with rotations (typical in videogames)
//   ** first-person mode with displacements (horizontal or vertical)

static const std::string nombre_modo[3] =
    { "examine (or orbital mode)",
       "first-person with rotations",
       "first-person with displacements"
   };



// ----------------------------------------------------------------------------
// checks that conversions between cartesian and spherical are correct

void TestCartesianasPolares()
{
   using namespace glm ;

   float max = 0.0 ;
   for( unsigned long i = 0 ; i < 1024*1024 ; i++ )
   {
      const float
         x = float(rand())/float(RAND_MAX),
         y = float(rand())/float(RAND_MAX),
         z = float(rand())/float(RAND_MAX) ;
      const glm::vec3
         cart  = { 2.0f*x-1.0f, 2.0f*y-1.0f, 2.0f*z-1.0f },
         pol   = Esfericas( cart ),
         cart2 = Cartesianas( pol );
      const float
         lsq = glm::length2(cart2-cart); 
      if ( lsq > max )
         max = lsq ;
   }

   using namespace std ;
   cout << "Test: cart2 - cart, max diff sq == " << max << endl ;
}

// ----------------------------------------------------------------------------
// creates a perspective camera with default values

Camara3Modos::Camara3Modos()
{
   // All parameters take default values, see class declaration)
   // CUA: I add this (29 Sept): it is in the other constructor, but not in this one:
   actualizarEjesMCV();
   matrices_actualizadas = false ;
}

// ----------------------------------------------------------------------------
// creates a camera, initially in examine mode, with the attention
// point at the origin, specified by:
//
// * perspectiva_ini : true if it is a perspective camera, false if orthographic
// * ratio_vp_ini    : initial viewport ratio (y/x)
// * origen_ini      : initial viewpoint (camera frame origin)
// * punto_aten_ini  : attention point
// * fovy_grad_ini   : if perspective, vertical field-of-view aperture in degrees

Camara3Modos::Camara3Modos( const bool perspectiva_ini,
                            const glm::vec3 & origen_ini, const float ratio_vp_ini,
                            const glm::vec3 & punto_aten_ini, const float fovy_grad_ini )
{
   //using namespace std ;
   //cout << "Camara3Modos creation, origin == " << origen << endl ;
   using namespace glm ;

   assert( 5.0 < fovy_grad_ini && fovy_grad_ini < 178.0 );
   assert( 0.0 <= ratio_vp_ini );
   const float d_sq = length2(punto_aten_ini-origen_ini) ; assert( 0.0 < d_sq );

   // initialize parameters with values different from defaults:
   perspectiva     = perspectiva_ini ;
   punto_atencion  = punto_aten_ini  ;
   ratio_vp        = ratio_vp_ini ;
   org_cartesianas = origen_ini - punto_aten_ini ; assert( 0.01 <= length(org_cartesianas) );
   org_polares     = Esfericas( org_cartesianas );
   fovy_grad       = fovy_grad_ini ;

   actualizarEjesMCV();

   matrices_actualizadas = false ;
}

// ----------------------------------------------------------------------------
// move or rotate the camera by 'dx' units horizontally and 'dy' units vertically
// (horizontal and vertical here are relative to the camera frame)

void Camara3Modos::desplRotarXY( const float dh, const float dv )
{
   switch( modo_actual )
   {
      case ModoCam::examinar :
      {
         // Rotate camera in examine mode
         //
         // update the first two components (angles) of polar coordinates
         // update cartesian coordinates from polar coordinates
         constexpr float pi2 = M_PI/2.0f - 0.01 ;
         float a = org_polares[0], b = org_polares[1], r = org_polares[2];

         a = a + dh*0.02f ; // note: 'a' is in radians
         b = std::min( std::max( b+dv*0.02f, -pi2), +pi2 );

         org_polares     = glm::vec3 { a, b, r } ;
         org_cartesianas = Cartesianas( org_polares );
         
         actualizarEjesMCV();
         break ;
      }
      case ModoCam::prim_pers_rot :
      {
         // Rotate camera in first-person mode with rotations
         //
         // 1. update the first two components (angles) of polar coordinates (same as in examine mode)
         // 2. compute the new cartesian coordinates and the displacement vector from new to old
         // 3. subtract that displacement vector from the attention point
         // 4. update cartesian coordinates
         // 5. update view-frame axes (actualizarEjesMCV)
         // .....
         
         constexpr float pi2 = M_PI/2.0f - 0.01 ;
         float a = org_polares[0], b = org_polares[1], r = org_polares[2];

         // same as 'examine' mode
         a = a + dh*0.02f ; // note: 'a' is in radians
         b = std::min( std::max( b+dv*0.02f, -pi2), +pi2 );

         org_polares = glm::vec3( a,b,r ) ;

         const glm::vec3 nue_org_cartesianas = Cartesianas( org_polares ),
                         despl_cartesianas   = nue_org_cartesianas - org_cartesianas ;

         punto_atencion  = punto_atencion - despl_cartesianas ;
         org_cartesianas = nue_org_cartesianas ;
         
         actualizarEjesMCV() ;
         break ;
      }
      case ModoCam::prim_pers_despl :
      {
         // Move camera in first-person mode with displacements
         //
         //   move attention point by 'da' units along camera X axis, and
         //   by 'db' units along camera Y axis.
         // (note: axes do not change)
         
         punto_atencion = punto_atencion + dh*0.02f*eje[0] + dv*0.02f*eje[1] ;
         break ;
      }
   }
   matrices_actualizadas = false ;
}
// ----------------------------------------------------------------------------
// zoom in/out the camera relative to the attention point
// (the axis is Z, relative to the camera frame, i.e., perpendicular to the view plane)

void Camara3Modos::moverZ( const float dz )
{

   switch( modo_actual )
   {
      case ModoCam::examinar :
      {
         // Move camera in Z in 'examine' mode
         //
         // 1. update polar-coordinate radius
         // 2. update cartesian coordinates from polar coordinates
         // note: axes and attention point do not change
         constexpr float
            d_min = 0.2 ,  // minimum distance to the origin
            rc    = 0.04 ; // growth ratio when 'dz=1'

         org_polares     = glm::vec3( org_polares[0], org_polares[1],
                                    d_min + (org_polares[2]-d_min)*std::pow( 1.0f+rc, dz ) );
         org_cartesianas = Cartesianas( org_polares );
         break ;
      }
      case ModoCam::prim_pers_rot :
      case ModoCam::prim_pers_despl :
      {
         // Move camera in Z in 'first-person' mode
         //
         // move attention point by 'dz' units along Z axis
         // note: axes do not change
         punto_atencion = punto_atencion + dz*eje[2] ;
         break ;
      }
   }
   matrices_actualizadas = false ;
}
// ----------------------------------------------------------------------------
// make it look toward attention point 'nuevo_punto_aten' and switch to examine mode

void Camara3Modos::mirarHacia( const glm::vec3 & nuevo_punto_aten )
{
   // Look toward a point and switch to examine mode
   //
   // 1. Update 'punto_atencion', moving it to the new attention point
   // 2. Update cartesian coordinates (shift them)
   // 3. Update polar coordinates from cartesian coordinates
   // 4. Set current mode to examine mode
   
   const glm::vec3 despl_pa = punto_atencion - nuevo_punto_aten ;

   punto_atencion  = nuevo_punto_aten ;
   org_cartesianas = org_cartesianas + despl_pa ;
   org_polares     = Esfericas( org_cartesianas );
   modo_actual     = ModoCam::examinar ;
   
   // update view coordinate frame axes
   actualizarEjesMCV();

   // mark matrices as 'not updated'
   matrices_actualizadas = false ;
}
// ----------------------------------------------------------------------------
// change the camera mode to the next one or the first one

void Camara3Modos::siguienteModo()
{
   modo_actual = ModoCam( (int(modo_actual)+1) % 3) ;
   using namespace std ;
   cout << "Camera mode changed to: " << nombre_modo[int(modo_actual)] << "." << endl ;
   matrices_actualizadas = false ;
}
// ----------------------------------------------------------------------------
// Update 'eje' array from 'org_cartesianas'

void Camara3Modos::actualizarEjesMCV()
{
   using namespace glm ;
   eje[Z] = normalize( org_cartesianas );
   eje[X] = normalize( cross( vec3( 0.0, 1.0, 0.0 ), eje[Z] ) );
   eje[Y] = normalize( cross( eje[Z], eje[X] ) );
}
// ----------------------------------------------------------------------------

void Camara3Modos::actualizarMatrices()
{
   using namespace glm ;
   const vec3      org  = punto_atencion + org_cartesianas ;
   constexpr float near = 0.05,
                   far  = near+1000.0 ;

   matriz_vista = MAT_Vista( eje, org );
   if ( perspectiva )
      matriz_proye = perspective( radians(fovy_grad), 1.0f/ratio_vp, near, far );  // CUA: ratio_vp is y/x, but this function expects 'aspect', which seems to be x/y
   else
   {
      constexpr float s = 8.0 ;
      matriz_proye = ortho( -s/2.0f, +s/2.0f, -s*ratio_vp/2.0f, +s*ratio_vp/2.0f, -20.0f, 100.0f ); // MAT_Orthographic( -s/2.0f, +s/2.0f, -s*ratio_vp/2.0f, +s*ratio_vp/2.0f, -20.0f, 100.0f );
   }
   matrices_actualizadas = true ;
}
// ----------------------------------------------------------------------------

glm::vec3 Camara3Modos::puntoAtencion()
{
   return punto_atencion ;
}
// -----------------------------------------------------------------------------
// get the camera description (and probably its state)

std::string Camara3Modos::descripcion()
{
   return std::string("3-mode camera, ") +
          (perspectiva ? "perspective" : "orthographic") +
          ", current mode: " + nombre_modo[int(modo_actual)] + ")";
}

} ; // end ilc namespace 


// declaration of class 'Camera' 
//
// Encapsulates all state data for a single window Vulkan App.

#pragma once

#include <vkhc/vulkan-context.h>

namespace ilc 
{

class Viewport
{
  public:
   int       org_x, org_y, // origin in pixels (lower-left corner)
                  ancho, alto ; // dimensions in pixels (num. columns, num. rows)
   float     ratio_yx ;    // == height/width (aspect ratio)
   glm::mat4  matrizVp ,    // viewport matrix ( maps: NDC ==> DC )
                  matrizVpInv ; // inverse matrix ( maps: DC ==> NDC )
  // constructor
   Viewport() ; // creates a 512 x 512 viewport with origin at (0,0)
  Viewport( int p_org_x, int p_org_y, int p_ancho, int p_alto );
} ;

// ******************************************************************
// base class for cameras

class Camara
{
   public: // ----------------------------

   // change the value of 'ratio_vp' (viewport height/width)
   void fijarRatioViewport( const float nuevo_ratio ) ;

   // get the camera description (and probably its state)
   virtual std::string descripcion() ;

   inline glm::mat4 & viewMatrix() { actualizarMatrices() ; return matriz_vista ; };
   inline glm::mat4 & projectionMatrix() { actualizarMatrices() ; return matriz_proye ; };

   

   protected: // ------------------------------

   bool      matrices_actualizadas = false ;        // true if matrices are updated
   glm::mat4 matriz_vista = glm::mat4(1.0) , // view matrix
             matriz_proye  = glm::mat4(1) ; // projection matrix
   float     ratio_vp      = 1.0 ;         // viewport height divided by viewport width

   // update 'matriz_vista' and 'matriz_proye' from the parameters
   // specific to each camera type
   virtual void actualizarMatrices() ;

   
} ;

// ******************************************************************
// (abstract) class for interactive cameras (manipulable cameras)

class CamaraInteractiva : public Camara
{
   public:
   // move or rotate the camera by 'da' units horizontally and 'db' units vertically
   // (horizontal and vertical here are relative to the camera frame)
   virtual void desplRotarXY( const float dh, const float dv ) = 0 ;

   // zoom in/out or move the camera along the Z axis by a total of 'dz' units
   // (the axis is Z, relative to the camera frame, i.e., perpendicular to the view plane)
   virtual void moverZ( const float dz ) = 0 ;

   // make it look toward the attention point 'paten' and
   // switch to examine mode
   // by default prints a warning that the camera does not provide this functionality
   virtual void mirarHacia( const glm::vec3 & paten ) ;

   // change the camera mode to the next one or the first one
   // by default prints a warning that the camera does not provide this functionality
   virtual void siguienteModo()  ;

   virtual std::string descripcion() override 
   {
      return "interactive camera" ;
   }
} ;

// ******************************************************************
// class for a simple orbital camera that always keeps the world
// coordinate origin at the center of the image (single mode)

class CamaraOrbitalSimple : public CamaraInteractiva
{
   public:

   // changes longitude using 'da' and latitude using 'db'
   virtual void desplRotarXY( const float dh, const float dv )   override ;

   // zoom in/out by 'dz' units toward the origin (without ever crossing it)
   virtual void moverZ( const float dz )  override ;

   // get the camera description (and probably its state)
   virtual std::string descripcion() override ;

   protected:
      // camera position in spherical coordinates relative to the world
      // coordinate origin (and its axes)
      float horz_angle_deg = 45.0,   // horizontal angle (angle of Z axis with plane z==0 in world coords) in degrees
            vert_angle_deg = -20.0,   // vertical angle (angle of Z axis with plane y==0 in world coords) in degrees
            d = 3.0 ;  // distance between camera origin and world coordinate origin

   virtual void actualizarMatrices() override ;
} ;

// enum type for interactive camera modes
enum class ModoCam { examinar, prim_pers_rot, prim_pers_despl } ;

// ******************************************************************
// full interactive camera, can work in three modes,
// and can focus on a point (switches to examine mode)
//
// Modes are:
//
//   ** examine mode (or orbital mode), relative to an attention point
//   ** first-person mode with rotations (typical in videogames)
//   ** first-person mode with displacements (horizontal or vertical)



class Camara3Modos : public CamaraInteractiva
{
   public:

   // creates a perspective camera with default values
   Camara3Modos() ;

   // creates a camera, initially in examine mode, with the attention
   // point at the origin, specified by:
   // * perspectiva_ini : true if it is a perspective camera, false if orthographic
   // * origen_ini      : initial viewpoint (camera frame origin)
   // * punto_aten_ini  : attention point
   // * fovy_grad_ini   : if perspective, vertical field-of-view aperture in degrees

   Camara3Modos( const bool perspectiva_ini,
                 const glm::vec3 & origen_ini, const float ratio_vp_ini,
                 const glm::vec3 & punto_aten_ini, const float fovy_grad_ini = 70.0 ) ;

   // move or rotate the camera by 'dx' units horizontally and 'dy' units vertically
   // (horizontal and vertical here are relative to the camera frame)
   virtual void desplRotarXY( const float dh, const float dv )  override ;

   // zoom in/out the camera relative to the attention point
   // (the axis is Z, relative to the camera frame, i.e., perpendicular to the view plane)
   virtual void moverZ( const float dz )  override ;

   // make it look toward the attention point 'paten' and switch to examine mode
   virtual void mirarHacia( const glm::vec3 & nuevo_punto_aten )  override ;

   // change the camera mode to the next one or the first one
   virtual void siguienteModo()  override ;

   // returns the current attention point
   virtual glm::vec3 puntoAtencion()  ;

   // get the camera description (and probably its state)
   virtual std::string descripcion() override ;

private:

   // current camera mode,
   // (0=examine, 1=first-person with displacements, 2=first-person with rotations)
   ModoCam modo_actual = ModoCam::examinar ;

   // true if the camera is perspective, false otherwise:
   bool perspectiva = true ;

   // vertical field-of-view aperture in degrees
   float fovy_grad = 60.0 ;

   // initial distance between frame origin and attention point
   const float d_ini = 3.0 ;

   // attention point
   glm::vec3 punto_atencion = { 0.0, 0.0, 0.0 } ;

   // polar coordinates of the view-frame coordinate origin, relative to the attention point
   // components 0 and 1 are longitude and latitude angles, both IN RADIANS, initially 0
   // component 2 is the distance to the origin, initially 'd_ini'
   glm::vec3 org_polares = { 0.0, 0.0, d_ini } ;

   // cartesian coordinates of the view-frame coordinate origin, relative to the attention point
   // ( == view-frame origin minus attention point)
   glm::vec3 org_cartesianas = { 0.0, 0.0, d_ini };

   // axes of the view coordinate frame
   glm::vec3 eje[3] = {  { 1.0, 0.0, 0.0 }, // X axis
                { 0.0, 1.0, 0.0 }, // Y axis
                { 0.0, 0.0, 1.0 }  // Z axis
                         };

   virtual void actualizarMatrices() override ;
   void actualizarEjesMCV() ;

} ;

} // end ilc namespace
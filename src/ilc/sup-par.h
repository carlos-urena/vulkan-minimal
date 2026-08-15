

#pragma once

#include <string>    
#include <glm/glm.hpp>


namespace ilc 
{


/// @brief Base class for surface parametrization functions
/// @brief derived classes must implement the 'evaluarPosicion' method
/// @brief which returns the surface position at a given point in the [0..1]^2 domain
///
class FuncionParam
{
   private:
      std::string nombre = "no asignado" ;
   
   public: 

      /// @brief Constructor
      /// @param nombre_inicial initial name for this function
      ///
      FuncionParam( const std::string & nombre_inicial );

      /// @brief Sets the name
      /// @param nuevo_nombre new name for this function
      ///
      void fijarNombre( const std::string & nuevo_nombre ) ;
      
      /// @brief Returns the function name
      /// @return current function name
      ///
      const std::string & leerNombre( ) const ;

      /// @brief Returns the surface position at a given point in the [0..1]^2 domain
      /// @param st point coordinates in the [0..1]^2 domain
      /// @return surface position at the given point
      ///
      virtual glm::vec3 evaluarPosicion( const glm::vec2 & st ) const = 0 ;
} ;
// -------------------------------------------------------------------------

   /// @brief Parametrization function for a sphere
///
class FPEsfera : public FuncionParam 
{
   public:

   FPEsfera() : FuncionParam( "Sphere" ) {}

   /// @brief Computes a point on the surface of a sphere
   /// @param st parametric coordinates of the point
   /// @return point on the surface
   ///
   virtual glm::vec3 evaluarPosicion( const glm::vec2 & st  ) const override ;
} ;
// -------------------------------------------------------------------------

/// @brief Parametrization function for a cylinder
///
class FPCilindro : public FuncionParam 
{
   public: 

   FPCilindro() : FuncionParam( "Cylinder" ) {}

   /// @brief Computes a point on the surface of a cylinder
   /// @param st parametric coordinates of the point
   /// @return point on the surface
   ///
   virtual glm::vec3 evaluarPosicion( const glm::vec2 & st  ) const override ;
} ;
// -------------------------------------------------------------------------

/// @brief Parametrization function for a cone
///
class FPCono : public FuncionParam 
{
   public:

   FPCono() : FuncionParam( "Cone" ) {}

   /// @brief Computes a point on the surface of a cone
   /// @param st parametric coordinates of the point
   /// @return point on the surface
   ///
   virtual glm::vec3 evaluarPosicion( const glm::vec2 & st  ) const override ;
} ;
// -------------------------------------------------------------------------

/// @brief Parametrization function for a baroque column
///
class FPColumna : public FuncionParam 
{
   public:

   FPColumna() : FuncionParam( "Column" ) {}

   /// @brief Computes a point on the surface of a baroque column
   /// @param st parametric coordinates of the point
   /// @return point on the surface
   ///
   virtual glm::vec3 evaluarPosicion( const glm::vec2 & st ) const override ;
} ;


} // end of namespace ilc

#pragma once

#include <vector>       // usar std::vector

#include <sup-par.h>     // declaración de 'FuncionParam'
#include <indexed-mesh.h>   // declaración de 'ObjetoVisu'

namespace ilc 
{

// -----------------------------------------------------------------------
///
/// @brief clase para mallas indexadas generadas a partir de una superficie paramétrica
///
class MallaSupPar : public IndexedMesh 
{
   private: 
      const FuncionParam * fp = nullptr ;  
      unsigned ns = 0 ;
      unsigned nt = 0 ;


   public:

   /// @brief crea una malla indexada a partir de una función de parametrización
   /// @param p_fp - puntero (no nulo) a la función de parametrización) 
   /// @param p_ns - número de muestras en la dirección 's' (primer parámetro de la función de parametrización)
   /// @param p_nt - número de muestras en la dirección 't' (segundo parámetro de la función de parametrización)
   ///
   MallaSupPar( const FuncionParam * p_fp, const unsigned p_ns, const unsigned p_nt, 
                const bool p_promediar_normales_col = false );

   protected:

   /// @brief promedia las normales de la primera y la última columna de vértices
   ///
   void promediarNormalesCol();
   
    
};

/// @brief Malla indexada generada con la parametrización de una esfera 
///
class MallaSPEsfera : public MallaSupPar 
{
   public:
   MallaSPEsfera( const unsigned ns, const unsigned nt );
};

/// @brief Malla indexada generada con la parametrización de un cilindro 
///
class MallaSPCilindro : public MallaSupPar 
{
   public:
   MallaSPCilindro( const unsigned ns, const unsigned nt );
};

/// @brief Malla indexada generada con la parametrización de un cono 
///
class MallaSPCono : public MallaSupPar 
{
   public:
   MallaSPCono( const unsigned ns, const unsigned nt );
};

/// @brief Malla indexada generada con la parametrización de una columna barroca 
///
class MallaSPColumna : public MallaSupPar
{
   public:
   MallaSPColumna( const unsigned ns, const unsigned nt );
};


} // end of namespace ilc
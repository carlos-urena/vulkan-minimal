#include <axes-object.h>
#include <pipeline3D.h>

// ---------------------------------------------------------------------------
// crea una tabla de índices para una rejilla con topología cilindrica

void CreateCilindricalTopologyTriangles( std::vector<glm::uvec3> & indices, 
                                            const unsigned na, const unsigned nb )
{
   indices.clear();

   for( unsigned i = 0 ; i < na ; i++ )
   for( unsigned j = 0 ; j < nb ; j++ )
   {        
      const unsigned int
         i00 = (i  )*nb + j,
         i10 = (i+1)*nb + j,
         i01 = (i  )*nb + ((j+1) % nb),
         i11 = (i+1)*nb + ((j+1) % nb);
   
      indices.push_back({ i00, i01, i11 });
      indices.push_back({ i00, i11, i10 });
   }
}
// ---------------------------------------------------------------------------

void CreateGridTopologyTriangles( std::vector<glm::uvec3> & indices, 
                                            const unsigned na, const unsigned nb )
{
   indices.clear();

   for( unsigned i = 0 ; i < na-1 ; i++ )
   for( unsigned j = 0 ; j < nb-1 ; j++ )
   {        
      const unsigned int
         i00 = (i  )*nb + j,
         i10 = (i+1)*nb + j,
         i01 = (i  )*nb + (j+1),
         i11 = (i+1)*nb + (j+1);
   
      indices.push_back({ i00, i01, i11 });
      indices.push_back({ i00, i11, i10 });
   }
}

// ---------------------------------------------------------------------------------------
// Indexed mesh with a single triangle 

// class IMTriangle : public IndexedMesh
// {
//    public: IMTriangle( const std::vector<glm::vec3> & vertices, const glm::vec3 & color) ;   
// } ;



// ---------------------------------------------------------------------------------------

// IMTriangle::IMTriangle( const std::vector<glm::vec3> & p_vertices, const glm::vec3 & p_color ) 
// {
//    Assert( p_vertices.size() == 3, "IMTriangle: must have exactly 3 vertices" ) ;

//    using namespace glm ;
//    vec3 tri_normal = normalize( cross( p_vertices[1]-p_vertices[0], p_vertices[2]-p_vertices[0] ) ) ;
   
//    for( unsigned iv = 0 ; iv < 3 ; iv++ )
//    {
//       vertices.push_back( p_vertices[iv] ) ;
//       vert_colors.push_back( p_color ) ;
//       vert_normals.push_back( tri_normal ) ;
//       vert_tcc.push_back( { 0.0f, 0.0f } ) ; // texture coordinates not used
//    }
//    triangles.push_back( { 0, 1, 2 } ) ;
// }

// ---------------------------------------------------------------------------------------
// Indexed mesh with a sphere shape

class IMSphere : public IndexedMesh
{
   public: IMSphere(  ) ;
} ;

IMSphere::IMSphere(  ) 
{
   using namespace glm ;
   constexpr unsigned int na = 16, nb = 32 ;  // number of rows and columns of vertexes 
   
   CreateCilindricalTopologyTriangles( triangles, na, nb );  

   for( unsigned iz = 0 ; iz < na  ; iz++ )
   for( unsigned is = 0 ; is < nb ; is++ )
   {   
      const float fz = float(iz)/float(na-1), 
                  fs = float(is)/float(nb-1), 
                  a  = 2.0f*M_PI*fs, 
                  b  = M_PI*(fz-0.5f),
                  ca = cos(a), 
                  sa = sin(a),
                  cb = cos(b),
                  sb = sin(b);

      vec3 vert = { ca*cb, sb, sa*cb } ;
      vertices.push_back( vert );   
      vert_colors.push_back( { 0.1f, 0.8f, 0.8f } );
      vert_tcc.push_back( { fs, fz } );
      vert_normals.push_back( vert );
   }
}
// ---------------------------------------------------------------------------------------

class CylinderZ01 : public IndexedMesh
{
  public:
    CylinderZ01( const std::string & name, const unsigned num_slices ) ;
} ;

// ---------------------------------------------------------------------------------------

CylinderZ01::CylinderZ01( const std::string & name, const unsigned num_slices )
   
:   IndexedMesh( )
{
    using namespace std ;
    constexpr unsigned int na = 2 ;
    const     unsigned     nb = num_slices ;  // number of rows and columns of vertexes 
    
    setName( name ) ;
    CreateGridTopologyTriangles( triangles, na, nb );  

    for( unsigned iz = 0 ; iz < na  ; iz++ )
    for( unsigned is = 0 ; is < nb ; is++ )
    {   
        const float fz = float(iz)/float(na-1), 
                    fs = float(is)/float(nb-1), 
                    a  = 2.0f*M_PI*fs, 
                    ca = cos(a), 
                    sa = sin(a) ;

        vertices.push_back( { ca, sa, fz } );   
        //vertices.push_back( { fs, +0.5f, fz } );
        vert_colors.push_back( { fs, fz, 0.0f } );
        vert_tcc.push_back( { fs, fz } );
        vert_normals.push_back( { ca, sa, 0.0f } );
    }
}
// ---------------------------------------------------------------------------------------

class ConeZ01 : public IndexedMesh
{
  public:
    ConeZ01( const std::string & name, const unsigned num_slices ) ;
} ;

// ---------------------------------------------------------------------------------------

ConeZ01::ConeZ01( const std::string & name, const unsigned num_slices )
   
:   IndexedMesh( )
{
    using namespace std ;
    constexpr unsigned int na = 2 ;
    const     unsigned     nb = num_slices ;  // number of rows and columns of vertexes 
    
    setName( name ) ;
    CreateGridTopologyTriangles( triangles, na, nb );  

    for( unsigned iz = 0 ; iz < na  ; iz++ )
    for( unsigned is = 0 ; is < nb ; is++ )
    {   
        const float fz = float(iz)/float(na-1), 
                    fs = float(is)/float(nb-1), 
                    a  = 2.0f*M_PI*fs, 
                    ca = cos(a), 
                    sa = sin(a) ;

        vertices.push_back( { fz*ca, fz*sa, fz } );   
        //vertices.push_back( { fs, +0.5f, fz } );
        vert_colors.push_back( { fs, fz, 0.0f } );
        vert_tcc.push_back( { fs, fz } );
        vert_normals.push_back( { ca, sa, 0.0f } );
    }
}

// --------------

class Segment : public DrawableObject
{
   private:
      vkhc::VertexArray * vertex_array = nullptr ;
      glm::vec3 p0, p1, color ;
   public:
      Segment( const glm::vec3 & p_p0, const glm::vec3 & p_p1, const glm::vec3 & p_color ) ;
      virtual void drawVK( vkhc::BasicPipeline * pipeline, vkhc::VulkanContext & context, VkCommandBuffer & cmdb_vk ) override ;
} ;


Segment::Segment( const glm::vec3 & p_p0, const glm::vec3 & p_p1, const glm::vec3 & p_color ) 
{
   using namespace glm ;
   using namespace vkhc ;

   setName( "segment" ) ;
   p0 = p_p0 ;
   p1 = p_p1 ;
   color = p_color ;
}
// -----------------------------------------------------------------------------------------

void Segment::drawVK( vkhc::BasicPipeline * pipeline, vkhc::VulkanContext & context, VkCommandBuffer & cmdb_vk )
{
   if ( vertex_array == nullptr ) 
   {
      vertex_array = new vkhc::VertexArray( context, VK_PRIMITIVE_TOPOLOGY_LINE_LIST );
      Assert( vertex_array != nullptr, "cannot create a VAO" );

      std::vector<glm::vec3> vertices = { p0, p1 } ;
      std::vector<glm::vec3> colors = { color, color } ;
      std::vector<glm::vec2> tcc = { glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 1.0f) } ;
      std::vector<glm::vec3> normals = { glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3( 0.0f, 1.0f, 0.0f )} ;

      vertex_array->addAttribData( vertices ) ;
      vertex_array->addAttribData( colors ) ;
      vertex_array->addAttribData( tcc ) ;
      vertex_array->addAttribData( normals ) ;
   }
}
// ---------------------------------------------------------------------------------------

AxesObject::AxesObject( ) 
{
   using namespace glm ; 
   using namespace vkhc ;

   setName( "axes object" ) ;
   axes_cylinder = new CylinderZ01( "axes cylinder Z 01", 32 ) ; Assert( axes_cylinder != nullptr, "Cannot create axes cylinder" ) ;
   axes_cone = new ConeZ01( "axes cone Z 01", 32 ) ; Assert( axes_cone != nullptr, "Cannot create axes cone" ) ;
   
   red_color_index   = BaseColorsSet::addBaseColor( vec3( 1.0f, 0.0f, 0.0f ) ) ;
   green_color_index = BaseColorsSet::addBaseColor( vec3( 0.0f, 1.0f, 0.0f ) ) ;
   blue_color_index  = BaseColorsSet::addBaseColor( vec3( 0.0f, 0.0f, 1.0f ) ) ;

   sphere = new IMSphere() ; Assert( sphere != nullptr, "Cannot create sphere" ) ;

   constexpr float start = -1.5f, end = 5.0f ;

   line_z = new Segment( vec3( 0.0f, 0.0f, start ), vec3( 0.0f, 0.0f, end ), vec3( 0.0f, 0.0f, 1.0f ) ) ;

   // constexpr float d = 1.05, h = 0.15 ;

   // xtri = new IMTriangle( {  
   //       vec3( d,   0.0f, -h ), 
   //       vec3( d,   0.0f, +h ), 
   //       vec3( d+2*h, 0.0f, 0.0f ) 
   //    }, 
   //    vec3( 1.0f, 0.0f, 0.0f ) 
   // ) ;   
   // Assert( xtri != nullptr, "Cannot create X axis triangle" ) ;

   // ytri = new IMTriangle( {  vec3(  -h,  d,   0.0f ), 
   //       vec3(   h,  d,   0.0f ), 
   //       vec3( 0.0f, d+2*h, 0.0f ) 
   //    }, 
   //    vec3( 0.0f, 1.0f, 0.0f ) 
   // ) ;   
   // Assert( ytri != nullptr, "Cannot create Y axis triangle" ) ;

   // ztri = new IMTriangle( {  vec3(   -h, 0.0f, d ), 
   //       vec3(    h, 0.0f, d ), 
   //       vec3( 0.0f, 0.0f, d+2*h ) 
   //    }, 
   //    vec3( 0.0f, 0.0f, 1.0f ) 
   // ) ;   
   // Assert( ztri != nullptr, "Cannot create Z axis triangle" ) ;
}

// ---------------------------------------------------------------------------------------

AxesObject::~AxesObject() 
{
    delete axes_cylinder ; axes_cylinder = nullptr ;
    delete sphere ; sphere = nullptr ;
    delete axes_cone ; axes_cone = nullptr ;
   
   //  delete xtri ; xtri = nullptr ;
   //  delete ytri ; ytri = nullptr ;
   //  delete ztri ; ztri = nullptr ;
}

// ---------------------------------------------------------------------------------------

void AxesObject::drawVK( vkhc::BasicPipeline * pipeline, vkhc::VulkanContext & context, VkCommandBuffer & cmd_vk ) 
{
   Assert( axes_cylinder != nullptr, "Axes cylinder not initialized" ) ;

   // retrieve a valid 3d pipeline  
   using namespace vkhc ;
   using namespace glm ;
   
   Assert( pipeline != nullptr, "No pipeline binded when drawing axes object" ) ;
   Pipeline3D * p = static_cast<Pipeline3D*>( pipeline ) ;
   Assert( p != nullptr, "Current binded pipeline is not a 3D pipeline when drawing axes object" ) ;

   constexpr float
      radius = 0.02f ,
      len_cyl = 0.85f ,
      len_cone = 1.0f - len_cyl ,
      rad_cone = radius*2.0f ; 

   const mat4 
      scale_mat = glm::scale(glm::vec3( radius, radius, len_cyl )),
      scale_mat_cone = glm::scale(glm::vec3( rad_cone, rad_cone, -len_cone)),
      translate_mat_cone = glm::translate(glm::vec3( 0.0f, 0.0f, 1.0f )),
      mat_cone = translate_mat_cone * scale_mat_cone,
      rot_m90_x = glm::rotate( radians(-90.0f), glm::vec3(1.0,0.0,0.0) ),
      rot_90_y  = glm::rotate( radians(90.0f),  glm::vec3(0.0,1.0,0.0) ),
      sphere_scale_mat = glm::scale(glm::vec3( 0.07f, 0.07f, 0.07f )) ;

   // save previous pipeline state
   int prev_texture_index = p->getTextureIndex() ; // save previous texture index
   int prev_base_color_index = p->getBaseColorIndex() ; // save previous base color index

   // disable use of textures
   p->setTextureIndex( cmd_vk, -1 ) ; // disable use of textures

   // draw Z axis cylinder
   p->pushModelMatrix( cmd_vk, scale_mat ) ; 
      p->setBaseColorIndex( cmd_vk, blue_color_index ) ;
      axes_cylinder->drawVK( pipeline, context, cmd_vk ) ; // axis Z (blue)
   p->popModelMatrix( cmd_vk ) ;

   // draw Y axis cylinder
   p->pushModelMatrix( cmd_vk, rot_m90_x* scale_mat ) ;
      p->setBaseColorIndex( cmd_vk, green_color_index ) ;
      axes_cylinder->drawVK( pipeline, context, cmd_vk ) ; // axis Y (green)
   p->popModelMatrix( cmd_vk );

   // draw X axis cylinder
   p->pushModelMatrix( cmd_vk, rot_90_y *scale_mat) ;
      p->setBaseColorIndex( cmd_vk, red_color_index ) ;
      axes_cylinder->drawVK( pipeline, context, cmd_vk ) ; // axis X (red)
   p->popModelMatrix( cmd_vk );


   // draw Z axis cone
   p->pushModelMatrix( cmd_vk, mat_cone ) ; 
      p->setBaseColorIndex( cmd_vk, blue_color_index ) ;
      axes_cone->drawVK( pipeline, context, cmd_vk ) ; // axis Z (blue)
   p->popModelMatrix( cmd_vk ) ;

   // draw Y axis cone
   p->pushModelMatrix( cmd_vk, rot_m90_x* mat_cone ) ;
      p->setBaseColorIndex( cmd_vk, green_color_index ) ;
      axes_cone->drawVK( pipeline, context, cmd_vk ) ; // axis Y (green)
   p->popModelMatrix( cmd_vk );

   // draw X axis cone
   p->pushModelMatrix( cmd_vk, rot_90_y *mat_cone) ;
      p->setBaseColorIndex( cmd_vk, red_color_index ) ;
      axes_cone->drawVK( pipeline, context, cmd_vk ) ; // axis X (red)
   p->popModelMatrix( cmd_vk );


   // draw Z axis line
   p->setBaseColorIndex( cmd_vk, blue_color_index ) ;
   line_z->drawVK( pipeline, context, cmd_vk ) ; // draw a line along the Z axis

   // // draw X axis triangle
   // pipeline3d->setBaseColorIndex( cmd_vk, red_color_index ) ;
   // xtri->drawVK( pipeline, context, cmd_vk ) ; // axis X (red)
   
   // // draw Y axis triangle
   // pipeline3d->setBaseColorIndex( cmd_vk, green_color_index ) ;
   // ytri->drawVK( pipeline, context, cmd_vk ) ; // axis Y (green)
   
   // // draw Z axis triangle
   // pipeline3d->setBaseColorIndex( cmd_vk, blue_color_index ) ;
   // ztri->drawVK( pipeline, context, cmd_vk ) ; // axis Z (blue)

   // draw Sphere
   p->setBaseColorIndex( cmd_vk, -1 ) ; // disable base color for next objects to be drawn
   p->pushModelMatrix( cmd_vk, sphere_scale_mat ) ;
      sphere->drawVK( pipeline, context, cmd_vk ) ; // draw a sphere to test normals, lighting and relative scalings in X, Y, Z
   p->popModelMatrix( cmd_vk ) ;

   // restore previous colors in the pipeline
   p->setBaseColorIndex( cmd_vk,  prev_base_color_index ) ; // restore previous base color index
   p->setTextureIndex( cmd_vk, prev_texture_index ) ; // restore previous texture index

   
}

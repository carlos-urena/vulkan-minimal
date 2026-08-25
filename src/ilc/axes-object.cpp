
#include <cstdlib>

#include <vkhc/pipeline3D.h>
#include <ilc/axes-object.h>

namespace ilc 
{ 

// ---------------------------------------------------------------------------
// crea una tabla de índices para una rejilla con topología cilindrica

void CreateCilindricalTopologyTriangles( std::vector<glm::uvec3> & indices, 
                                            const unsigned na, const unsigned nb )
{
   indices.clear();

   for( unsigned i = 0 ; i < na-1 ; i++ )
   for( unsigned j = 0 ; j < nb-1 ; j++ )
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
   constexpr unsigned int na = 32, nb = 64 ;  // number of rows and columns of vertexes 
   setName( "axes sphere" ) ;

   
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

      vec3 vert = { -ca*cb, -sb, -sa*cb } ;
      vertices.push_back( vert );   
      vert_colors.push_back({ 0.1f, 0.1f, 0.7f });
      //vert_normals.push_back( vert );
      vert_tcc.push_back( { fs, fz } );
   }

   computeNormals() ; // compute normals for the sphere mesh

   assert( vert_normals.size() == vertices.size() );
   assert( vert_colors.size() == vertices.size() );
   assert( vert_tcc.size() == vertices.size() );
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
        vert_colors.push_back( { fs, fz, 0.0f } );
        vert_tcc.push_back( { fs, fz } );
        //vert_normals.push_back( { ca, sa, 0.0f } );
    }


    computeNormals() ; // compute normals for the cylinder mesh
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
      vert_colors.push_back( { fs, fz, 0.0f } );
      vert_tcc.push_back( { fs, fz } );
      //vert_normals.push_back( { ca, sa, 0.0f } );
    }

   computeNormals() ; // compute normals for the cone mesh
}

// -----------------------------------------------------------------------------------------

class Segment : public DrawableObject
{
   private:
      vkhc::VertexArray * vertex_array = nullptr ;
      glm::vec3 p0, p1, color ;
   public:
      Segment( const glm::vec3 & p_p0, const glm::vec3 & p_p1, const glm::vec3 & p_color ) ;
      virtual ~Segment() { delete vertex_array ; vertex_array = nullptr ; } ;
      virtual void drawVK( vkhc::BasicPipeline * pipeline, vkhc::VulkanContext & context, VkCommandBuffer & cmdb_vk ) override ;
} ;
// -----------------------------------------------------------------------------------------


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
      vertex_array = new vkhc::VertexArray( context, VK_PRIMITIVE_TOPOLOGY_LINE_LIST, 4 );
      Assert( vertex_array != nullptr, "cannot create a VAO" );

      std::vector<glm::vec3> vertices = { p0, p1 } ;
      std::vector<glm::vec3> colors = { color, color } ;
      std::vector<glm::vec2> tcc = { glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 1.0f) } ;
      std::vector<glm::vec3> normals = { glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3( 0.0f, 1.0f, 0.0f )} ;

      vertex_array->setAttribData( 0, vertices ) ;
      vertex_array->setAttribData( 1, colors ) ;
      vertex_array->setAttribData( 2, tcc ) ;
      vertex_array->setAttribData( 3, normals ) ;
   }
   vertex_array->draw(  cmdb_vk ) ;
}
// ---------------------------------------------------------------------------------------

constexpr float gstart = -4.0f, gend = 4.0f ;

AxesObject::AxesObject( vkhc::BaseColorsSet * p_base_colors_set ) 
{
   using namespace std ;
   using namespace glm ; 
   using namespace vkhc ;

   setName( "axes object" ) ;
   base_colors_set = p_base_colors_set ;
   Assert( base_colors_set != nullptr, "AxesObject: base colors set is null" ) ;

   cout << "Creating axes object ..." << endl ;

   axes_cylinder = new CylinderZ01( "axes cylinder Z 01", 32 ) ; Assert( axes_cylinder != nullptr, "Cannot create axes cylinder" ) ;
   axes_cone = new ConeZ01( "axes cone Z 01", 32 ) ; Assert( axes_cone != nullptr, "Cannot create axes cone" ) ;
   
   red_color_index   = base_colors_set->add( vec3( 1.0f, 0.0f, 0.0f ) ) ;
   green_color_index = base_colors_set->add( vec3( 0.0f, 1.0f, 0.0f ) ) ;
   blue_color_index  = base_colors_set->add( vec3( 0.0f, 0.5f, 1.0f ) ) ;
   grid_color_index  = base_colors_set->add( vec3( 0.5f, 0.5f, 0.5f ) ) ;

   sphere = new IMSphere() ; Assert( sphere != nullptr, "Cannot create sphere" ) ;
   
   //constexpr float start = -1.5f, end = 5.0f ;

   line_x = new Segment( vec3( gstart, 0.0f,   0.0f ),   vec3( gend, 0.0f, 0.0f ), vec3( 1.0f, 0.0f, 0.0f ) ) ;
   line_y = new Segment( vec3( 0.0f,   gstart, 0.0f ),   vec3( 0.0f, gend, 0.0f ), vec3( 0.0f, 1.0f, 0.0f ) ) ;
   line_z = new Segment( vec3( 0.0f,   0.0f,   gstart ), vec3( 0.0f, 0.0f, gend ), vec3( 0.0f, 0.0f, 1.0f ) ) ;
   line01z = new Segment( vec3( 0.0f,  0.0f,   0.0f ),   vec3( 0.0f, 0.0f, 1.0f ), vec3( 0.5f, 0.5f, 0.5f ) ) ;

   cout << "Axes object created." << endl ;
}

// ---------------------------------------------------------------------------------------

void AxesObject::setActive( bool p_draw_axes, bool p_draw_grid ) 
{
   draw_axes = p_draw_axes ;
   draw_grid = p_draw_grid ;
}

// ---------------------------------------------------------------------------------------

AxesObject::~AxesObject() 
{
   delete axes_cylinder ; axes_cylinder = nullptr ;
   delete sphere ; sphere = nullptr ;
   delete axes_cone ; axes_cone = nullptr ;
   delete line_x ; line_x = nullptr ;
   delete line_y ; line_y = nullptr ;
   delete line_z ; line_z = nullptr ;
}

// ---------------------------------------------------------------------------------------

void AxesObject::drawGridVK( vkhc::Pipeline3D * pipeline, vkhc::VulkanContext & context, VkCommandBuffer & cmd_vk )
{
Assert( axes_cylinder != nullptr, "Axes cylinder not initialized" ) ;

   // retrieve a valid 3d pipeline  
   using namespace vkhc ;
   using namespace glm ;
   
   Assert( pipeline != nullptr, "No pipeline binded when drawing axes object" ) ;
   Pipeline3D * p = static_cast<Pipeline3D*>( pipeline ) ;
   Assert( p != nullptr, "Current binded pipeline is not a 3D pipeline when drawing axes object" ) ;

   
   // draw grid 
   constexpr int n = 40 ;
   
   const mat4 sctr_mat = translate(vec3{ gstart, 0.0f, gstart })* scale(vec3{ gend-gstart, 1.0f, gend-gstart }) ;
   const mat4 rot_90y =  rotate(radians(90.0f), vec3( 0.0f, 1.0f, 0.0f ) ) ;

   p->setBaseColorIndex( cmd_vk, grid_color_index ) ; // set the base color index for the grid lines
   

   for( int iz = 0 ; iz <= n ; iz++ )
   {
      const float fz = float(iz)/float(n);
      p->pushModelMatrix( cmd_vk, sctr_mat*translate( vec3{ fz, 0.0f, 0.0f }) ) ;
         line01z->drawVK( pipeline, context, cmd_vk ) ; // draw a line along the Z axis
      p->popModelMatrix( cmd_vk ) ;
   }
  
   for( int ix = 0 ; ix <= n ; ix++ )
   {
      const float fx = float(ix)/float(n);
      p->pushModelMatrix( cmd_vk, sctr_mat*translate( vec3{ 0.0f, 0.0f, fx }) * rot_90y ) ;
         line01z->drawVK( pipeline, context, cmd_vk ) ; // draw a line along the X axis
      p->popModelMatrix( cmd_vk ) ;
   }
}
// ---------------------------------------------------------------------------------------

void AxesObject::drawAxesVK( vkhc::Pipeline3D * p, vkhc::VulkanContext & context, VkCommandBuffer & cmd_vk )
{
   Assert( axes_cylinder != nullptr, "Axes cylinder not initialized" ) ;

   // retrieve a valid 3d pipeline  
   using namespace vkhc ;
   using namespace glm ;
   
   constexpr float
      radius = 0.017f ,
      len_cyl = 0.85f ,
      len_cone = 1.0f - len_cyl ,
      rad_cone = radius*2.0f ,
      rad_sphere = 0.04f ; // 0.05f ;

   const mat4 
      scale_mat = glm::scale(glm::vec3( radius, radius, len_cyl )),
      scale_mat_cone = glm::scale(glm::vec3( rad_cone, rad_cone, -len_cone)),
      translate_mat_cone = glm::translate(glm::vec3( 0.0f, 0.0f, 1.0f )),
      mat_cone = translate_mat_cone * scale_mat_cone,
      rot_m90_x = glm::rotate( radians(-90.0f), glm::vec3(1.0,0.0,0.0) ),
      rot_90_y  = glm::rotate( radians(90.0f),  glm::vec3(0.0,1.0,0.0) ),
      sphere_scale_mat = glm::scale(glm::vec3( rad_sphere, rad_sphere, rad_sphere )) ;

   
   // debug 
   //p->setEvalIllumination( cmd_vk, true ) ; // enable lighting for next objects to be drawn

   // draw Z axis cylinder
   p->pushModelMatrix( cmd_vk, scale_mat ) ; 
      p->setBaseColorIndex( cmd_vk, blue_color_index ) ;
      axes_cylinder->drawVK( p, context, cmd_vk ) ; // axis Z (blue)
   p->popModelMatrix( cmd_vk ) ;

   // draw Y axis cylinder
   p->pushModelMatrix( cmd_vk, rot_m90_x* scale_mat ) ;
      p->setBaseColorIndex( cmd_vk, green_color_index ) ;
      axes_cylinder->drawVK( p, context, cmd_vk ) ; // axis Y (green)
   p->popModelMatrix( cmd_vk );

   // draw X axis cylinder
   p->pushModelMatrix( cmd_vk, rot_90_y *scale_mat) ;
      p->setBaseColorIndex( cmd_vk, red_color_index ) ;
      axes_cylinder->drawVK( p, context, cmd_vk ) ; // axis X (red)
   p->popModelMatrix( cmd_vk );


   // draw Z axis cone
   p->pushModelMatrix( cmd_vk, mat_cone ) ; 
      p->setBaseColorIndex( cmd_vk, blue_color_index ) ;
      axes_cone->drawVK( p, context, cmd_vk ) ; // axis Z (blue)
   p->popModelMatrix( cmd_vk ) ;

   // draw Y axis cone
   p->pushModelMatrix( cmd_vk, rot_m90_x* mat_cone ) ;
      p->setBaseColorIndex( cmd_vk, green_color_index ) ;
      axes_cone->drawVK( p, context, cmd_vk ) ; // axis Y (green)
   p->popModelMatrix( cmd_vk );

   // draw X axis cone
   p->pushModelMatrix( cmd_vk, rot_90_y *mat_cone) ;
      p->setBaseColorIndex( cmd_vk, red_color_index ) ;
      axes_cone->drawVK( p, context, cmd_vk ) ; // axis X (red)
   p->popModelMatrix( cmd_vk );


   // draw axis lines (deactivated, thin-cylinders are used instead)
   //p->setBaseColorIndex( cmd_vk, -1 ) ;
   //line_x->drawVK( pipeline, context, cmd_vk ) ; // draw a line along the X axis
   //line_y->drawVK( pipeline, context, cmd_vk ) ; // draw a line along the Y axis
   //line_z->drawVK( pipeline, context, cmd_vk ) ; // draw a line along the Z axis

   // draw thin cylinders in the axes
   // draw Z axis cylinder
   constexpr float lines_radius = 0.005f ;
   const mat4 lines_scale_mat = glm::translate(vec3{0.0f,0.0f,gstart})*glm::scale(glm::vec3( lines_radius, lines_radius, gend-gstart )) ;
   p->pushModelMatrix( cmd_vk, lines_scale_mat ) ; 
      p->setBaseColorIndex( cmd_vk, blue_color_index ) ;
      axes_cylinder->drawVK( p, context, cmd_vk ) ; // axis Z (blue)
   p->popModelMatrix( cmd_vk ) ;

   // draw Y axis cylinder
   p->pushModelMatrix( cmd_vk, rot_m90_x* lines_scale_mat ) ;
      p->setBaseColorIndex( cmd_vk, green_color_index ) ;
      axes_cylinder->drawVK( p, context, cmd_vk ) ; // axis Y (green)
   p->popModelMatrix( cmd_vk );

   // draw X axis cylinder
   p->pushModelMatrix( cmd_vk, rot_90_y *lines_scale_mat) ;
      p->setBaseColorIndex( cmd_vk, red_color_index ) ;
      axes_cylinder->drawVK( p, context, cmd_vk ) ; // axis X (red)
   p->popModelMatrix( cmd_vk );

   // -----------
   // draw Sphere
   p->setBaseColorIndex( cmd_vk, -1 ) ; // disable base color for next objects to be drawn
   p->pushModelMatrix( cmd_vk, sphere_scale_mat ) ;
      sphere->drawVK( p, context, cmd_vk ) ; // draw a sphere to test normals, lighting and relative scalings in X, Y, Z
   p->popModelMatrix( cmd_vk ) ;

   // restore 'eval ilum to false' 
   p->setEvalIllumination( cmd_vk, false ) ; // disable lighting for next objects to be drawn
}

// ---------------------------------------------------------------------------------------

void AxesObject::drawVK( vkhc::BasicPipeline * pipeline, vkhc::VulkanContext & context, VkCommandBuffer & cmd_vk ) 
{
   using namespace vkhc ;

   if ( !draw_axes && !draw_grid ) 
      return ; // nothing to draw

   Assert( pipeline != nullptr, "No pipeline binded when drawing axes object" ) ;
   Pipeline3D * pipeline3d = static_cast<Pipeline3D*>( pipeline ) ;
   Assert( pipeline3d != nullptr, "Current binded pipeline is not a 3D pipeline when drawing axes object" ) ;
   
   // save previous pipeline state
   int prev_texture_index = pipeline3d->getTextureIndex() ; // save previous texture index
   int prev_base_color_index = pipeline3d->getBaseColorIndex() ; // save previous base color index
   int prev_brdfs_params_index = pipeline3d->getBrdfParamsIndex() ; // save previous brdfs params index
   int prev_eval_illumination = pipeline3d->getEvalIllumination() ; // save previous illumination evaluation mode
   int prev_draw_edges = pipeline3d->getDrawWireframe() ; // save previous draw edges mode

   // set state of push constants and pipeline settings
   pipeline3d->setDrawWireframe( false ) ; // disable draw wireframe mode
   pipeline3d->setTextureIndex( cmd_vk, -1 ) ; // disable use of textures
   pipeline3d->setEvalIllumination( cmd_vk, false ) ; // disable illumination evaluation in the shaders

   if ( draw_grid ) 
      drawGridVK( pipeline3d, context, cmd_vk ) ;
   if ( draw_axes ) 
      drawAxesVK( pipeline3d, context, cmd_vk ) ;

   // restore state of push constants and pipeline settings
   pipeline3d->setBaseColorIndex( cmd_vk,  prev_base_color_index ) ; // restore previous base color index
   pipeline3d->setBrdfParamsIndex( cmd_vk, prev_brdfs_params_index ) ; // restore previous brdfs params index
   pipeline3d->setTextureIndex( cmd_vk, prev_texture_index ) ; // restore previous texture index
   pipeline3d->setEvalIllumination( cmd_vk, prev_eval_illumination ) ; // restore previous illumination evaluation mode
   pipeline3d->setDrawWireframe( prev_draw_edges ) ; // restore previous draw edges mode
}

// ---------------------------------------------------------------------------------------

} // end namespace ilc
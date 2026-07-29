#include <lights.h>


LightSource::LightSource( const glm::vec4 & p_pos_dir, const glm::vec4 & p_color )
    
{
    pos_dir = p_pos_dir ;
    color = p_color ;
}
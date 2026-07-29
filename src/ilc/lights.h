#pragma once

#include <glm/glm.hpp>

class LightSource
{

    private:
        glm::vec4 pos_dir ; // position (for w=1) or direction (for w=0)
        glm::vec4 color ;   // emission color (RGB)
    public:
        LightSource( const glm::vec4 & p_pos_dir, const glm::vec4 & p_color ) ;
} ;
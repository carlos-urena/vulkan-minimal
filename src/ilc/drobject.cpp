#include <ilc/drobject.h>


const std::string & DrawableObject::getName() const 
{
    return name_str ;
}

void DrawableObject::setName( const std::string & new_name )
{
    name_str = new_name ;
}

void DrawableObject::drawIMGUIWidgets( ) 
{
    // default implementation does nothing
}

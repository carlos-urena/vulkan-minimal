// Implementation for common includes and definitions for all headers and cpps


#include <common.h>


namespace vkhc 
{


const char* getVendorName( uint32_t vendorId ) 
{
    switch (vendorId) {
        case 0x10DE: return "NVIDIA";
        case 0x1002:
        case 0x1022: return "AMD";
        case 0x8086: return "Intel";
        case 0x13B5: return "ARM";
        case 0x5143: return "Qualcomm";
        case 0x1010: return "Imagination Technologies";
        case 0x106B: return "Apple";
        default: return "Unknown vendor";
    }
}

void ErrorExitFunction( const std::string & msg, const char * file, const int line ) 
{
    using namespace std ;
    cerr << "ERROR detected. Aborting. " << endl  
         << "   message : " << msg << "." << endl 
         << "   at      : " << file << ", line " << line << "." << endl 
         << endl ;

    std::exit( EXIT_FAILURE ) ; 
}


void AssertFunction( bool condition, const std::string & msg, const char * file, const int line ) 
{
    if ( condition ) 
        return ;

    const std::string full_msg = "assertion failed: " + msg ;
    ErrorExitFunction( full_msg, file, line ) ;
} 


} // end of namespace vkhc
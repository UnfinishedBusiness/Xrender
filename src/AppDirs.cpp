#include <iostream>
#include <string>
#include <Xrender.h>

using namespace std;

string Xrender_get_env( const std::string & var )
{
  const char * val = std::getenv( var.c_str() );
  if ( val == nullptr )
  {
    return "";
  }
  else
  {
    return val;
  }
}
string Xrender_get_config_dir(string app_name)
{
  #ifdef __linux__   
    return Xrender_get_env("HOME") + "/.config/" + app_name + "/";
  #elif __APPLE__   
    return Xrender_get_env("HOME") + "/Library/" + app_name + "/";
  #elif _WIN32
    return Xrender_get_env("CSIDL_DEFAULT_APPDATA") + app_name + "\\";
  #else     
    #error Platform not supported! Must be Linux, OSX, or Windows
  #endif
}
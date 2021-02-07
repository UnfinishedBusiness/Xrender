#include <Xrender.h>
#include <string>
#include <dxf/DXFParse_Class.h>
#include <dxflib/dl_dxf.h>

using namespace std;

void Xrender_parse_dxf_file(string file, void (*callback)(dxf_object_t o, int x, int n))
{
    DXFParse_Class* creationClass = new DXFParse_Class();
    DL_Dxf* dxf = new DL_Dxf();
    if (!dxf->in(file, creationClass)) { // if file open failed
        std::cerr << file << " could not be opened.\n";
        return;
    }
    delete dxf;
    for (int x = 0; x < creationClass->dxf.size(); x++)
    {
        callback(creationClass->dxf[x], x, creationClass->dxf.size());
    }
    delete creationClass;
}
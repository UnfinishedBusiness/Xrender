#include <Xrender.h>
#include <string>
#include <dxf/DXFParse_Class.h>
#include <dxflib/dl_dxf.h>

using namespace std;

void Xrender_parse_dxf_file(string file)
{
    DXFParse_Class* creationClass = new DXFParse_Class();
    DL_Dxf* dxf = new DL_Dxf();
    if (!dxf->in(file, creationClass)) { // if file open failed
        std::cerr << file << " could not be opened.\n";
        return;
    }
    delete dxf;
    int scale = 100;
    for (int x = 0; x < creationClass->dxf.size(); x++)
    {
        //printf("%d = %s\n", x, creationClass->dxf[x].type.c_str());
        if (creationClass->dxf[x].type == "line")
        {
            Xrender_object_t *l = Xrender_push_line(to_string(x), {int(creationClass->dxf[x].line.start.x * scale), int(creationClass->dxf[x].line.start.y * scale)}, {int(creationClass->dxf[x].line.end.x * scale), int(creationClass->dxf[x].line.end.y * scale)}, 1);
            l->line.color = {255, 0 , 0};
        }
    }
    delete creationClass;
}
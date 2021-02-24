#include "Xrender.h"
#include "primatives/Line.h"
#include "geometry/geometry.h"

Xrender_core_t *Xcore;

double zoom = 1;
double_point_t pan = {0, 0};

int main()
{
    Geometry g;
    if (Xrender_init({{"window_title", "Test1"}, {"maximize", false}, {"clear_color", { {"r", 0}, {"g", 51}, {"b", 102}, {"a", 255}}}}))
    {
        Xcore = Xrender_get_core_variables();

        Xrender_object_t *line = Xrender_push_object(Line({0, 0}, {100, 100}));
        Line *l = (Line*)line->get_obj();
        l->properties.color[1] = 0;

        while(Xrender_tick())
        {
            //Running!
        }
        Xrender_close();
    }
    return 0;
}
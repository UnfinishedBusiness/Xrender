#include "Xrender.h"
#include "primatives/Line.h"
#include "geometry/geometry.h"

Xrender_core_t *Xcore;

double zoom = 1;
double_point_t pan = {0, 0};

bool performance_timer()
{
    printf("FPS: %.4f\n", 1000 / (float)Xrender_get_performance());
    return true;
}

int main()
{
    Geometry g;
    if (Xrender_init({{"window_title", "Test1"}, {"maximize", false}, {"clear_color", { {"r", 0}, {"g", 51}, {"b", 102}, {"a", 255}}}}))
    {
        Xcore = Xrender_get_core_variables();

        Line *line = Xrender_push_object(new Line({0, 0}, {0, 100}));

        while(Xrender_tick())
        {
            Xrender_push_timer(1000, &performance_timer);
        }
        Xrender_close();
    }
    return 0;
}
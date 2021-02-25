#include "Xrender.h"
#include "primatives/Line.h"
#include "geometry/geometry.h"

Xrender_core_t *Xcore;

double zoom = 1;
double_point_t pan = {0, 0};

Line *line;

bool performance_timer()
{
    printf("FPS: %.4f\n", 1000 / (float)Xrender_get_performance());
    //line->properties->scale = 0.5;
    return true;
}
void zoom_in(nlohmann::json e)
{
    line->properties->scale += line->properties->scale * 0.1;
}
void zoom_out(nlohmann::json e)
{
    line->properties->scale -= line->properties->scale * 0.1;
}
int main()
{
    Geometry g;
    if (Xrender_init({{"window_title", "Test1"}, {"maximize", false}, {"clear_color", { {"r", 0}, {"g", 51}, {"b", 102}, {"a", 255}}}}))
    {
        Xcore = Xrender_get_core_variables();

        line = Xrender_push_object(new Line({0, 0}, {0, 100}));

        Xrender_push_timer(1000, &performance_timer);

        Xrender_push_key_event({"up", "scroll", zoom_in});
        Xrender_push_key_event({"down", "scroll", zoom_out});

        while(Xrender_tick())
        {
            
        }
        Xrender_close();
    }
    return 0;
}
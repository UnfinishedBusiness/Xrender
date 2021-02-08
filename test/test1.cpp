#include "Xrender.h"
#include "json/json.h"

Xrender_object_t *performance_label;

double zoom = 0.5;
double_point_t pan = {100, 100};

nlohmann::json dxf_matrix(nlohmann::json data)
{
    nlohmann::json new_data = data;
    if (data["type"] == "line")
    {
        new_data["start"]["x"] = ((double)data["start"]["x"] * zoom) + pan.x;
        new_data["start"]["y"] = ((double)data["start"]["y"] * zoom) + pan.y;
        new_data["end"]["x"] = ((double)data["end"]["x"] * zoom) + pan.x;
        new_data["end"]["y"] = ((double)data["end"]["y"] * zoom) + pan.y;
    }
    if (data["type"] == "arc" || data["type"] == "circle")
    {
        new_data["center"]["x"] = ((double)data["center"]["x"] * zoom) + pan.x;
        new_data["center"]["y"] = ((double)data["center"]["y"] * zoom) + pan.y;
        new_data["radius"] = (double)data["radius"] * zoom;
    }
    return new_data;
}
bool test_timer()
{
    performance_label->data["textval"] = to_string(Xrender_get_performance());
    performance_label->data["size"]["width"] = 0;
    performance_label->data["size"]["height"] = 0;
    Xrender_rebuild_object(performance_label);
    return true;
}
void handle_dxf(nlohmann::json dxf, int x, int n)
{
    Xrender_object_t *o;
    if (dxf["type"] == "line")
    {
        o = Xrender_push_line(dxf);
    }
    if (dxf["type"] == "arc")
    {
        o = Xrender_push_arc(dxf);
    }
    if (dxf["type"] == "circle")
    {
        o = Xrender_push_circle(dxf);
    }
    o->matrix_data = dxf_matrix;
}
void plus_key()
{
    zoom += 1.5;
}
void minus_key()
{
    zoom -= 1.5;
}
int main()
{
    printf("App Config Dir = %s\n", Xrender_get_config_dir("test1").c_str());
    if (Xrender_init({{"window_title", "Test1"}}))
    {
        Xrender_push_key_event({"Up", "keyup", plus_key});
        Xrender_push_key_event({"Down", "keyup", minus_key});
        performance_label = Xrender_push_text({
            {"textval", "0"},
            {"position", {
                {"x", 10},
                {"y", 10}
            }},
            {"font_size", 20}
        });
        Xrender_parse_dxf_file("test.dxf", handle_dxf);
        Xrender_push_timer(10, test_timer);
        while(Xrender_tick())
        {
            //Running!
        }
        Xrender_close();
    }
    return 0;
}
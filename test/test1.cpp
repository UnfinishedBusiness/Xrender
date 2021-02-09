#include "Xrender.h"
#include "json/json.h"
#include "serial/serial.h"
#include "geometry/geometry.h"

Xrender_object_t *performance_label;
Xrender_object_t *circle;

serial::Serial serial_port;

double zoom = 30;
double_point_t pan = {100, 100};

void mouse_callback(Xrender_object_t* o,nlohmann::json e)
{
    if (e["event"] == "mouse_in")
    {
        o->data["color"]["g"] = 255;
    }
    if (e["event"] == "mouse_out")
    {
        o->data["color"]["g"] = 0;
    }
    if (e["event"] == "left_click_down")
    {
        o->data["color"]["r"] = 255;
    }
    if (e["event"] == "left_click_up")
    {
        o->data["color"]["r"] = 0;
    }
    if (e["event"] == "right_click_down")
    {
        o->data["color"]["b"] = 255;
    }
    if (e["event"] == "right_click_up")
    {
        o->data["color"]["b"] = 0;
    }
    if (e["event"] == "middle_click_down")
    {
        o->data["color"]["b"] = 255;
        o->data["color"]["r"] = 255;
    }
    if (e["event"] == "middle_click_up")
    {
        o->data["color"]["b"] = 0;
        o->data["color"]["r"] = 0;
    }
}
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
    o->data["zindex"] = 0;
    o->matrix_data = dxf_matrix;
    o->mouse_callback = mouse_callback;
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
    Geometry g;
    printf("App Config Dir = %s\n", Xrender_get_config_dir("test1").c_str());
    if (Xrender_init({{"window_title", "Test1"}, {"clear_color", { {"r", 220}, {"g", 220}, {"b", 220}, {"a", 255}}}}))
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

        circle = Xrender_push_circle({
            {"center", {
                {"x", 5},
                {"y", 5}
            }},
            {"radius", 10}
        });
        circle->mouse_callback = mouse_callback;
        circle->matrix_data = dxf_matrix;

        Xrender_parse_dxf_file("test.dxf", handle_dxf);
        Xrender_push_timer(10, test_timer);

        std::vector<serial::PortInfo> devices_found = serial::list_ports();
        std::vector<serial::PortInfo>::iterator iter = devices_found.begin();
        while( iter != devices_found.end() )
        {
            serial::PortInfo device = *iter++;
            printf("%s - %s\n\r", device.port.c_str(), device.description.c_str());
        }
        while(Xrender_tick())
        {
            //Running!
        }
        Xrender_close();
    }
    return 0;
}
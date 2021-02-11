#include "Xrender.h"
#include "json/json.h"
#include "serial/serial.h"
#include "geometry/geometry.h"
#include "gui/imgui.h"

Xrender_gui_t *test_window;
Xrender_object_t *performance_label;
Xrender_object_t *circle;

serial::Serial serial_port;

double zoom = 1;
double_point_t pan = {0, 0};
double_point_t mouse_pos_in_screen_coordinates = {0, 0};
double_point_t mouse_pos_in_matrix_coordinates = {0, 0};

void mouse_motion(nlohmann::json e)
{
    //printf("%s\n", e.dump().c_str());
    mouse_pos_in_screen_coordinates = {(double)e["pos"]["x"], (double)e["pos"]["y"]};
    mouse_pos_in_matrix_coordinates = {
        (mouse_pos_in_screen_coordinates.x -  pan.x) / zoom,
        (mouse_pos_in_screen_coordinates.y - pan.y) / zoom
    };
    //printf("Mouse pos: (%.4f, %.4f)\n", mouse_pos_in_matrix_coordinates.x, mouse_pos_in_matrix_coordinates.y);
}

void mouse_callback(Xrender_object_t* o,nlohmann::json e)
{
    Geometry g;
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
        if (o->data["type"] == "line")
        {
            printf("Angle: %.4f\n", g.measure_polar_angle({(double)o->data["start"]["x"], (double)o->data["start"]["y"]}, {(double)o->data["end"]["x"], (double)o->data["end"]["y"]}));
        }
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
        new_data["center"]["y"] = ((double)data["center"]["y"]* zoom) + pan.y;
        new_data["radius"] = ((double)data["radius"] * zoom);
    }
    return new_data;
}
bool test_timer()
{
    performance_label->data["textval"] = to_string(1000.0f / (float)Xrender_get_performance());
    performance_label->data["size"]["width"] = 0;
    performance_label->data["size"]["height"] = 0;
    Xrender_rebuild_object(performance_label);
    return true;
}
void handle_dxf(nlohmann::json dxf, int x, int n)
{
    Geometry g;
    Xrender_object_t *o;
    if (dxf["type"] == "line")
    {
        o = Xrender_push_line(dxf);
    }
    if (dxf["type"] == "arc")
    {
        o = Xrender_push_arc(dxf);
        /*double_line_t start_line = g.create_polar_line({o->data["center"]["x"], o->data["center"]["y"]}, o->data["start_angle"], o->data["radius"]);
        double_line_t end_line = g.create_polar_line({o->data["center"]["x"], o->data["center"]["y"]}, o->data["end_angle"], o->data["radius"]);
        circle = Xrender_push_circle({
            {"center", {
                {"x", start_line.end.x},
                {"y", start_line.end.y}
            }},
            {"color", {
                {"r", 0},
                {"g", 255},
                {"b", 0}, 
                {"a", 255}
            }},
            {"radius", 10}
        });
        circle->matrix_data = dxf_matrix;
        circle->mouse_callback = NULL;
        circle = Xrender_push_circle({
            {"center", {
                {"x", end_line.end.x},
                {"y", end_line.end.y}
            }},
            {"color", {
                {"r", 255},
                {"g", 0},
                {"b", 0}, 
                {"a", 255}
            }},
            {"radius", 10}
        });
        circle->matrix_data = dxf_matrix;
        circle->mouse_callback = NULL;*/
    }
    if (dxf["type"] == "circle")
    {
        o = Xrender_push_circle(dxf);
    }
    o->data["zindex"] = 0;
    o->matrix_data = dxf_matrix;
    o->mouse_callback = mouse_callback;
}
void plus_key(nlohmann::json e)
{
    double old_zoom = zoom;
    zoom += zoom * 0.125;
    if (zoom > 1000000)
    {
        zoom = 1000000;
    }
    double scalechange = old_zoom - zoom;
    pan.x += mouse_pos_in_matrix_coordinates.x * scalechange;
    pan.y += mouse_pos_in_matrix_coordinates.y * scalechange; 
}
void minus_key(nlohmann::json e)
{
    double old_zoom = zoom;
    zoom += zoom * -0.125;
    if (zoom < 0.00001)
    {
        zoom = 0.00001;
    }
    double scalechange = old_zoom - zoom;
    pan.x += mouse_pos_in_matrix_coordinates.x * scalechange;
    pan.y += mouse_pos_in_matrix_coordinates.y * scalechange; 
}
void up(nlohmann::json e)
{
    pan.y += 1.5;
    //printf("pan.y: %.4f\n", pan.y);
}
void down(nlohmann::json e)
{
    pan.y -= 1.5;
    //printf("pan.y: %.4f\n", pan.y);
}
void left(nlohmann::json e)
{
    pan.x -= 1.5;
    //printf("pan.x: %.4f\n", pan.x);
}
void right(nlohmann::json e)
{
    pan.x += 1.5;
    //printf("pan.x: %.4f\n", pan.x);
}
void _test_window()
{
    ImGui::Begin("My First Tool", &test_window->visable, ImGuiWindowFlags_MenuBar);
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Open..", "Ctrl+O")) { /* Do stuff */ }
            if (ImGui::MenuItem("Save", "Ctrl+S"))   { /* Do stuff */ }
            if (ImGui::MenuItem("Close", "Ctrl+W"))  { test_window->visable = false; }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }
    // Plot some values
    const float my_values[] = { 0.2f, 0.1f, 1.0f, 0.5f, 0.9f, 2.2f };
    ImGui::PlotLines("Frame Times", my_values, IM_ARRAYSIZE(my_values));

    // Display contents in a scrolling region
    ImGui::TextColored(ImVec4(1,1,0,1), "Important Stuff");
    ImGui::BeginChild("Scrolling");
    for (int n = 0; n < 50; n++)
        ImGui::Text("%04d: Some text", n);
    ImGui::EndChild();
    ImGui::End();
}
int main()
{
    Geometry g;
    printf("App Config Dir = %s\n", Xrender_get_config_dir("test1").c_str());
    if (Xrender_init({{"window_title", "Test1"}, {"maximize", true}, {"clear_color", { {"r", 220}, {"g", 220}, {"b", 220}, {"a", 255}}}}))
    {
        Xrender_push_key_event({"up", "scroll", plus_key});
        Xrender_push_key_event({"down", "scroll", minus_key});
        Xrender_push_key_event({"none", "mouse_move", mouse_motion});
        Xrender_push_key_event({"Up", "keyup", up});
        Xrender_push_key_event({"Down", "keyup", down});
        Xrender_push_key_event({"Left", "keyup", left});
        Xrender_push_key_event({"Right", "keyup", right});

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

        Xrender_parse_dxf_file("test1.dxf", handle_dxf);
        Xrender_push_timer(100, test_timer);

        test_window = Xrender_push_gui(true, _test_window);

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
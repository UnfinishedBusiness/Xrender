#include "Xrender.h"
#include "json/json.h"
#include "serial/serial.h"
#include "geometry/geometry.h"
#include "gui/imgui.h"
#include "gui/ImGuiFileDialog.h"
#include "gui/TextEditor.h"
#include "stk500/stk500.h"

#include "http/httplib.h"

TextEditor editor;
static float progress = 0.0f;
Xrender_gui_t *menu_bar;
Xrender_gui_t *progress_window;
Xrender_gui_t *editor_window;
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
        (mouse_pos_in_screen_coordinates.x - pan.x) / zoom,
        (mouse_pos_in_screen_coordinates.y - pan.y) / zoom
    };
    //printf("Mouse pos: (%.4f, %.4f)\n", mouse_pos_in_matrix_coordinates.x, mouse_pos_in_matrix_coordinates.y);
}

void mouse_callback(Xrender_object_t* o,nlohmann::json e)
{
    Geometry g;
    if (e["event"] == "mouse_in")
    {
        o->data["color"]["r"] = 0;
        o->data["color"]["g"] = 255;
        o->data["color"]["b"] = 0;
    }
    if (e["event"] == "mouse_out")
    {
        o->data["color"]["r"] = 255;
        o->data["color"]["g"] = 255;
        o->data["color"]["b"] = 255;
    }
    if (e["event"] == "left_click_down")
    {
        if (o->data["type"] == "line")
        {
            printf("Angle: %.4f\n", g.measure_polar_angle({(double)o->data["start"]["x"], (double)o->data["start"]["y"]}, {(double)o->data["end"]["x"], (double)o->data["end"]["y"]}));
        }
    }
    if (e["event"] == "left_click_up")
    {

    }
    if (e["event"] == "right_click_down")
    {

    }
    if (e["event"] == "right_click_up")
    {

    }
    if (e["event"] == "middle_click_down")
    {

    }
    if (e["event"] == "middle_click_up")
    {

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
    }
    if (dxf["type"] == "circle")
    {
        o = Xrender_push_circle(dxf);
    }
    o->data["color"] = {
        {"r", 255},
        {"g", 255},
        {"b", 255},
        {"a", 255}
    };
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
void _menu_bar()
{
    if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey", ImGuiWindowFlags_NoCollapse, ImVec2(500, 500))) 
    {
        if (ImGuiFileDialog::Instance()->IsOk())
        {
            std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
            std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();
            printf("File Path: %s\n", filePathName.c_str());
            Xrender_parse_dxf_file(filePathName, handle_dxf);
                    // action
        }
                    // close
        ImGuiFileDialog::Instance()->Close();
    }
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Open", "CTRL+O"))
            {
                ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".dxf", ".");
            }
            if (ImGui::MenuItem("Update", ""))
            {
                //stk500_write_program("firmware.hex", "/dev/cu.usbmodemFA121");
                Xrender_core_t *c = Xrender_get_core_variables();
                c->data["clear_color"]["r"] = 255;

            }
            if (ImGui::MenuItem("Close", "")) {}
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit"))
        {
            if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
            if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
            ImGui::Separator();
            if (ImGui::MenuItem("Cut", "CTRL+X")) {}
            if (ImGui::MenuItem("Copy", "CTRL+C")) {}
            if (ImGui::MenuItem("Paste", "CTRL+V")) {}
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}
char buf[200];
void _progress_window()
{
    ImGui::SetNextWindowSize(ImVec2(550, 60), ImGuiCond_FirstUseEver);
    ImGui::Begin("Progress", &progress_window->visable, 0);
    ImGui::ProgressBar(progress, ImVec2(0.0f, 0.0f));
    ImGui::InputText("string", buf, IM_ARRAYSIZE(buf));
    ImGui::End();
}
void _editor_window()
{
    auto cpos = editor.GetCursorPosition();
	ImGui::Begin("Text Editor", nullptr, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_MenuBar);
	ImGui::SetWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Save"))
			{
				auto textToSave = editor.GetText();
				/// save text....
			}
			if (ImGui::MenuItem("Quit", "Alt-F4"))
            {
                editor_window->visable = false;
            }
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Edit"))
		{
			bool ro = editor.IsReadOnly();
			if (ImGui::MenuItem("Read-only mode", nullptr, &ro))
				editor.SetReadOnly(ro);
			ImGui::Separator();
			if (ImGui::MenuItem("Undo", "ALT-Backspace", nullptr, !ro && editor.CanUndo()))
            {
                editor.Undo();
            }
			if (ImGui::MenuItem("Redo", "Ctrl-Y", nullptr, !ro && editor.CanRedo()))
            {
                editor.Redo();
            }
			ImGui::Separator();
			if (ImGui::MenuItem("Copy", "Ctrl-C", nullptr, editor.HasSelection()))
            {
                editor.Copy();
            }			
			if (ImGui::MenuItem("Cut", "Ctrl-X", nullptr, !ro && editor.HasSelection()))
            {
                editor.Cut();
            }	
			if (ImGui::MenuItem("Delete", "Del", nullptr, !ro && editor.HasSelection()))
            {
                editor.Delete();
            }
			if (ImGui::MenuItem("Paste", "Ctrl-V", nullptr, !ro && ImGui::GetClipboardText() != nullptr))
            {
                editor.Paste();
            }
			ImGui::Separator();
			if (ImGui::MenuItem("Select all", nullptr, nullptr))
            {
                editor.SetSelection(TextEditor::Coordinates(), TextEditor::Coordinates(editor.GetTotalLines(), 0));
            }
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("View"))
		{
			if (ImGui::MenuItem("Dark palette"))
            {
                editor.SetPalette(TextEditor::GetDarkPalette());
            }
			if (ImGui::MenuItem("Light palette"))
            {
                editor.SetPalette(TextEditor::GetLightPalette());
            }
			if (ImGui::MenuItem("Retro blue palette"))
            {
                editor.SetPalette(TextEditor::GetRetroBluePalette());
            }
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}
	ImGui::Text("%6d/%-6d %6d lines  | %s | %s | %s", cpos.mLine + 1, cpos.mColumn + 1, editor.GetTotalLines(),
		editor.IsOverwrite() ? "Ovr" : "Ins",
		editor.CanUndo() ? "*" : " ",
		editor.GetLanguageDefinition().mName.c_str());

	editor.Render("TextEditor");
    ImGui::End();
}
int main()
{
    Geometry g;
    printf("App Config Dir = %s\n", Xrender_get_config_dir("test1").c_str());
    if (Xrender_init({{"window_title", "Test1"}, {"maximize", true}, {"clear_color", { {"r", 0}, {"g", 51}, {"b", 102}, {"a", 255}}}}))
    {
        Xrender_push_key_event({"up", "scroll", plus_key});
        Xrender_push_key_event({"down", "scroll", minus_key});
        Xrender_push_key_event({"none", "mouse_move", mouse_motion});
        Xrender_push_key_event({"Up", "keyup", up});
        Xrender_push_key_event({"Down", "keyup", down});
        Xrender_push_key_event({"Left", "keyup", left});
        Xrender_push_key_event({"Right", "keyup", right});

        auto lang = TextEditor::LanguageDefinition::CPlusPlus();
        editor.SetLanguageDefinition(lang);

        performance_label = Xrender_push_text({
            {"textval", "0"},
            {"position", {
                {"x", 10},
                {"y", 10}
            }},
            {"font_size", 20}
        });

        /*circle = Xrender_push_circle({
            {"center", {
                {"x", 5},
                {"y", 5}
            }},
            {"radius", 10}
        });
        circle->mouse_callback = mouse_callback;
        circle->matrix_data = dxf_matrix;*/

        Xrender_push_timer(100, test_timer);

        menu_bar = Xrender_push_gui(true, _menu_bar);

        progress_window = Xrender_push_gui(false, _progress_window);

        editor_window = Xrender_push_gui(false, _editor_window);

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
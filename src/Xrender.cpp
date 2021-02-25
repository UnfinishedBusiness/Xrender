#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <sys/time.h>
#include <ctime>
#include <algorithm> 
#include <Xrender.h>
#include <geometry/geometry.h>
#include <json/json.h>
#include <gui/imgui.h>
#include <gui/imgui_impl_glfw.h>
#include <gui/imgui_impl_opengl2.h>
#include <fonts/Sans.ttf.h>
#define STB_IMAGE_IMPLEMENTATION
#include <gui/stb_image.h>
#define STB_TRUETYPE_IMPLEMENTATION
#include <gui/stb_truetype.h>

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif
#include <GLFW/glfw3.h>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
   //define something for Windows (32-bit and 64-bit, this part is common)
   #include <GL/freeglut.h>
   #include <GL/gl.h>
   #define GL_CLAMP_TO_EDGE 0x812F
   #ifdef _WIN64
      //define something for Windows (64-bit only)
   #else
      //define something for Windows (32-bit only)
   #endif
#elif __APPLE__
    #include <OpenGL/glu.h>
#elif __linux__
    #include <GL/glu.h>
#elif __unix__
    #include <GL/glu.h>
#elif defined(_POSIX_VERSION)
    // POSIX
#else
#   error "Unknown compiler"
#endif

#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif
unsigned long tick_performance; //Measurement of how long the tick function is taking in ms
unsigned long tick_performance_timestamp; //Millis timestamp of when last tick function began


Xrender_core_t *core;
int mouse_check_skip_cycles = 0;

vector<Xrender_key_event_t> key_events;
vector<Xrender_object_t*> object_stack;
vector<Xrender_timer_t> timers;
vector<Xrender_gui_t*> gui_stack;

Line* Xrender_push_object(Line *l)
{  
    Xrender_object_t *o = new Xrender_object_t(l);
    object_stack.push_back(o);
    return o->line;
}

void Xrender_object_t::process_mouse(float mpos_x, float mpos_y)
{
    if (this->type == "line")
    {
        this->line->process_mouse(mpos_x, mpos_y);
    }
}
void Xrender_object_t::render()
{
    if (this->type == "line")
    {
        this->line->render();
    }
}

vector<Xrender_object_t*> *Xrender_get_object_stack()
{
    return &object_stack;
}

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}
static void window_close_callback(GLFWwindow* window)
{
    glfwSetWindowShouldClose(core->window, GL_TRUE);
}
static void Xrender_scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    if (!io.WantCaptureKeyboard || !io.WantCaptureMouse)
    {
        if (yoffset > 0)
        {
            for (int x = 0; x < key_events.size(); x++)
            {
                if (key_events[x].type == "scroll" && key_events[x].key == "up")
                {
                    key_events.at(x).callback({"scroll", yoffset});
                }
            }
        }
        else
        {
            for (int x = 0; x < key_events.size(); x++)
            {
                if (key_events[x].type == "scroll" && key_events[x].key == "down")
                {
                    key_events.at(x).callback({"scroll", yoffset});
                }
            }
        }
    }
}
static void Xrender_mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    if (!io.WantCaptureKeyboard || !io.WantCaptureMouse)
    {
        double_point_t m = Xrender_get_current_mouse_position();
        if (button == 0) //Left click
        {
            if (action == 0) //Up
            {
                
            }
            if (action == 1) //Down
            {
                
            }
        }
        if (button == 1) //right click
        {
            if (action == 0) //Up
            {
                
            }
            if (action == 1) //Down
            {
                
            }
        }
        if (button == 2) //middle click
        {
            if (action == 0) //Up
            {
                
            }
            if (action == 1) //Down
            {
                
            }
        }
    }
}
static void Xrender_cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    double_point_t m = Xrender_get_current_mouse_position();
    for (int x = 0; x < key_events.size(); x++)
    {
        if (key_events[x].type == "mouse_move")
        {
            key_events.at(x).callback({{"pos",{{"x", m.x},{"y", m.y}}}});
        }
    }
}
static void Xrender_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    string keyname;
    if (glfwGetKeyName(key, scancode) != NULL)
    {
        keyname = string(glfwGetKeyName(key, scancode));
    }
    else
    {
        //printf("Unknown key: %d\n", key);
        switch(key)
        {
            case 256: keyname = "Escape"; break;
            case 32: keyname = "Space"; break;
            case 258: keyname = "Tab"; break;
            case 265: keyname = "Up"; break;
            case 264: keyname = "Down"; break;
            case 263: keyname = "Left"; break;
            case 262: keyname = "Right"; break;
            default: keyname = "None";
        }
    }
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    if (!io.WantCaptureKeyboard || !io.WantCaptureMouse)
    {
        for (int x = 0; x < key_events.size(); x++)
        {
            if (key_events.at(x).type == "keyup" && action == 1)
            {
                if (key_events.at(x).key == keyname)
                {
                    if (key_events.at(x).callback != NULL)
                    {
                        key_events.at(x).callback({{"type", key_events.at(x).type}, {"key", keyname}, {"action", action}});
                    }
                }
            }
            if (key_events.at(x).type == "keydown" && action == 0)
            {
                if (key_events.at(x).key == keyname)
                {
                    if (key_events.at(x).callback != NULL)
                    {
                        key_events.at(x).callback({{"type", key_events.at(x).type}, {"key", keyname}, {"action", action}});
                    }
                }
            }
            if (key_events.at(x).type == "repeat" && action == 2)
            {
                if (key_events.at(x).key == keyname)
                {
                    if (key_events.at(x).callback != NULL)
                    {
                        key_events.at(x).callback({{"type", key_events.at(x).type}, {"key", keyname}, {"action", action}});
                    }
                }
            }
        }
    }
}
static void Xrender_window_size_callback(GLFWwindow* window, int width, int height)
{
    for (int x = 0; x < key_events.size(); x++)
    {
        if (key_events[x].type == "window_resize")
        {
            key_events.at(x).callback({{"size",{{"width", width},{"height", height}}}});
        }
    }
}
Xrender_core_t *Xrender_get_core_variables()
{
    return core;
}
bool Xrender_init(nlohmann::json i)
{
    core = new Xrender_core_t;
    core->data = i;
    /*
        Make sure core->data paramaters are set and if not, then default! 
    */
    if (!core->data.contains("window_title"))
    {
        core->data["window_title"] = "Xrender";
    }
    if (!core->data.contains("window_width"))
    {
        core->data["window_width"] = 900;
    }
    if (!core->data.contains("window_height"))
    {
        core->data["window_height"] = 700;
    }
    if (!core->data.contains("show_cursor"))
    {
        core->data["show_cursor"] = true;
    }
    if (!core->data.contains("maximize"))
    {
        core->data["maximize"] = false;
    }
    if (!core->data.contains("ini_file_name"))
    {
        core->data["ini_file_name"] = "gui.ini";
    }
    if (!core->data.contains("log_file_name"))
    {
        core->data["log_file_name"] = "gui.log";
    }
    if (!core->data.contains("gui_style"))
    {
        core->data["gui_style"] = "light";
    }
    if (!core->data.contains("clear_color"))
    {
        core->data["clear_color"] = {
            {"r", 200},
            {"g", 200},
            {"b", 200},
            {"a", 255}
        };
    }
    bool success = true;
    if (!glfwInit())
    {
        printf("Could not init GLFW3!\n");
        success = false;
    }
    core->window = glfwCreateWindow((int)core->data["window_width"], (int)core->data["window_height"], string(core->data["window_title"]).c_str(), NULL, NULL);
    if (core->window == NULL)
    {
        printf("Could not open window!\n");
        success = false;
    }
    glfwSetKeyCallback(core->window, Xrender_key_callback);
    glfwSetMouseButtonCallback(core->window, Xrender_mouse_button_callback);
    glfwSetScrollCallback(core->window, Xrender_scroll_callback);
    glfwSetWindowCloseCallback(core->window, window_close_callback);
    glfwSetCursorPosCallback(core->window, Xrender_cursor_position_callback);
    glfwSetWindowSizeCallback(core->window, Xrender_window_size_callback);
    glfwMakeContextCurrent(core->window);
    glfwSwapInterval(1); // Enable vsync
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    core->IniFileName = (char*)malloc(sizeof(char) * strlen(string(core->data["ini_file_name"]).c_str()));
    strcpy(core->IniFileName, string(core->data["ini_file_name"]).c_str());
    core->LogFileName = (char*)malloc(sizeof(char) * strlen(string(core->data["log_file_name"]).c_str()));
    strcpy(core->LogFileName, string(core->data["log_file_name"]).c_str());
    io.IniFilename = core->IniFileName;
    io.LogFilename = core->LogFileName;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    ImGui::StyleColorsLight();
    //ImGui::StyleColorsClassic();
    ImGui_ImplGlfw_InitForOpenGL(core->window, true);
    ImGui_ImplOpenGL2_Init();
    if (core->data["show_cursor"] == false)
    {
        glfwSetInputMode(core->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
    if (core->data["maximize"] == true)
    {
        glfwMaximizeWindow(core->window);
    }
    Xrender_tick();
	return success;
}
void call_mouse_callback(Xrender_object_t* o, nlohmann::json matrix_data, double mouseX, double mouseY, std::string event) 
{
    /*if (o->mouse_callback != NULL)
    {
         o->mouse_callback(o, {
            {"data", o->data},
            {"matrix_data", matrix_data},
            {"event", event},
            {"mouse_pos", {
                {"x", mouseX},
                {"y", mouseY}
            }}
        });
    }*/
}
double_point_t Xrender_get_current_mouse_position()
{
    double mouseX, mouseY;
    glfwGetCursorPos(core->window, &mouseX, &mouseY);
    //return {mouseX, mouseY};
    return {mouseX - ((double)core->data["window_width"] / 2.0f), ((double)core->data["window_height"] - mouseY) - ((double)core->data["window_height"] / 2.0f)};
}
bool Xrender_tick()
{
    tick_performance_timestamp = Xrender_millis();
    Geometry g;
    int window_width, window_height;
    float ratio;
    double_point_t m = Xrender_get_current_mouse_position();
    glfwGetFramebufferSize(core->window, &window_width, &window_height);
    ratio = window_width / (float) window_height;
    core->data["window_width"] = window_width;
    core->data["window_height"] = window_height;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthRange(0, 1);
    glDepthFunc(GL_LEQUAL);
    glClearColor((float)core->data["clear_color"]["r"] / 255, (float)core->data["clear_color"]["g"] / 255, (float)core->data["clear_color"]["b"] / 255, (float)core->data["clear_color"]["a"] / 255);
    ImGui_ImplOpenGL2_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    for (int x = 0; x < gui_stack.size(); x++)
    {
        if (gui_stack[x]->visable == true)
        {
            gui_stack[x]->callback();
        }
    }
    ImGui::Render();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho( -window_width/2, window_width/2, -window_height/2, window_height/2, -1,1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glViewport(0, 0, window_width, window_height);
    glClear(GL_COLOR_BUFFER_BIT);
	sort(object_stack.begin(), object_stack.end(), [](auto* lhs, auto* rhs) {
        return lhs->properties->zindex < rhs->properties->zindex;
    });
    for (int x = 0; x < object_stack.size(); x++)
    {
        object_stack[x]->render();
        object_stack[x]->process_mouse(m.x, m.y);
    }
    ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
    glfwMakeContextCurrent(core->window);
    glfwSwapBuffers(core->window);
    glfwPollEvents();
    for (int x = 0; x < timers.size(); x++)
    {
        if ((Xrender_millis() - timers[x].timer) > timers[x].intervol)
        {
            timers[x].timer = Xrender_millis();
            if (timers[x].callback != NULL)
            {
                if (timers[x].callback() == false) //Dont repeat
                {
                    timers.erase(timers.begin() + x);
                }
            }
        }
    }
    tick_performance = Xrender_millis() - tick_performance_timestamp;
    return !glfwWindowShouldClose(core->window);
}
void Xrender_close()
{
    for (int x = 0; x < object_stack.size(); x++)
    {
        delete object_stack[x];
    }
    object_stack.clear();
    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(core->window);
    glfwTerminate();
    free(core->IniFileName);
    free(core->LogFileName);
    delete core;
}
unsigned long Xrender_get_performance()
{
    return tick_performance;
}
Xrender_gui_t *Xrender_push_gui(bool visable, void (*callback)())
{
    Xrender_gui_t *g = new Xrender_gui_t;
    g->visable = visable;
    g->callback = callback;
    gui_stack.push_back(g);
    return g;
}
void Xrender_push_key_event(Xrender_key_event_t e)
{
    key_events.push_back(e);
    if (e.type == "window_resize")
    {
        /*
            Window resize event should be called right away just once to make sure client app has propper dimensions for their gui layout!
            We tick once to make sure all objects (like text and images) have their size calculated
        */
        for (int x = 0; x < 3; x++) Xrender_tick();
        Xrender_window_size_callback(core->window, (int)core->data["window_width"], (int)core->data["window_height"]);
    }
}
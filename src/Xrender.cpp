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

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif
#include <GLFW/glfw3.h>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
   //define something for Windows (32-bit and 64-bit, this part is common)
   #include <GL/freeglut.h>
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
#define MOUSE_CHECK_CYCLE 1

vector<Xrender_key_event_t> key_events;
vector<Xrender_object_t*> object_stack;
vector<Xrender_timer_t> timers;
vector<Xrender_gui_t*> gui_stack;

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
                for (int x = 0; x < object_stack.size(); x++)
                {
                    if (object_stack[x]->data["visable"] == true && object_stack[x]->data["mouse_over"] == true)
                    {
                        if (object_stack[x]->mouse_callback != NULL)
                        {
                            nlohmann::json matrix_data = object_stack[x]->data;
                            if (object_stack[x]->matrix_data != NULL)
                            {
                                matrix_data = object_stack[x]->matrix_data(object_stack[x]->data);
                            }
                            call_mouse_callback(object_stack[x], matrix_data, m.x, m.y, "left_click_up");
                        }
                    }
                }
            }
            if (action == 1) //Down
            {
                for (int x = 0; x < object_stack.size(); x++)
                {
                    if (object_stack[x]->data["visable"] == true && object_stack[x]->data["mouse_over"] == true)
                    {
                        if (object_stack[x]->mouse_callback != NULL)
                        {
                            nlohmann::json matrix_data = object_stack[x]->data;
                            if (object_stack[x]->matrix_data != NULL)
                            {
                                matrix_data = object_stack[x]->matrix_data(object_stack[x]->data);
                            }
                            call_mouse_callback(object_stack[x], matrix_data, m.x, m.y, "left_click_down");
                        }
                    }
                }
            }
        }
        if (button == 1) //right click
        {
            if (action == 0) //Up
            {
                for (int x = 0; x < object_stack.size(); x++)
                {
                    if (object_stack[x]->data["visable"] == true && object_stack[x]->data["mouse_over"] == true)
                    {
                        if (object_stack[x]->mouse_callback != NULL)
                        {
                            nlohmann::json matrix_data = object_stack[x]->data;
                            if (object_stack[x]->matrix_data != NULL)
                            {
                                matrix_data = object_stack[x]->matrix_data(object_stack[x]->data);
                            }
                            call_mouse_callback(object_stack[x], matrix_data, m.x, m.y, "right_click_up");
                        }
                    }
                }
            }
            if (action == 1) //Down
            {
                for (int x = 0; x < object_stack.size(); x++)
                {
                    if (object_stack[x]->data["visable"] == true && object_stack[x]->data["mouse_over"] == true)
                    {
                        if (object_stack[x]->mouse_callback != NULL)
                        {
                            nlohmann::json matrix_data = object_stack[x]->data;
                            if (object_stack[x]->matrix_data != NULL)
                            {
                                matrix_data = object_stack[x]->matrix_data(object_stack[x]->data);
                            }
                            call_mouse_callback(object_stack[x], matrix_data, m.x, m.y, "right_click_down");
                        }
                    }
                }
            }
        }
        if (button == 2) //middle click
        {
            if (action == 0) //Up
            {
                for (int x = 0; x < object_stack.size(); x++)
                {
                    if (object_stack[x]->data["visable"] == true && object_stack[x]->data["mouse_over"] == true)
                    {
                        if (object_stack[x]->mouse_callback != NULL)
                        {
                            nlohmann::json matrix_data = object_stack[x]->data;
                            if (object_stack[x]->matrix_data != NULL)
                            {
                                matrix_data = object_stack[x]->matrix_data(object_stack[x]->data);
                            }
                            call_mouse_callback(object_stack[x], matrix_data, m.x, m.y, "middle_click_up");
                        }
                    }
                }
            }
            if (action == 1) //Down
            {
                for (int x = 0; x < object_stack.size(); x++)
                {
                    if (object_stack[x]->data["visable"] == true && object_stack[x]->data["mouse_over"] == true)
                    {
                        if (object_stack[x]->mouse_callback != NULL)
                        {
                            nlohmann::json matrix_data = object_stack[x]->data;
                            if (object_stack[x]->matrix_data != NULL)
                            {
                                matrix_data = object_stack[x]->matrix_data(object_stack[x]->data);
                            }
                            call_mouse_callback(object_stack[x], matrix_data, m.x, m.y, "middle_click_down");
                        }
                    }
                }
            }
        }
    }
}
static void Xrender_cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    double_point_t m = {xpos - ((double)core->data["window_width"] / 2.0f), ((double)core->data["window_height"] - ypos) - ((double)core->data["window_height"] / 2.0f)};
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
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    if (!io.WantCaptureKeyboard || !io.WantCaptureMouse)
    {
        for (int x = 0; x < key_events.size(); x++)
        {
            if (key_events.at(x).type == "keyup")
            {
                if (key_events.at(x).key == string(glfwGetKeyName(key, scancode)))
                {
                    if (key_events.at(x).callback != NULL)
                    {
                        key_events.at(x).callback({});
                    }
                }
            }
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
    glfwMakeContextCurrent(core->window);
    glfwSwapInterval(1); // Enable vsync
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    ImGui::StyleColorsLight();
    //ImGui::StyleColorsClassic();
    ImGui_ImplGlfw_InitForOpenGL(core->window, true);
    ImGui_ImplOpenGL2_Init();
    if (core->data["show_cursor"] == false)
    {
        glfwSetInputMode(core->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
    Xrender_tick();
	return success;
}
void call_mouse_callback(Xrender_object_t* o, nlohmann::json matrix_data, double mouseX, double mouseY, std::string event) 
{
    if (o->mouse_callback != NULL)
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
    }
}
void render_arc(double cx, double cy, double radius, double start_angle, double end_angle)
{
    if ( (((MAX(start_angle, end_angle) - MIN(start_angle, end_angle)) / 360.0f) * (2 * 3.1415926f * radius)) > 8)
    {
        double num_segments = 10;
        double_point_t start;
        double_point_t sweeper;
        double_point_t end;
        double_point_t last_point;
        start.x = cx + (radius * cosf((start_angle) * 3.1415926f / 180.0f));
        start.y = cy + (radius * sinf((start_angle) * 3.1415926f / 180.0f));
        end.x = cx + (radius * cosf((end_angle) * 3.1415926f / 180.0f));
        end.y = cy + (radius * sinf((end_angle) * 3.1415926 / 180.0f));
        double diff = MAX(start_angle, end_angle) - MIN(start_angle, end_angle);
        if (diff > 180) diff = 360 - diff;
        double angle_increment = diff / num_segments;
        double angle_pointer = start_angle + angle_increment;
        last_point = start;
        for (int i = 0; i < num_segments; i++)
        {
            sweeper.x = cx + (radius * cosf((angle_pointer) * 3.1415926f / 180.0f));
            sweeper.y = cy + (radius * sinf((angle_pointer) * 3.1415926f / 180.0f));
            angle_pointer += angle_increment;
            glBegin(GL_LINES);
                glVertex3f(last_point.x, last_point.y, 0);
                glVertex3f(sweeper.x, sweeper.y, 0);
            glEnd();
            last_point = sweeper;
        }
        //aalineRGBA(core->gRenderer, last_point.x, (double)core->data["window_height"] - last_point.y, end.x, (double)core->data["window_height"] - end.y, r, g, b, a);
        //SDL_RenderDrawLine(core->gRenderer, last_point.x, (double)core->data["window_height"] - last_point.y, end.x, (double)core->data["window_height"] - end.y);
        glBegin(GL_LINES);
            glVertex3f(last_point.x, last_point.y, 0);
            glVertex3f(end.x, end.y, 0);
        glEnd();
    }
    else
    {
        double_point_t start;
        double_point_t end;
        start.x = cx + (radius * cosf((start_angle) * 3.1415926f / 180.0f));
        start.y = cy + (radius * sinf((start_angle) * 3.1415926f / 180.0f));
        end.x = cx + (radius * cosf((end_angle) * 3.1415926f / 180.0f));
        end.y = cy + (radius * sinf((end_angle) * 3.1415926 / 180.0f));
        glBegin(GL_LINES);
            glVertex3f(start.x, start.y, 0);
            glVertex3f(end.x, end.y, 0);
        glEnd();
    }
}
double_point_t Xrender_get_current_mouse_position()
{
    double mouseX, mouseY;
    glfwGetCursorPos(core->window, &mouseX, &mouseY);
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
        return lhs->data["zindex"] < rhs->data["zindex"];
    });
    for (int x = 0; x < object_stack.size(); x++)
    {
        if (object_stack[x]->data["visable"] == true) //Texture re-gen
        {
            /*if (object_stack[x]->texture == NULL) //We need to render the texture!
            {
                if (object_stack[x]->data["type"] == "text")
                {
                    TTF_Font* f;
                    if (object_stack[x]->data["font"] == "default")
                    {
                        f = TTF_OpenFontRW(SDL_RWFromConstMem(Sans_ttf,sizeof(Sans_ttf)), 1, object_stack[x]->data["font_size"]);
                    }
                    else
                    {
                        f = TTF_OpenFont(string(object_stack[x]->data["font"]).c_str(), object_stack[x]->data["font_size"]);
                    }
                    if (f)
                    {
                        SDL_Color color = {object_stack[x]->data["color"]["r"], object_stack[x]->data["color"]["g"], object_stack[x]->data["color"]["b"]};
                        SDL_Surface* surfaceMessage = TTF_RenderText_Solid(f, string(object_stack[x]->data["textval"]).c_str(), color);
                        //SDL_DestroyTexture(ObjectStack[x].texture);
                        object_stack[x]->texture = SDL_CreateTextureFromSurface(core->gRenderer, surfaceMessage);
                        SDL_FreeSurface(surfaceMessage);
                        TTF_CloseFont(f);
                    }
                    else
                    {
                        printf("Could not render text!\n");
                    }
                }
                if (object_stack[x]->data["type"] == "image")
                {
                    SDL_Surface* loadedSurface = IMG_Load(string(object_stack[x]->data["path"]).c_str());
                    if( loadedSurface == NULL )
                    {
                        printf( "Unable to load image %s! SDL_image Error: %s\n", string(object_stack[x]->data["path"]).c_str(), IMG_GetError() );
                    }
                    else
                    {
                        object_stack[x]->texture = SDL_CreateTextureFromSurface( core->gRenderer, loadedSurface );
                        if( object_stack[x]->texture == NULL )
                        {
                            printf( "Unable to create texture from %s! SDL Error: %s\n", string(object_stack[x]->data["path"]).c_str(), SDL_GetError() );
                        }
                        SDL_FreeSurface( loadedSurface );
                    }
                }
            }*/
        }
        if (object_stack[x]->data["visable"] == true)
        {
            //printf("(Rendering %d) %s\n", x, object_stack[x]->data.dump().c_str());
            nlohmann::json data;
            if (object_stack[x]->matrix_data == NULL)
            {
                data = object_stack[x]->data;
            }
            else
            {
                data = object_stack[x]->matrix_data(object_stack[x]->data);
            }
            if (object_stack[x]->data["type"] == "line")
            {
                if (object_stack[x]->mouse_callback != NULL && mouse_check_skip_cycles == MOUSE_CHECK_CYCLE)
                {
                    if (g.line_intersects_with_circle({{(double)data["start"]["x"], (double)data["start"]["y"]}, {(double)data["end"]["x"], (double)data["end"]["y"]}}, {(double)m.x, (double)m.y}, 10))
                    {
                        if (object_stack[x]->data["mouse_over"] == false)
                        {
                            call_mouse_callback(object_stack[x], data, m.x, m.y, "mouse_in");
                            object_stack[x]->data["mouse_over"] = true;
                        }    
                    }
                    else
                    {
                        if (object_stack[x]->data["mouse_over"] == true)
                        {
                            call_mouse_callback(object_stack[x], data, m.x, m.y, "mouse_out");
                            object_stack[x]->data["mouse_over"] = false;
                        }
                    }
                }
                if (object_stack[x]->data["width"] == 1)
                {
                    //aalineRGBA(core->gRenderer, (double)data["start"]["x"], (double)core->data["window_height"] - (double)data["start"]["y"], (double)data["end"]["x"], (double)core->data["window_height"] - (double)data["end"]["y"], (double)data["color"]["r"], data["color"]["g"], data["color"]["b"], data["color"]["a"]);
                    //SDL_SetRenderDrawColor(core->gRenderer, data["color"]["r"], data["color"]["g"], data["color"]["b"], data["color"]["a"]);
                    //SDL_RenderDrawLine(core->gRenderer, (double)data["start"]["x"], (double)core->data["window_height"] - (double)data["start"]["y"], (double)data["end"]["x"], (double)core->data["window_height"] - (double)data["end"]["y"]);
                    glColor4f((float)data["color"]["r"] / 255, (float)data["color"]["g"] / 255, (float)data["color"]["b"] / 255, (float)data["color"]["a"] / 255);
                    glBegin(GL_LINES);
                        glVertex3f((double)data["start"]["x"], (double)data["start"]["y"], 0);
                        glVertex3f((double)data["end"]["x"], (double)data["end"]["y"], 0);
                    glEnd();
                }
                else
                {
                    //thickLineRGBA(core->gRenderer, (double)data["start"]["x"], (double)core->data["window_height"] - (double)data["start"]["y"], (double)data["end"]["x"], (double)core->data["window_height"] -  (double)data["end"]["y"], data["width"], data["color"]["r"], data["color"]["g"], data["color"]["b"], data["color"]["a"]);
                }
            }
            else if (object_stack[x]->data["type"] == "arc")
            {
                if (object_stack[x]->mouse_callback != NULL && mouse_check_skip_cycles == MOUSE_CHECK_CYCLE)
                {
                    if (g.distance({(double)data["center"]["x"], (double)data["center"]["y"]}, {(double)m.x, (double)m.y}) > ((double)data["radius"] - 5) &&
                    g.distance({(double)data["center"]["x"], (double)data["center"]["y"]}, {(double)m.x, (double)m.y}) < ((double)data["radius"] + 5) &&
                    g.lines_intersect({g.create_polar_line({(double)data["center"]["x"], (double)data["center"]["y"]}, (double)data["start_angle"], (double)data["radius"]).end, g.create_polar_line({(double)data["center"]["x"], (double)data["center"]["y"]}, (double)data["end_angle"], (double)data["radius"]).end}, {{(double)data["center"]["x"], (double)data["center"]["y"]}, {(double)m.x, (double)m.y}}))
                    {
                        if (object_stack[x]->data["mouse_over"] == false)
                        {
                            call_mouse_callback(object_stack[x], data, m.x, m.y, "mouse_in");
                            object_stack[x]->data["mouse_over"] = true;
                        }
                    }
                    else
                    {
                        if (object_stack[x]->data["mouse_over"] == true)
                        {
                            call_mouse_callback(object_stack[x], data, m.x, m.y, "mouse_out");
                            object_stack[x]->data["mouse_over"] = false;
                        }
                    }
                    //printf("start_angle: %.4f, end_angle: %.4f\n", (double)data["start_angle"], (double)data["end_angle"]);
                }
                //arcRGBA(core->gRenderer, (double)data["center"]["x"], (double)data["center"]["y"], (double)data["radius"], (double)data["start_angle"], (double)data["end_angle"], data["color"]["r"], data["color"]["g"], data["color"]["b"], data["color"]["a"]);
                glColor4f((float)data["color"]["r"] / 255, (float)data["color"]["g"] / 255, (float)data["color"]["b"] / 255, (float)data["color"]["a"] / 255);
                render_arc((double)data["center"]["x"], (double)data["center"]["y"], (double)data["radius"], (double)data["start_angle"], (double)data["end_angle"]);
            }
            else if (object_stack[x]->data["type"] == "circle")
            {
                if (object_stack[x]->mouse_callback != NULL && mouse_check_skip_cycles == MOUSE_CHECK_CYCLE)
                {
                    if (g.distance({(double)data["center"]["x"], (double)data["center"]["y"]}, {(double)m.x, (double)m.y}) > ((double)data["radius"] - 5) &&
                    g.distance({(double)data["center"]["x"], (double)data["center"]["y"]}, {(double)m.x, (double)m.y}) < ((double)data["radius"] + 5))
                    {
                        if (object_stack[x]->data["mouse_over"] == false)
                        {
                            call_mouse_callback(object_stack[x], data, m.x, m.y, "mouse_in");
                            object_stack[x]->data["mouse_over"] = true;
                        }
                    }
                    else
                    {
                        if (object_stack[x]->data["mouse_over"] == true)
                        {
                            call_mouse_callback(object_stack[x], data, m.x, m.y, "mouse_out");
                            object_stack[x]->data["mouse_over"] = false;
                        }
                    }
                }
                //aacircleRGBA(core->gRenderer, (double)data["center"]["x"], (double)core->data["window_height"] - (double)data["center"]["y"], (double)data["radius"], data["color"]["r"], data["color"]["g"], data["color"]["b"], data["color"]["a"]);
            }
            else if (object_stack[x]->data["type"] == "box")
            {
                if (object_stack[x]->mouse_callback != NULL && mouse_check_skip_cycles == MOUSE_CHECK_CYCLE)
                {
                    if (m.x > (int)data["tl"]["x"] && m.x < (int)data["br"]["x"] && m.y > (int)data["tl"]["y"] && m.y < (int)data["br"]["y"])
                    {
                        if (object_stack[x]->data["mouse_over"] == false)
                        {
                            call_mouse_callback(object_stack[x], data, m.x, m.y, "mouse_in");
                            object_stack[x]->data["mouse_over"] = true;
                        }
                    }
                    else
                    {
                        if (object_stack[x]->data["mouse_over"] == true)
                        {
                            call_mouse_callback(object_stack[x], data, m.x, m.y, "mouse_out");
                            object_stack[x]->data["mouse_over"] = false;
                        }
                    }
                }
                //roundedBoxRGBA(core->gRenderer, (double)data["tl"]["x"], (double)core->data["window_height"] - (double)data["tl"]["y"], (double)data["br"]["x"], (double)core->data["window_height"] - (double)data["br"]["y"], (double)data["corner_radius"], data["color"]["r"], data["color"]["g"], data["color"]["b"], data["color"]["a"]);
            }
            else
            {
                /*dst.x = object_stack[x]->data["position"]["x"];
                dst.y = (int)core->data["window_height"] - (int)object_stack[x]->data["position"]["y"];
                if (object_stack[x]->data["size"]["width"] > 0 && object_stack[x]->data["size"]["height"] > 0)
                {
                    dst.w = object_stack[x]->data["size"]["width"];
                    dst.h = object_stack[x]->data["size"]["height"];
                }
                else
                {
                    SDL_QueryTexture(object_stack[x]->texture, NULL, NULL, &dst.w, &dst.h);
                    object_stack[x]->data["size"]["width"] = dst.w;
                    object_stack[x]->data["size"]["height"] = dst.h;
                }
                dst.y -= (int)object_stack[x]->data["size"]["height"];
                SDL_SetTextureBlendMode( object_stack[x]->texture, SDL_BLENDMODE_BLEND );
                SDL_SetTextureAlphaMod( object_stack[x]->texture, object_stack[x]->data["color"]["a"] );
                SDL_RenderCopyEx( core->gRenderer, object_stack[x]->texture, NULL, &dst, object_stack[x]->data["angle"], NULL, SDL_FLIP_NONE);*/
            }
        }
    }
    ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
    glfwMakeContextCurrent(core->window);
    glfwSwapBuffers(core->window);
    glfwPollEvents();

    if (mouse_check_skip_cycles > MOUSE_CHECK_CYCLE)
    {
        mouse_check_skip_cycles = 0;
    }
    mouse_check_skip_cycles++;
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
        
    }
    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(core->window);
    glfwTerminate();
    delete core;
}

Xrender_gui_t *Xrender_push_gui(bool visable, void (*callback)())
{
    Xrender_gui_t *g = new Xrender_gui_t;
    g->visable = visable;
    g->callback = callback;
    gui_stack.push_back(g);
    return g;
}
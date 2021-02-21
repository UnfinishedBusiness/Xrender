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
#define MOUSE_CHECK_CYCLE 1

vector<Xrender_key_event_t> key_events;
vector<Xrender_object_t*> object_stack;
vector<Xrender_timer_t> timers;
vector<Xrender_gui_t*> gui_stack;

vector<Xrender_object_t*> *Xrender_get_object_stack()
{
    return &object_stack;
}

void Xrender_RenderFont(float pos_x, float pos_y, std::string text, Xrender_object_t *o)
{
    double width, height = 0;
    glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, o->texture);
        glPushMatrix();
            glColor4f((double)o->data["color"]["r"] / 255, (double)o->data["color"]["g"] / 255, (double)o->data["color"]["b"] / 255, (double)o->data["color"]["a"] / 255);
            glRotatef((double)o->data["angle"], 0.0, 0.0, 1.0);
            glBegin(GL_QUADS);
            for (int x = 0; x < text.size(); x++)
            {
                if (text[x] >=32 && text[x] < 128)
                {
                    stbtt_aligned_quad q;
                    stbtt_GetBakedQuad(o->cdata, 512,512, text[x]-32, &pos_x,&pos_y,&q,1);
                    glTexCoord2f(q.s0,q.t1); glVertex2f(q.x0,-q.y1);
                    glTexCoord2f(q.s1,q.t1); glVertex2f(q.x1,-q.y1);
                    glTexCoord2f(q.s1,q.t0); glVertex2f(q.x1,-q.y0);
                    glTexCoord2f(q.s0,q.t0); glVertex2f(q.x0,-q.y0);
                    width += MAX(q.x0, q.x1) - MIN(q.x0, q.x1);
                    if ((MAX(q.y0, q.y1) - MIN(q.y0, q.y1)) > height) height = (MAX(q.y0, q.y1) - MIN(q.y0, q.y1));
                }
            }
            glEnd();
        glPopMatrix();
    glDisable(GL_TEXTURE_2D);
    glFlush();
    if ((double)o->data["size"]["width"] == 0 || (double)o->data["size"]["height"] == 0)
    {
        o->data["size"] = {{"width", width}, {"height", height}};
    }
}

bool Xrender_InitFontFromFile(const char* filename, int font_size, Xrender_object_t *o)
{
    unsigned char temp_bitmap[512*512];
    unsigned char ttf_buffer[1<<20];
    FILE *fp;
    if (string(filename) == "default")
    {
        for (int x = 0; x < (1<<20); x++)
        {
            ttf_buffer[x] = Sans_ttf[x];
        }
    }
    else
    {
        fp = fopen(string(filename).c_str(), "rb");
        if (fp)
        {
            fread(ttf_buffer, 1, 1<<20, fp);
        }
        else
        {
            return false;
        }
    }
    stbtt_BakeFontBitmap(ttf_buffer,0, (float)font_size, temp_bitmap ,512,512, 32,96, o->cdata); // no guarantee this fits!
    glGenTextures(1, &o->texture);
    glBindTexture(GL_TEXTURE_2D, o->texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 512,512, 0, GL_ALPHA, GL_UNSIGNED_BYTE, temp_bitmap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    fclose(fp);
    return true;
}

bool Xrender_ImageToTextureFromFile(const char* filename, GLuint* out_texture, int* out_width, int* out_height)
{
    // Load from file
    int image_width = 0;
    int image_height = 0;
    unsigned char* image_data = stbi_load(filename, &image_width, &image_height, NULL, 4);
    if (image_data == NULL)
        return false;

    // Create a OpenGL texture identifier
    GLuint image_texture;
    glGenTextures(1, &image_texture);
    glBindTexture(GL_TEXTURE_2D, image_texture);

    // Setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same

    // Upload pixels into texture
    #if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    #endif
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
    stbi_image_free(image_data);

    *out_texture = image_texture;
    *out_width = image_width;
    *out_height = image_height;

    return true;
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
void render_convex_polygon(vector<double_point_t> points)
{
    if (points.size() > 2)
    {
        for (int x = 2; x < points.size(); x+= 2)
        {
            glBegin(GL_TRIANGLES);
                glVertex2f(points[0].x, points[0].y);
                glVertex2f(points[x-1].x, points[x-1].y);
                glVertex2f(points[x].x, points[x].y);
            glEnd();
        }
        glBegin(GL_TRIANGLES);
            glVertex2f(points[0].x, points[0].y);
            glVertex2f(points[points.size() -2].x, points[points.size() -2].y);
            glVertex2f(points[points.size() -1].x, points[points.size()-1].y);
        glEnd();
    }
}
void render_rectangle_with_radius(float x, float y, float width, float height, float radius = 0.0)
{
    #define ROUNDING_POINT_COUNT 8
    double_point_t top_left[ROUNDING_POINT_COUNT];
    double_point_t bottom_left[ROUNDING_POINT_COUNT];
    double_point_t top_right[ROUNDING_POINT_COUNT];
    double_point_t bottom_right[ROUNDING_POINT_COUNT];
    if( radius == 0.0 )
    {
        radius = min(width, height);
        radius *= 0.00001;
    }
    int i = 0;
    float x_offset, y_offset;
    float step = ( 2.0f * 3.14159265359 ) / (ROUNDING_POINT_COUNT * 4), angle = 0.0f;
    unsigned int index = 0, segment_count = ROUNDING_POINT_COUNT;
    double_point_t bottom_left_corner = { x + radius, y - height + radius }; 
    while( i != segment_count )
    {
        x_offset = cosf( angle );
        y_offset = sinf( angle );
        top_left[index].x = bottom_left_corner.x - (x_offset * radius);
        top_left[index].y = (height - (radius * 2.0f)) + bottom_left_corner.y - (y_offset * radius);
        top_right[index].x = (width - (radius * 2.0f)) + bottom_left_corner.x + (x_offset * radius);
        top_right[index].y = (height - (radius * 2.0f)) + bottom_left_corner.y -(y_offset * radius);
        bottom_right[index].x = (width - (radius * 2.0f)) + bottom_left_corner.x + (x_offset * radius);
        bottom_right[index].y = bottom_left_corner.y + (y_offset * radius);
        bottom_left[index].x = bottom_left_corner.x - (x_offset * radius);
        bottom_left[index].y = bottom_left_corner.y + (y_offset * radius);
        top_left[index].x = top_left[index].x;
        top_left[index].y = top_left[index].y;
        top_right[index].x = top_right[index].x;
        top_right[index].y = top_right[index].y;
        bottom_right[index].x = bottom_right[index].x;
        bottom_right[index].y = bottom_right[index].y;
        bottom_left[index].x = bottom_left[index].x;
        bottom_left[index].y = bottom_left[index].y;
        angle -= step;
        ++index;
        ++i;
    }
    glBegin( GL_TRIANGLE_STRIP );
    {
        for( i = segment_count - 1 ; i >= 0 ; i--)
        {
            glVertex2f( top_left[ i ].x, top_left[ i ].y );
            glVertex2f( top_right[ i ].x, top_right[ i ].y );
        }
        glVertex2f( top_right[ 0 ].x, top_right[ 0 ].y );
        glVertex2f( top_right[ 0 ].x, top_right[ 0 ].y );
        glVertex2f( top_right[ 0 ].x, top_right[ 0 ].y );
        glVertex2f( top_left[ 0 ].x, top_left[ 0 ].y );
        glVertex2f( bottom_right[ 0 ].x, bottom_right[ 0 ].y );
        glVertex2f( bottom_left[ 0 ].x, bottom_left[ 0 ].y );
        for( i = 0; i != segment_count ; i++ )
        {
            glVertex2f( bottom_right[ i ].x, bottom_right[ i ].y );    
            glVertex2f( bottom_left[ i ].x, bottom_left[ i ].y );                                    
        }    
    }
    glEnd();
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
        return lhs->data["zindex"] < rhs->data["zindex"];
    });
    for (int x = 0; x < object_stack.size(); x++)
    {
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
            if (object_stack[x]->data["type"] == "text")
            {
                if (object_stack[x]->texture == -1)
                {
                    bool ret = Xrender_InitFontFromFile(string(object_stack[x]->data["font"]).c_str(), (int)object_stack[x]->data["font_size"], object_stack[x]);
                    if (ret == false)
                    {
                        printf("Could not init font: %s\n", string(object_stack[x]->data["font"]).c_str());
                        object_stack[x]->texture = -1;
                    }
                }
                else
                {
                    Xrender_RenderFont((float)data["position"]["x"], -(float)data["position"]["y"], string(data["textval"]), object_stack[x]);
                }
            }
            if (object_stack[x]->data["type"] == "image")
            {
                if (object_stack[x]->texture == -1)
                {
                    int my_image_width = 0;
                    int my_image_height = 0;
                    GLuint my_image_texture = 0;
                    bool ret = Xrender_ImageToTextureFromFile(string(object_stack[x]->data["path"]).c_str(), &object_stack[x]->texture, &my_image_width, &my_image_height);
                    if (ret)
                    {
                        //printf("Loaded image %s, width: %d, height: %d\n", string(object_stack[x]->data["path"]).c_str(), my_image_width, my_image_height);
                        object_stack[x]->data["image_width"] = my_image_width;
                        object_stack[x]->data["image_height"] = my_image_height;
                        if (object_stack[x]->data["size"]["width"] == 0 && object_stack[x]->data["size"]["height"] == 0)
                        {
                            object_stack[x]->data["size"]["width"] = my_image_width;
                            object_stack[x]->data["size"]["height"] = my_image_height;
                        }
                    }
                    else
                    {
                        printf("Could not load image!\n");
                        object_stack[x]->texture = -1;
                    }
                }
                else
                {
                    glColor4f((double)object_stack[x]->data["color"]["a"] / 255, (double)object_stack[x]->data["color"]["a"] / 255, (double)object_stack[x]->data["color"]["a"] / 255, (double)object_stack[x]->data["color"]["a"] / 255);
                    glPushMatrix();
                        glTranslatef((double)data["position"]["x"], (double)data["position"]["y"], 0.0);
                        glRotatef((double)object_stack[x]->data["angle"], 0.0, 0.0, 1.0);
                        glScalef(1.0f, -1.0f, 1.0f);
                        double imgWidth = (double)data["size"]["width"];
                        double imgHeight = (double)data["size"]["height"];
                        glBindTexture(GL_TEXTURE_2D, object_stack[x]->texture);
                        glEnable(GL_TEXTURE_2D);
                            glBegin(GL_QUADS);
                                glTexCoord2f(0, 0); glVertex2f(-imgWidth, -imgHeight);
                                glTexCoord2f(1, 0); glVertex2f(imgWidth, -imgHeight);
                                glTexCoord2f(1,1);  glVertex2f(imgWidth, imgHeight);
                                glTexCoord2f(0, 1); glVertex2f(-imgWidth, imgHeight);
                            glEnd();
                        glDisable(GL_TEXTURE_2D);
                        glFlush();
                    glPopMatrix();
                }
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
                glColor4f((float)data["color"]["r"] / 255, (float)data["color"]["g"] / 255, (float)data["color"]["b"] / 255, (float)data["color"]["a"] / 255);
                glLineWidth((float)data["width"]);
                if (data["style"] == "dashed")
                {
                    glPushAttrib(GL_ENABLE_BIT);
                    glLineStipple(10, 0xAAAA);
                    glEnable(GL_LINE_STIPPLE);
                }
                glBegin(GL_LINES);
                    glVertex3f((double)data["start"]["x"], (double)data["start"]["y"], 0);
                    glVertex3f((double)data["end"]["x"], (double)data["end"]["y"], 0);
                glEnd();
                glLineWidth(1);
                glDisable(GL_LINE_STIPPLE);
            }
            else if (object_stack[x]->data["type"] == "path")
            {
                if (object_stack[x]->mouse_callback != NULL && mouse_check_skip_cycles == MOUSE_CHECK_CYCLE)
                {
                    /*if (g.line_intersects_with_circle({{(double)data["start"]["x"], (double)data["start"]["y"]}, {(double)data["end"]["x"], (double)data["end"]["y"]}}, {(double)m.x, (double)m.y}, 10))
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
                    }*/
                }
                glColor4f((float)data["color"]["r"] / 255, (float)data["color"]["g"] / 255, (float)data["color"]["b"] / 255, (float)data["color"]["a"] / 255);
                glLineWidth((float)data["width"]);
                if (data["style"] == "dashed")
                {
                    glPushAttrib(GL_ENABLE_BIT);
                    glLineStipple(10, 0xAAAA);
                    glEnable(GL_LINE_STIPPLE);
                }
                if ((bool)data["closed"] == true)
                {
                    glBegin(GL_LINE_LOOP);
                }
                else
                {
                    glBegin(GL_LINES);
                }
                    for (int i = 0; i < data["points"].size(); i++)
                    {
                        glVertex3f((double)data["points"][i]["x"], (double)data["points"][i]["y"], 0);
                    }
                glEnd();
                glLineWidth(1);
                glDisable(GL_LINE_STIPPLE);
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
                }
                glColor4f((float)data["color"]["r"] / 255, (float)data["color"]["g"] / 255, (float)data["color"]["b"] / 255, (float)data["color"]["a"] / 255);
                glLineWidth((float)data["width"]);
                if (data["style"] == "dashed")
                {
                    glPushAttrib(GL_ENABLE_BIT);
                    glLineStipple(10, 0xAAAA);
                    glEnable(GL_LINE_STIPPLE);
                }
                render_arc((double)data["center"]["x"], (double)data["center"]["y"], (double)data["radius"], (double)data["start_angle"], (double)data["end_angle"]);
                glLineWidth(1);
                glDisable(GL_LINE_STIPPLE);
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
                glColor4f((float)data["color"]["r"] / 255, (float)data["color"]["g"] / 255, (float)data["color"]["b"] / 255, (float)data["color"]["a"] / 255);
                glLineWidth((float)data["width"]);
                if (data["style"] == "dashed")
                {
                    glPushAttrib(GL_ENABLE_BIT);
                    glLineStipple(10, 0xAAAA);
                    glEnable(GL_LINE_STIPPLE);
                }
                render_arc((double)data["center"]["x"], (double)data["center"]["y"], (double)data["radius"], 0, 180);
                render_arc((double)data["center"]["x"], (double)data["center"]["y"], (double)data["radius"], 180, 360);
                glLineWidth(1);
                glDisable(GL_LINE_STIPPLE);
            }
            else if (object_stack[x]->data["type"] == "box")
            {
                if (object_stack[x]->mouse_callback != NULL && mouse_check_skip_cycles == MOUSE_CHECK_CYCLE)
                {
                    if (m.x > (double)data["tl"]["x"] && m.x < (double)data["br"]["x"] && m.y > (double)data["br"]["y"] && m.y < (double)data["tl"]["y"])
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
                glColor4f((float)data["color"]["r"] / 255, (float)data["color"]["g"] / 255, (float)data["color"]["b"] / 255, (float)data["color"]["a"] / 255);
                render_rectangle_with_radius((double)data["tl"]["x"], (double)data["tl"]["y"], ((double)data["br"]["x"] - (double)data["tl"]["x"]), -((double)data["br"]["y"] - (double)data["tl"]["y"]), (double)data["radius"]);
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
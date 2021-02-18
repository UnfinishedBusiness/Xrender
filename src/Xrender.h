#ifndef XRENDER_
#define XRENDER_

#include <iostream>
#include <string>
#include <vector>
#include <json/json.h>
#include <gui/stb_truetype.h>

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif
#include <GLFW/glfw3.h>

using namespace std;

struct Xrender_core_t{
    nlohmann::json data;
    char *IniFileName;
    char *LogFileName;
    GLFWwindow* window;
};

struct Xrender_key_event_t{
    std::string key;
    std::string type; //keyup, keydown, scroll, mouse_click, mouse_move, window_resize
    void (*callback)(nlohmann::json);
};
struct Xrender_gui_t{
    bool visable;
    void (*callback)();
};
struct Xrender_timer_t{
    unsigned long timer;
    unsigned long intervol;
    bool (*callback)();
};
struct int_point_t{
    int x;
    int y;
};
struct double_point_t{
    double x;
    double y;
};
struct double_line_t{
    double_point_t start;
    double_point_t end;
};
struct Xrender_object_t{
    nlohmann::json data;
    GLuint texture;
    stbtt_bakedchar cdata[96];
    void (*mouse_callback)(Xrender_object_t*, nlohmann::json);
    nlohmann::json (*matrix_data)(nlohmann::json);
};

extern unsigned long tick_performance;
extern std::vector<Xrender_object_t*> object_stack;
extern std::vector<Xrender_key_event_t> key_events;
extern std::vector<Xrender_timer_t> timers;
extern std::vector<Xrender_gui_t*> gui_stack;

/*
    Renders font of specified text
*/
void Xrender_RenderFont(float pos_x, float pos_y, std::string text, Xrender_object_t *o);
/*
    Reads an ttf from file and creates a texture with the chars renderered to it
*/
bool Xrender_InitFontFromFile(const char* filename, int font_size, Xrender_object_t *o);

/*
    Reads an image from file and creates a texture
*/
bool Xrender_ImageToTextureFromFile(const char* filename, GLuint* out_texture, int* out_width, int* out_height);


/*
    Returns a pointer to the core variables
*/
Xrender_core_t *Xrender_get_core_variables();

/*
    Return an unsigned long of the number of milliseconds that have passed since the program has started
        used for timers and for performance measuring
*/
unsigned long Xrender_millis();

/*
    Blocks until ms time passes...
*/
void Xrender_delay(unsigned long ms);

/*
    Init SDL window and setup the library
        i["window_title"] = std::string of the desired window title
        i["window_width"] = int of the desired window width
        i["window_height"] = int of the desired window height
        i["show_cursor"] = bool: true if you want to see cursor and false if you want it hidden
        i["clear_color"]["r"] = uint8_t (0-255) red value
        i["clear_color"]["g"] = uint8_t (0-255) blue value
        i["clear_color"]["b"] = uint8_t (0-255) green value
        i["clear_color"]["a"] = uint8_t (0-255) alpha value
*/
bool Xrender_init(nlohmann::json i);


/*
    The program heartbeat. Handles all events and takes care of rendering
        returns false if quit event is handled
*/
bool Xrender_tick();

/*
    Query the current mouse position
*/
double_point_t Xrender_get_current_mouse_position();


/*
    Pushes a key event to the stack
        e.key = "X" - assci charactor
        e.type = "keyup" or "keydown"
        e.callback = void callback() - Function pointer that gets called uppon event
*/
void Xrender_push_key_event(Xrender_key_event_t e);

/*
    Pushes a new text object to the render stack
        text["textval"] = std::string of text that should be rendered !required!
        text["font_size"] = int value of desired text font size !required!
        text["font"] = ttf file path (defaults to ./Sans.ttf)
        text["position"]["x"] = x_position !required!
        text["position"]["y"] = y_position !required!
        text["color"]["r"] = uint8_t (0-255) red value
        text["color"]["g"] = uint8_t (0-255) blue value
        text["color"]["b"] = uint8_t (0-255) green value
        text["color"]["a"] = uint8_t (0-255) alpha value
*/
Xrender_object_t * Xrender_push_text(nlohmann::json text);

/*
    Pushes a new image object to the render stack
        image["path"] = std::string of image path !required!
        image["position"]["x"] = x_position !required!
        image["position"]["y"] = y_position !required!
        image["color"]["a"] = uint8_t (0-255) alpha value
*/
Xrender_object_t *Xrender_push_image(nlohmann::json image);

/*
    Pushes a new line object to the render stack
        line["start"]["x"] = Start X !required!
        line["start"]["y"] = Start Y !required!
        line["end"]["x"] = End X !required!
        line["end"]["y"] = End Y !required!
        line["width"] = line width - defaults to 1
        line["color"]["r"] = uint8_t (0-255) red value
        line["color"]["g"] = uint8_t (0-255) blue value
        line["color"]["b"] = uint8_t (0-255) green value
        line["color"]["a"] = uint8_t (0-255) alpha value
*/
Xrender_object_t *Xrender_push_line(nlohmann::json line);

/*
    Pushes a new path object to the render stack
        line["path"] = array of x and y points
        line["width"] = line width - defaults to 1
        line["color"]["r"] = uint8_t (0-255) red value
        line["color"]["g"] = uint8_t (0-255) blue value
        line["color"]["b"] = uint8_t (0-255) green value
        line["color"]["a"] = uint8_t (0-255) alpha value
*/
Xrender_object_t *Xrender_push_path(nlohmann::json path);

/*
    Pushes a new box object to the render stack
        box["tl"]["x"] = Top Left X !required!
        box["tl"]["y"] = Top Left Y !required!
        box["br"]["x"] = Bottom Right X !required!
        box["br"]["y"] = Bottom Right Y !required!
        box["corner_radius"] = Corner radius of box - defaults to 0
        box["color"]["r"] = uint8_t (0-255) red value
        box["color"]["g"] = uint8_t (0-255) blue value
        box["color"]["b"] = uint8_t (0-255) green value
        box["color"]["a"] = uint8_t (0-255) alpha value
*/
Xrender_object_t *Xrender_push_box(nlohmann::json box);

/*
    Pushes a new arc object to the render stack
        arc["center"]["x"] = Center X !required!
        arc["center"]["y"] = Center Y !required!
        arc["radius"] = Radius !required!
        arc["start_angle"] = start angle in degres !required!
        arc["end_angle"] = end angle in degrees !required!
        arc["color"]["r"] = uint8_t (0-255) red value
        arc["color"]["g"] = uint8_t (0-255) blue value
        arc["color"]["b"] = uint8_t (0-255) green value
        arc["color"]["a"] = uint8_t (0-255) alpha value
*/
Xrender_object_t *Xrender_push_arc(nlohmann::json arc);

/*
    Pushes a new circle object to the render stack
        circle["center"]["x"] = Center X !required!
        circle["center"]["y"] = Center Y !required!
        circle["radius"] = Radius !required!
        circle["color"]["r"] = uint8_t (0-255) red value
        circle["color"]["g"] = uint8_t (0-255) blue value
        circle["color"]["b"] = uint8_t (0-255) green value
        circle["color"]["a"] = uint8_t (0-255) alpha value
*/
Xrender_object_t *Xrender_push_circle(nlohmann::json circle);
/* End Object Creation */


/* Object Manipulation */
void Xrender_set_property(std::string, std::string, std::string);
/* End Object Manipulation */

/* GUI */
Xrender_gui_t *Xrender_push_gui(bool visable, void (*callback)());
/* End GUI */

/* Timers */
void Xrender_push_timer(unsigned long, bool (*)());
/* End Timers */

/* AppDirs & Environment */
string Xrender_get_env(const std::string &);
string Xrender_get_config_dir(string);
/* End AppDirs & Environment */

/* DXF File Handling */

/*
    Parses a DXF file and calls callback with individual entity information
        filename = path to the dxf file
        callback = callback defined as void cb(nlohmann::json, int, int)
*/
void Xrender_parse_dxf_file(string filename, void (*callback)(nlohmann::json, int, int));
/* End DXF File handling */

/* Debuging */
void Xrender_dump_object_stack(); //Debug the object stack
unsigned long Xrender_get_performance();
/* End Debugging */

void Xrender_rebuild_object(Xrender_object_t *o); //Flag an onbject for re-rendering
void Xrender_close(); //Close the library

/* Private function */
static void Xrender_mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
static void Xrender_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
static void window_close_callback(GLFWwindow* window);
static void glfw_error_callback(int error, const char* description);
static void call_mouse_callback(Xrender_object_t* o, nlohmann::json matrix_data, double mouseX, double mouseY, std::string event);
static void Xrender_scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
static void Xrender_cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
static void Xrender_window_size_callback(GLFWwindow* window, int width, int height);
#endif
#ifndef XRENDER_
#define XRENDER_

#include <iostream>
#include <string>
#include <vector>
#include <json/json.h>
#include <primatives/primatives.h>
#include <primatives/Line.h>


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

class Xrender_object_t{
    private:
        std::string type;
        Line *line_pointer;

    public:
        void process_mouse(float mpos_x, float mpos_y);
        void render();

        void *get_obj();

        Xrender_object_t(Line l)
        {
            line_pointer = &l;
            this->type = line_pointer->get_type_name();
        }
};

Xrender_object_t* Xrender_push_object(Line);

extern unsigned long tick_performance;
extern std::vector<Xrender_object_t*> object_stack;
extern std::vector<Xrender_key_event_t> key_events;
extern std::vector<Xrender_timer_t> timers;
extern std::vector<Xrender_gui_t*> gui_stack;


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

/*
    Returns pointer of object stack
*/
vector<Xrender_object_t*> *Xrender_get_object_stack();

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
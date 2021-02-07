#ifndef XRENDER_
#define XRENDER_

#include <iostream>
#include <string>
#include <vector>
#include <SDL.h>
#include <json/json.h>

using namespace std;

struct Xrender_object_t{
    std::string type;
    nlohmann::json data;
    SDL_Texture* texture;
    void (*click_callback)();
};
struct Xrender_key_event_t{
    std::string key;
    std::string type; //KEYUP, KEYDOWN
    void (*callback)();
};
struct Xrender_timer_t{
    unsigned long timer;
    unsigned long intervol;
    bool (*callback)();
};

struct dxf_point_t{
    double x;
    double y;
};
struct dxf_line_t{
    dxf_point_t start;
    dxf_point_t end;
};
struct dxf_arc_t{
    dxf_point_t center;
    double start_angle;
    double end_angle;
    double radius;
};
struct dxf_circle_t{
    dxf_point_t center;
    double radius;
};
struct dxf_object_t{
    std::string layer;
    std::string type;
    dxf_line_t line;
    dxf_arc_t arc;
    dxf_circle_t circle;
};
extern unsigned long tick_performance;
extern std::vector<Xrender_object_t*> object_stack;
extern std::vector<Xrender_key_event_t> key_events;
extern std::vector<Xrender_timer_t> timers;

/*
    Return an unsigned long of the number of milliseconds that have passed since the program has started
        used for timers and for performance measuring
*/
unsigned long Xrender_millis();

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
//Xrender_object_t *Xrender_push_image(string, string, SDL_Rect, int, int);
//Xrender_object_t *Xrender_push_line(string, SDL_Rect, SDL_Rect, int);
//Xrender_object_t *Xrender_push_box(string, SDL_Rect, SDL_Rect, int);
/* End Object Creation */


/* Object Manipulation */
void Xrender_set_property(std::string, std::string, std::string);
/* End Object Manipulation */

/* Timers */
void Xrender_push_timer(unsigned long, bool (*)());
/* End Timers */

/* AppDirs & Environment */
string Xrender_get_env(const std::string &);
string Xrender_get_config_dir(string);
/* End AppDirs & Environment */

/* DXF File Handling */
void Xrender_parse_dxf_file(string, void (*callback)(nlohmann::json, int, int));
/* End DXF File handling */

/* Debuging */
void Xrender_dump_object_stack(); //Debug the object stack
unsigned long Xrender_get_performance();
/* End Debugging */

void Xrender_rebuild_object(Xrender_object_t *o); //Flag an onbject for re-rendering
void Xrender_close(); //Close the library



#endif

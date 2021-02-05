#ifndef XRENDER_
#define XRENDER_

#include <iostream>
#include <string>
#include <vector>
#include <SDL.h>

using namespace std;

struct Xrender_color_t{
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
};
struct Xrender_text_object_t{
    std::string textval;
    int font_size;
    Xrender_color_t color;
};
struct Xrender_image_object_t{
    std::string path;
};
struct Xrender_line_object_t{
    SDL_Rect p1;
    SDL_Rect p2;
    int width;
    Xrender_color_t color;
};
struct Xrender_box_object_t{
    SDL_Rect p1;
    SDL_Rect p2;
    int radius;
    void (*click_callback)();
    Xrender_color_t color;
};
struct Xrender_object_t{
    std::string id_name;
    std::string group_name;
    std::string type;
    int zindex;
    bool visable;
    int opacity;
    float angle;
    SDL_Rect position;
    SDL_Rect size;
    SDL_Texture* texture;
    Xrender_text_object_t text;
    Xrender_image_object_t image;
    Xrender_line_object_t line;
    Xrender_box_object_t box;
};
struct Xrender_init_t{
    std::string window_title;
    int window_width;
    int window_height;
    Xrender_color_t clear_color;
};
struct Xrender_key_event_t{
    std::string key;
    std::string type; //KEYUP, KEYDOWN
    void (*callback)();
};
extern std::vector<Xrender_object_t*> object_stack;
extern std::vector<Xrender_key_event_t> key_events;

unsigned long Xrender_millis();
bool Xrender_init(Xrender_init_t); //Init the library
bool Xrender_tick(); //Poll events and respond to them
void Xrender_push_key_event(Xrender_key_event_t); //Push a key event to the event stack

/* Object Creation */
Xrender_object_t * Xrender_push_text(std::string, std::string, int, Xrender_color_t, SDL_Rect);
Xrender_object_t *Xrender_push_image(string, string, SDL_Rect, int, int);
Xrender_object_t *Xrender_push_line(string, SDL_Rect, SDL_Rect, int);
Xrender_object_t *Xrender_push_box(string, SDL_Rect, SDL_Rect, int);
/* End Object Creation */


/* Object Manipulation */
void Xrender_set_property(std::string, std::string, std::string);
/* End Object Manipulation */

void Xrender_rebuilt_object(Xrender_object_t *o); //Flag an onbject for re-rendering
void Xrender_close(); //Close the library
void Xrender_dump_object_stack(); //Debug the object stack

#endif
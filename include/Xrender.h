#ifndef XRENDER_
#define XRENDER_

#include <iostream>
#include <string>
#include <SDL.h>

using namespace std;

struct Xrender_color_t{
    int r;
    int g;
    int b;
    int a;
};
struct Xrender_text_object_t{
    std::string textval;
    int font_size;
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

bool Xrender_init(Xrender_init_t); //Init the library
bool Xrender_tick(); //Poll events and respond to them
void Xrender_push_key_event(Xrender_key_event_t); //Push a key event to the event stack

/* Object Creation */
Xrender_object_t * Xrender_push_text(std::string, std::string, int, Xrender_color_t, SDL_Rect);
/* End Object Creation */


/* Object Manipulation */
void Xrender_set_property(std::string, std::string, std::string);
/* End Object Manipulation */

void Xrender_rebuilt_object(Xrender_object_t *o); //Flag an onbject for re-rendering
void Xrender_close(); //Close the library
void Xrender_dump_object_stack(); //Debug the object stack

#endif
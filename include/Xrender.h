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

bool Xrender_init(Xrender_init_t);
bool Xrender_tick();
void Xrender_push_key_event(Xrender_key_event_t);
void Xrender_push_text(std::string, std::string, int, Xrender_color_t, SDL_Rect);
void Xrender_close();
void Xrender_dump_object_stack();

#endif
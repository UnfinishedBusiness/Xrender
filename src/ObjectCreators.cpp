#include <Xrender.h>


Xrender_object_t *Xrender_push_text(string id_name, string textval, int font_size, Xrender_color_t color, SDL_Rect position)
{
    Xrender_object_t *o = new Xrender_object_t;
    o->id_name = id_name;
    o->type = "TEXT";
    o->zindex = 0;
    o->visable = true;
    o->opacity = 255;
    o->position = position;
    o->size.w = 0;
    o->size.h = 0;
    o->angle = 0;
    o->text.textval = textval;
    o->text.font_size = font_size;
    o->text.color = color;
    o->texture = NULL;
    object_stack.push_back(o);
    return o;
}
Xrender_object_t *Xrender_push_image(string id_name, string path, SDL_Rect position, int width, int height)
{
    Xrender_object_t *o = new Xrender_object_t;
    o->id_name = id_name;
    o->type = "IMAGE";
    o->zindex = 0;
    o->visable = true;
    o->opacity = 255;
    o->position = position;
    o->size.w = width;
    o->size.h = height;
    o->image.path = path;
    o->angle = 0;
    o->texture = NULL;
    object_stack.push_back(o);
    return o;
}
Xrender_object_t *Xrender_push_line(string id_name, SDL_Rect p1, SDL_Rect p2, int width)
{
    Xrender_object_t *o = new Xrender_object_t;
    o->id_name = id_name; 
    o->type = "LINE";
    o->zindex = 0;
    o->visable = true;
    o->opacity = 255;
    o->position.x = 0;
    o->position.y = 0;
    o->size.w = 0;
    o->size.h = 0;
    o->line.p1.x = p1.x;
    o->line.p1.y = p1.y;
    o->line.p2.x = p2.x;
    o->line.p2.y = p2.y;
    o->line.width = width;
    o->line.color.r = 0;
    o->line.color.g = 0;
    o->line.color.b = 0;
    o->angle = 0;
    o->texture = NULL;
    object_stack.push_back(o);
    return o;
}
Xrender_object_t *Xrender_push_box(string id_name, SDL_Rect p1, SDL_Rect p2, int radius)
{
    Xrender_object_t *o = new Xrender_object_t;
    o->id_name = id_name; 
    o->type = "BOX";
    o->zindex = 0;
    o->visable = true;
    o->opacity = 255;
    o->position.x = 0;
    o->position.y = 0;
    o->size.w = 0;
    o->size.h = 0;
    o->box.p1.x = p1.x;
    o->box.p1.y = p1.y;
    o->box.p2.x = p2.x;
    o->box.p2.y = p2.y;
    o->box.radius = radius;
    o->angle = 0;
    o->texture = NULL;
    object_stack.push_back(o);
    return o;
}
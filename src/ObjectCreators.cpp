#include <Xrender.h>


Xrender_object_t *Xrender_push_text(nlohmann::json text)
{
    Xrender_object_t *o = new Xrender_object_t;
    o->type = "text";
    o->data = text;
    //printf("%s\n", text.dump().c_str());
    if (!o->data.contains("id"))
    {
        o->data["id"] = "none";
    }
    if (!o->data.contains("font"))
    {
        o->data["font"] = "./Sans.ttf";
    }
    if (!o->data.contains("zindex"))
    {
        o->data["zindex"] = 0;
    }
    if (!o->data.contains("angle"))
    {
        o->data["angle"] = 0;
    }
    if (!o->data.contains("visable"))
    {
        o->data["visable"] = true;
    }
    if (!o->data.contains("size"))
    {
        o->data["size"] = {
            {"width", 0}, 
            {"height", 0}
        };
    }
    if (!o->data.contains("color"))
    {
        o->data["color"] = {
            {"r", 0},
            {"g", 0},
            {"b", 0},
            {"a", 255}
        };
    }
    //printf("%s\n", o->data.dump().c_str());
    if (!o->data.contains("font_size") || !o->data.contains("textval") || !o->data.contains("position"))
    {
        return NULL;
    }
    if (!o->data["position"].contains("x") || !o->data["position"].contains("y"))
    {
        return NULL;
    }
    o->texture = NULL;
    object_stack.push_back(o);
    return o;
}
Xrender_object_t *Xrender_push_image(nlohmann::json image)
{
    Xrender_object_t *o = new Xrender_object_t;
    o->type = "image";
    o->data = image;
    if (!o->data.contains("id"))
    {
        o->data["id"] = "none";
    }
    if (!o->data.contains("zindex"))
    {
        o->data["zindex"] = 0;
    }
    if (!o->data.contains("angle"))
    {
        o->data["angle"] = 0;
    }
    if (!o->data.contains("visable"))
    {
        o->data["visable"] = true;
    }
    if (!o->data.contains("size"))
    {
        o->data["size"] = {
            {"width", 0}, 
            {"height", 0}
        };
    }
    if (!o->data.contains("color"))
    {
        o->data["color"] = {
            {"r", 0},
            {"g", 0},
            {"b", 0},
            {"a", 255}
        };
    }
    if (!o->data.contains("position") || !o->data.contains("path"))
    {
        return NULL;
    }
    if (!o->data["position"].contains("x") || !o->data["position"].contains("y"))
    {
        return NULL;
    }
    o->texture = NULL;
    object_stack.push_back(o);
    return o;
}
Xrender_object_t *Xrender_push_line(nlohmann::json line)
{
    Xrender_object_t *o = new Xrender_object_t;
    o->type = "line";
    o->data = line;
    if (!o->data.contains("id"))
    {
        o->data["id"] = "none";
    }
    if (!o->data.contains("zindex"))
    {
        o->data["zindex"] = 0;
    }
    if (!o->data.contains("angle"))
    {
        o->data["angle"] = 0;
    }
    if (!o->data.contains("visable"))
    {
        o->data["visable"] = true;
    }
    if (!o->data.contains("width"))
    {
        o->data["width"] = 1;
    }
    if (!o->data.contains("size"))
    {
        o->data["size"] = {
            {"width", 0}, 
            {"height", 0}
        };
    }
    if (!o->data.contains("color"))
    {
        o->data["color"] = {
            {"r", 0},
            {"g", 0},
            {"b", 0},
            {"a", 255}
        };
    }
    if (!o->data.contains("start") || !o->data.contains("end"))
    {
        return NULL;
    }
    if (!o->data["start"].contains("x") || !o->data["start"].contains("y"))
    {
        return NULL;
    }
    if (!o->data["end"].contains("x") || !o->data["end"].contains("y"))
    {
        return NULL;
    }
    o->texture = NULL;
    object_stack.push_back(o);
    return o;
}
/*Xrender_object_t *Xrender_push_box(string id_name, SDL_Rect p1, SDL_Rect p2, int radius)
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
}*/
void Xrender_rebuild_object(Xrender_object_t *o)
{
    SDL_DestroyTexture(o->texture);
    o->texture = NULL;
}
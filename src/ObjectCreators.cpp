#include <Xrender.h>


Xrender_object_t *Xrender_push_text(nlohmann::json text)
{
    Xrender_object_t *o = new Xrender_object_t;
    o->data = text;
    o->data["type"] = "text";
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
    ////printf("%s\n", o->data.dump().c_str());
    if (!o->data.contains("font_size") || !o->data.contains("textval") || !o->data.contains("position"))
    {
        return NULL;
    }
    if (!o->data["position"].contains("x") || !o->data["position"].contains("y"))
    {
        return NULL;
    }
    o->data["mouse_over"] = false;
    o->texture = NULL;
    o->matrix_data = NULL;
    object_stack.push_back(o);
    //printf("(Xrender_push_text) %s\n", o->data.dump().c_str());
    return o;
}
Xrender_object_t *Xrender_push_image(nlohmann::json image)
{
    Xrender_object_t *o = new Xrender_object_t;
    o->data = image;
    o->data["type"] = "image";
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
    o->data["mouse_over"] = false;
    o->texture = NULL;
    o->matrix_data = NULL;
    object_stack.push_back(o);
    //printf("(Xrender_push_image) %s\n", o->data.dump().c_str());
    return o;
}
Xrender_object_t *Xrender_push_line(nlohmann::json line)
{
    Xrender_object_t *o = new Xrender_object_t;
    o->data = line;
    o->data["type"] = "line";
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
    o->data["mouse_over"] = false;
    o->texture = NULL;
    o->matrix_data = NULL;
    object_stack.push_back(o);
    //printf("(Xrender_push_line) %s\n", o->data.dump().c_str());
    return o;
}
Xrender_object_t *Xrender_push_box(nlohmann::json box)
{
    Xrender_object_t *o = new Xrender_object_t;
    o->data = box;
    o->data["type"] = "box";
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
    if (!o->data.contains("corner_radius"))
    {
        o->data["corner_radius"] = 0;
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
    if (!o->data.contains("tl") || !o->data.contains("br"))
    {
        return NULL;
    }
    if (!o->data["tl"].contains("x") || !o->data["tl"].contains("y"))
    {
        return NULL;
    }
    if (!o->data["br"].contains("x") || !o->data["br"].contains("y"))
    {
        return NULL;
    }
    o->data["mouse_over"] = false;
    o->texture = NULL;
    o->matrix_data = NULL;
    object_stack.push_back(o);
    //printf("(Xrender_push_box) %s\n", o->data.dump().c_str());
    return o;
}
Xrender_object_t *Xrender_push_arc(nlohmann::json arc)
{
    Xrender_object_t *o = new Xrender_object_t;
    o->data = arc;
    o->data["type"] = "arc";
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
    if (!o->data.contains("center") || !o->data.contains("start_angle") || !o->data.contains("end_angle") || !o->data.contains("radius"))
    {
        return NULL;
    }
    if (!o->data["center"].contains("x") || !o->data["center"].contains("y"))
    {
        return NULL;
    }
    o->data["mouse_over"] = false;
    o->texture = NULL;
    o->matrix_data = NULL;
    object_stack.push_back(o);
    //printf("(Xrender_push_arc) %s\n", o->data.dump().c_str());
    return o;
}
Xrender_object_t *Xrender_push_circle(nlohmann::json circle)
{
    Xrender_object_t *o = new Xrender_object_t;
    o->data = circle;
    o->data["type"] = "circle";
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
    if (!o->data.contains("center") || !o->data.contains("radius"))
    {
        return NULL;
    }
    if (!o->data["center"].contains("x") || !o->data["center"].contains("y"))
    {
        return NULL;
    }
    o->data["mouse_over"] = false;
    o->texture = NULL;
    o->matrix_data = NULL;
    object_stack.push_back(o);
    return o;
}
void Xrender_rebuild_object(Xrender_object_t *o)
{
    SDL_DestroyTexture(o->texture);
    o->texture = NULL;
}
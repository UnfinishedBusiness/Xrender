#ifndef PRIMATIVES_
#define PRIMATIVES_

#include <Xrender.h>
#include <json/json.h>
#include <string>

struct int_point_t{
    int x;
    int y;
};
struct double_point_t{
    double x;
    double y;
    double z;
};
struct double_line_t{
    double_point_t start;
    double_point_t end;
};

/*
    Properties that all primatives will share
*/
class PrimativeProperties{
    public:
        bool visable;
        bool mouse_over;
        int zindex;
        float color[4];
        float offset[3];
        float scale;
        float angle;
        std::string id;
        nlohmann::json data;

        PrimativeProperties()
        {
            this->visable = true;
            this->zindex = 1;
            this->color[0] = 255;
            this->color[1] = 255;
            this->color[2] = 255;
            this->color[3] = 255;
            this->scale = 1;
            this->offset[0] = 0;
            this->offset[1] = 0;
            this->offset[2] = 0;
            this->angle = 0;
            this->id = "";
            this->mouse_over = false;
            this->data = NULL;
        }
};

#endif //PRIMATIVES_
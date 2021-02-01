#include "Xrender.h"

Xrender_object_t *text1;
Xrender_object_t *text2;

void on_a_press()
{
    //printf("A Key Pressed\n");
    Xrender_dump_object_stack();
    //text1->visable = !text1->visable;
}
void on_up_press()
{
    text1->text.font_size += 10;
    text1->angle += 10;
    Xrender_rebuilt_object(text1);
}
void on_down_press()
{
    text1->text.font_size -= 10;
    text1->angle -= 10;
    Xrender_rebuilt_object(text1);
}

int main()
{
    if (Xrender_init({"Test", 900, 700, {0, 0, 0, 0}}))
    {
        Xrender_push_key_event({"A", "KEYUP", &on_a_press});
        Xrender_push_key_event({"Up", "KEYUP", &on_up_press});
        Xrender_push_key_event({"Down", "KEYUP", &on_down_press});

        text1 = Xrender_push_text("test", "This is text", 30, {255, 255, 255}, {0, 0});
        text2 = Xrender_push_text("test1", "Second label", 30, {255, 255, 255}, {30, 30});
        while(Xrender_tick())
        {
            //Program is running
        }
    }
}
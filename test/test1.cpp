#include "Xrender.h"
#include "SDL.h"

Xrender_object_t *text1;

void on_a_press()
{
    //printf("A Key Pressed\n");
    Xrender_dump_object_stack();
}
void on_up_press()
{
    printf("UP Key Pressed\n");
    text1->visable = !text1->visable;
}

int main()
{
    if (Xrender_init({"Test", 900, 700, {0, 0, 0, 0}}))
    {
        Xrender_push_key_event({"A", "KEYUP", &on_a_press});
        Xrender_push_key_event({"Up", "KEYUP", &on_up_press});

        text1 = Xrender_push_text("test", "This is text", 30, {255, 255, 255}, {0, 0});
        while(Xrender_tick())
        {
            //Program is running
        }
    }
}
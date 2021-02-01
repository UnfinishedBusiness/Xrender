#include "Xrender.h"

Xrender_object_t *text1;
Xrender_object_t *text2;
unsigned long timer_one;

void on_a_press()
{
    //printf("A Key Pressed\n");
    Xrender_dump_object_stack();
    //text1->visable = !text1->visable;
}
void on_up_press()
{
    //text1->text.font_size += 10;
    text1->angle += 10;
    Xrender_rebuilt_object(text1);
}
void on_down_press()
{
    //text1->text.font_size -= 10;
    text1->angle -= 10;
    Xrender_rebuilt_object(text1);
}
void on_right_press()
{
    text2->position.x += 10;
    Xrender_rebuilt_object(text2);
}

int main()
{
    timer_one = Xrender_millis();
    if (Xrender_init({"Test", 900, 700, {100, 100, 100, 0}}))
    {
        Xrender_push_key_event({"A", "KEYUP", &on_a_press});
        Xrender_push_key_event({"Up", "KEYUP", &on_up_press});
        Xrender_push_key_event({"Down", "KEYUP", &on_down_press});
        Xrender_push_key_event({"Right", "KEYUP", &on_right_press});

        text1 = Xrender_push_text("test", "This is text", 30, {255, 255, 255}, {100, 100});
        text1->zindex = 10;
        text1->text.color = {255, 0, 0};
        text2 = Xrender_push_text("test1", "Second label", 30, {255, 255, 255}, {30, 30});
        //Xrender_push_image("background_image", "Background.png", {150,150}, 800, 400);

        Xrender_push_line("TestLine", {100, 100}, {200, 200}, 5);
        while(Xrender_tick())
        {
            if ((Xrender_millis() - timer_one) > 1000)
            {
                timer_one = Xrender_millis();
                printf("Timer one running!\n");
            }
        }
        Xrender_close();
    }
}
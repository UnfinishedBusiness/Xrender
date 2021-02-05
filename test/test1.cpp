#include "Xrender.h"

Xrender_object_t *text1;
Xrender_object_t *text2;
Xrender_object_t *line1;
Xrender_object_t *box;

int bar_value = 100;
Xrender_object_t *bar[100];

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
    line1->line.p1.x += 5;
    //Xrender_rebuilt_object(line1);
}
void on_right_press()
{
   bar_value++;
   if (bar_value > 100) bar_value = 100;
   for (int x = 0; x < 100; x++)
   {
       if (x < bar_value)
       {
           bar[x]->visable = true;
       }
       else
       {
           bar[x]->visable = false;
       }   
   }
}
void on_left_press()
{
    bar_value--;
   if (bar_value < 0) bar_value = 0;
   for (int x = 0; x < 100; x++)
   {
       if (x < bar_value)
       {
           bar[x]->visable = true;
       }
       else
       {
           bar[x]->visable = false;
       }   
   }
}

int main()
{
    if (Xrender_init({"Test", 900, 700, {100, 100, 100, 0}}))
    {
        Xrender_push_key_event({"A", "KEYUP", &on_a_press});
        Xrender_push_key_event({"Up", "KEYUP", &on_up_press});
        Xrender_push_key_event({"Down", "KEYUP", &on_down_press});
        Xrender_push_key_event({"Right", "KEYUP", &on_right_press});
        Xrender_push_key_event({"Left", "KEYUP", &on_left_press});

        text1 = Xrender_push_text("test", "This is text", 30, {255, 255, 255}, {100, 100});
        text1->zindex = 10;
        text1->text.color = {255, 0, 0};
        text2 = Xrender_push_text("test1", "Second label", 30, {255, 255, 255}, {30, 30});
        //Xrender_push_image("background_image", "Background.png", {150,150}, 800, 400);

        line1 = Xrender_push_line("TestLine", {100, 100}, {200, 200}, 5);
        //box = Xrender_push_box("TestBox", {100, 100}, {200, 200}, 5);
        //box->box.color.b = 200;

        int x_pos = 10;

        for (int x = 0; x < 100; x++)
        {
            x_pos += 3;
            bar[x] = Xrender_push_line("Line-" + to_string(x), {x_pos, 300}, {x_pos, 350}, 3);
        }
        Xrender_push_timer(100, on_down_press);
        while(Xrender_tick())
        {
            //Running!
        }
        Xrender_close();
    }
}
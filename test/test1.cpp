#include "Xrender.h"
#include "json/json.h"

Xrender_object_t *text1;
Xrender_object_t *text2;
Xrender_object_t *line1;
Xrender_object_t *box;

int bar_value = 200;
Xrender_object_t *bar[200];

void on_left_press()
{
    bar_value--;
   if (bar_value < 0) bar_value = 0;
   for (int x = 0; x < 200; x++)
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
void process_dxf(nlohmann::json o, int x, int n)
{
    int scale = 10;
    int xoffset = 200;
    int yoffset = 200;
    //printf("Handling object %d of %d\n", x, n);
    if (o["type"] == "line")
    {
        Xrender_object_t *l = Xrender_push_line(to_string(x), {int((double)o["start"]["x"] * scale) + xoffset, int((double)o["start"]["y"] * scale) + yoffset}, {int((double)o["end"]["x"] * scale) + xoffset, int((double)o["end"]["y"] * scale) + yoffset}, 1);
        l->line.color = {255, 0 , 0};
    }
    on_left_press();
    Xrender_tick(); //This will make sure progress bar gets updated
}
void on_a_press()
{
    //printf("A Key Pressed\n");
    //Xrender_dump_object_stack();
    //text1->visable = !text1->visable;
    Xrender_parse_dxf_file("test.dxf", process_dxf);
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

    text2->text.textval = to_string(Xrender_get_performance());
    Xrender_rebuilt_object(text2);
}
void on_right_press()
{
   bar_value++;
   if (bar_value > 200) bar_value = 200;
   for (int x = 0; x < 200; x++)
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
int test = 0;
bool test_timer()
{
    on_down_press();
    test++;
    if (test > 1000)
    {
        return false;
    }
    else
    {
        return true;
    }
}
int main()
{
    printf("App Config Dir = %s\n", Xrender_get_config_dir("test1").c_str());
    if (Xrender_init({{"window_title", "Test1"}}))
    {
        Xrender_push_key_event({"A", "keyup", on_a_press});
        Xrender_push_key_event({"Up", "keyup", on_up_press});
        Xrender_push_key_event({"Down", "keyup", on_down_press});
        Xrender_push_key_event({"Right", "keyup", on_right_press});
        Xrender_push_key_event({"Left", "keyup", on_left_press});

        text1 = Xrender_push_text("test", "This is text", 30, {255, 255, 255}, {100, 100});
        text1->zindex = 10;
        text1->text.color = {255, 0, 0};
        text2 = Xrender_push_text("test1", "Second label", 30, {255, 255, 255}, {30, 30});
        //Xrender_push_image("background_image", "Background.png", {150,150}, 800, 400);

        line1 = Xrender_push_line("TestLine", {100, 100}, {200, 200}, 5);
        //box = Xrender_push_box("TestBox", {100, 100}, {200, 200}, 5);
        //box->box.color.b = 200;

        int x_pos = 10;

        for (int x = 0; x < 200; x++)
        {
            x_pos += 3;
            bar[x] = Xrender_push_line("Line-" + to_string(x), {x_pos, 300}, {x_pos, 350}, 3);
        }
        Xrender_push_timer(100, test_timer);
        while(Xrender_tick())
        {
            //Running!
        }
        Xrender_close();
    }
}
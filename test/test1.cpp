#include "Xrender.h"
#include "json/json.h"

Xrender_object_t *text1;
Xrender_object_t *image1;
Xrender_object_t *line1;

bool test_timer()
{
    text1->data["angle"] = (int)text1->data["angle"] + 1;
    Xrender_rebuild_object(text1);

    image1->data["angle"] = (int)image1->data["angle"] + 1;
    Xrender_rebuild_object(image1);
    return true;
}
int main()
{
    printf("App Config Dir = %s\n", Xrender_get_config_dir("test1").c_str());
    if (Xrender_init({{"window_title", "Test1"}}))
    {
        text1 = Xrender_push_text({
            {"textval", "This is a test"},
            {"font_size", 100},
            {"zindex", 1},
            {"position", {
                {"x", 100},
                {"y", 200}
            }},
        });
        if (text1 == NULL)
        {
            printf("Object push missing required parameters!\n");
        }
        else
        {
            printf("%s\n", text1->data.dump().c_str());
        }

        image1 = Xrender_push_image({
            {"path", "Background.png"},
            {"position", {
                {"x", 0},
                {"y", 0}
            }},
        });
        if (image1 == NULL)
        {
            printf("Object push missing required parameters!\n");
        }
        else
        {
            printf("%s\n", image1->data.dump().c_str());
        }

        line1 = Xrender_push_line({
            {"start", {
                {"x", 0},
                {"y", 0}
            }},
            {"end", {
                {"x", 100},
                {"y", 100}
            }},
        });
        if (line1 == NULL)
        {
            printf("Object push missing required parameters!\n");
        }
        else
        {
            printf("%s\n", line1->data.dump().c_str());
        }
        Xrender_push_timer(10, test_timer);
        while(Xrender_tick())
        {
            //Running!
        }
        Xrender_close();
    }
    return 0;
}
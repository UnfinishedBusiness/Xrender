#include "Xrender.h"
#include "json/json.h"

Xrender_object_t *box;

bool test_timer()
{
    box->data["corner_radius"] = (int)box->data["corner_radius"] + 1;
    return true;
}
int main()
{
    printf("App Config Dir = %s\n", Xrender_get_config_dir("test1").c_str());
    if (Xrender_init({{"window_title", "Test1"}}))
    {
        box = Xrender_push_box({
            {"tl", {
                {"x", 100},
                {"y", 200}
            }},
            {"br", {
                {"x", 300},
                {"y", 400}
            }},
        });
        if (box == NULL)
        {
            printf("Object push missing required parameters!\n");
        }
        else
        {
            printf("%s\n", box->data.dump().c_str());
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
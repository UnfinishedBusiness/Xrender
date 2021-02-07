#include "Xrender.h"
#include "json/json.h"

Xrender_object_t *box;

bool test_timer()
{
    return true;
}
int main()
{
    printf("App Config Dir = %s\n", Xrender_get_config_dir("test1").c_str());
    if (Xrender_init({{"window_title", "Test1"}}))
    {
        box = Xrender_push_circle({
            {"center", {
                {"x", 450},
                {"y", 200}
            }},
            {"radius", 400},
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
#include "Xrender.h"
#include "json/json.h"

Xrender_object_t *text1;

bool test_timer()
{
    text1->data["angle"] = (int)text1->data["angle"] + 1;
    Xrender_rebuild_object(text1);
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
        Xrender_push_timer(10, test_timer);
        while(Xrender_tick())
        {
            //Running!
        }
        Xrender_close();
    }
    return 0;
}
#include "Xrender.h"
#include "json/json.h"

Xrender_object_t *box;

bool test_timer()
{
    return true;
}
void handle_dxf(nlohmann::json dxf, int x, int n)
{
    if (dxf["type"] == "line")
    {
        Xrender_push_line(dxf);
    }
    if (dxf["type"] == "arc")
    {
        Xrender_push_arc(dxf);
    }
    if (dxf["type"] == "circle")
    {
        Xrender_push_circle(dxf);
    }
}
int main()
{
    printf("App Config Dir = %s\n", Xrender_get_config_dir("test1").c_str());
    if (Xrender_init({{"window_title", "Test1"}}))
    {
        Xrender_parse_dxf_file("test.dxf", handle_dxf);
        Xrender_push_timer(10, test_timer);
        while(Xrender_tick())
        {
            //Running!
        }
        Xrender_close();
    }
    return 0;
}
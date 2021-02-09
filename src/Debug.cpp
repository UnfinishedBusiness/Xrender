#include <Xrender.h>

void Xrender_dump_object_stack()
{
    printf("Beginning stack dump:\n");
    for (int x = 0; x < object_stack.size(); x++)
    {
        if (object_stack[x]->data["type"] == "text")
        {
            printf("[Object %d](text) %s\n", x, object_stack[x]->data.dump().c_str());
        }
        if (object_stack[x]->data["type"] == "image")
        {
            printf("[Object %d](text) %s\n", x, object_stack[x]->data.dump().c_str());
        }
        if (object_stack[x]->data["type"] == "line")
        {
            printf("[Object %d](text) %s\n", x, object_stack[x]->data.dump().c_str());
        }
    }
    printf("End stack dump:\n");
}
unsigned long Xrender_get_performance()
{
    return tick_performance;
}
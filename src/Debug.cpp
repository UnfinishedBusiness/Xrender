#include <Xrender.h>

void Xrender_dump_object_stack()
{
    printf("Beginning stack dump:\n");
    for (int x = 0; x < object_stack.size(); x++)
    {
        if (object_stack[x]->type == "TEXT")
        {
            printf("[Object %d](TEXT)\n", x);
            printf("\tid_name=%s\n", object_stack[x]->id_name.c_str());
            printf("\tgroup_name=%s\n", object_stack[x]->group_name.c_str());
            printf("\tzindex=%d\n", object_stack[x]->zindex);
            printf("\tvisable=%s\n", object_stack[x]->visable ? "true" : "false");
            printf("\topacity=%d\n", object_stack[x]->opacity);
            printf("\tangle=%.4f\n", object_stack[x]->angle);
            printf("\ttextval=%s\n", object_stack[x]->text.textval.c_str());
        }
        if (object_stack[x]->type == "IMAGE")
        {
            printf("[Object %d](IMAGE)\n", x);
            printf("\tid_name=%s\n", object_stack[x]->id_name.c_str());
            printf("\tgroup_name=%s\n", object_stack[x]->group_name.c_str());
            printf("\tzindex=%d\n", object_stack[x]->zindex);
            printf("\tvisable=%s\n", object_stack[x]->visable ? "true" : "false");
            printf("\topacity=%d\n", object_stack[x]->opacity);
            printf("\tangle=%.4f\n", object_stack[x]->angle);
            printf("\tpath=%s\n", object_stack[x]->image.path.c_str());
        }
        if (object_stack[x]->type == "LINE")
        {
            printf("[Object %d](LINE)\n", x);
            printf("\tid_name=%s\n", object_stack[x]->id_name.c_str());
            printf("\tgroup_name=%s\n", object_stack[x]->group_name.c_str());
            printf("\tzindex=%d\n", object_stack[x]->zindex);
            printf("\tvisable=%s\n", object_stack[x]->visable ? "true" : "false");
            printf("\topacity=%d\n", object_stack[x]->opacity);
            printf("\tangle=%.4f\n", object_stack[x]->angle);
            printf("\tp1=(%d, %d)\n", object_stack[x]->line.p1.x, object_stack[x]->line.p1.y);
            printf("\tp2=(%d, %d)\n", object_stack[x]->line.p2.x, object_stack[x]->line.p2.y);
            printf("\twidth=%d\n", object_stack[x]->line.width);
        }
    }
    printf("End stack dump:\n");
}
unsigned long Xrender_get_performance()
{
    return tick_performance;
}
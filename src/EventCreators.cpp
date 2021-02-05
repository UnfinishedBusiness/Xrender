#include <Xrender.h>

void Xrender_push_key_event(Xrender_key_event_t k)
{
    key_events.push_back(k);
}
void Xrender_rebuilt_object(Xrender_object_t *o)
{
    SDL_DestroyTexture( o->texture );
    o->texture = NULL;
}
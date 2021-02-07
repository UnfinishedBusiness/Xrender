#include <Xrender.h>
#include <json/json.h>

void Xrender_push_key_event(Xrender_key_event_t e)
{
    key_events.push_back(e);
}
#include <Xrender.h>
#include <json/json.h>

void Xrender_push_key_event(Xrender_key_event_t e)
{
    key_events.push_back(e);
    if (e.type == "window_resize")
    {
        /*
            Window resize event should be called right away just once to make sure client app has propper dimensions for their gui layout!
        */
        Xrender_window_size_callback(core->window, (int)core->data["window_width"], (int)core->data["window_height"]);
    }
}
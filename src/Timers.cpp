#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <sys/time.h>
#include <ctime>
#include <algorithm> 
#include <Xrender.h>

void Xrender_push_timer(unsigned long intervol, void (*callback)())
{
    Xrender_timer_t t;
    t.timer = Xrender_millis();
    t.intervol = intervol;
    t.callback = callback;
    timers.push_back(t);
}
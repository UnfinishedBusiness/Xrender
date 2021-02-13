#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <sys/time.h>
#include <ctime>
#include <algorithm> 
#include <Xrender.h>


using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::seconds;
using std::chrono::system_clock;

auto program_start_time = chrono::steady_clock::now();

unsigned long Xrender_millis()
{
  auto end = chrono::steady_clock::now();
  unsigned long m = (unsigned long)chrono::duration_cast<chrono::milliseconds>(end - program_start_time).count();
  return m;
}
void Xrender_delay(unsigned long ms)
{
  unsigned long delay_timer = Xrender_millis();
  while((Xrender_millis() - delay_timer) < ms);
  return;
}
void Xrender_push_timer(unsigned long intervol, bool (*callback)())
{
    Xrender_timer_t t;
    t.timer = Xrender_millis();
    t.intervol = intervol;
    t.callback = callback;
    timers.push_back(t);
}
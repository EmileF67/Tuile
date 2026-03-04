#include "Apps/BarComponents/Module.h"


Module::Module(std::time_t refresh_time_, int size_)
    : refresh_time(static_cast<std::time_t>(refresh_time_)), last_drawn(static_cast<std::time_t>(0)), size(size_)
{

}


bool Module::should_be_drawn()
{
    std::time_t now = std::time(nullptr);
    if ((now - last_drawn) >= refresh_time) {
        last_drawn = now;
        return true;
    }
    return false;
}



void Module::set_win(WINDOW* win_)
{
    win = win_;
}
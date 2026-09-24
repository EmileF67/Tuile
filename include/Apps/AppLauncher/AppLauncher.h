#ifndef APPLAUNCHER_H
#define APPLAUNCHER_H

#include <string>
#include <vector>
#include <ncurses.h>
#include <algorithm>
#include "Apps/AppLauncher/Apps.h"
#include "Engine/Components/Cadre.h"


#define COLOR_UNSELECTED 5
#define COLOR_SELECTED 2


class AppLauncher {
    private :
        WINDOW* win;
        bool is_linux_console;
        std::size_t i_selected;
        std::size_t scroll_offset;
        std::vector<Apps> apps;
        Cadre cadre;

    public :
        AppLauncher(WINDOW* win_, bool is_linux_console_);
        void draw();
        void handle_key(int key);
        void launch(Apps app);
};



#endif // APPLAUNCHER_H
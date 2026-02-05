#ifndef MAINENGINE_H
#define MAINENGINE_H

#include <ncurses.h>
#include <string>
#include <vector>

class MainEngine {
private:
    WINDOW* stdscr;

    // ORDRE GARANTI
    std::vector<WINDOW*> windows;
    std::vector<std::string> window_names;

    int rows;
    int cols;

    bool bar;

    void update_layout();

public:
    explicit MainEngine(WINDOW* stdscr);
    ~MainEngine();

    WINDOW* new_window(const std::string& name);

    void refresh_all_and_update();

    bool detect_resizing();
};

#endif // MAINENGINE_H

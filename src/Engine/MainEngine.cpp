#include "Engine/MainEngine.h"
#include <ncurses.h>

#define MAX_DISPLAYED_WINDOWS 2

MainEngine::MainEngine(WINDOW* stdscr_)
    : stdscr(stdscr_), rows(0), cols(0), bar(true)
{
    getmaxyx(stdscr, rows, cols);
}

MainEngine::~MainEngine() {
    for (WINDOW* win : windows) {
        delwin(win);
    }
}

WINDOW* MainEngine::new_window(const std::string& name)
{
    if (windows.size() >= MAX_DISPLAYED_WINDOWS)
        return nullptr;

    // Mise à jour layout AVANT création
    update_layout();

    int h = rows;
    int w = cols;
    int y = 0;
    int x = 0;

    if (windows.size() == 1) {
        w = cols / 2;
        x = cols / 2;
    }

    if (bar) {
        h -= 2;
        y += 2;
    }

    WINDOW* win = newwin(h, w, y, x);
    windows.push_back(win);
    window_names.push_back(name);

    // Recalcul complet après ajout
    update_layout();

    return win;
}

void MainEngine::update_layout()
{
    if (windows.empty())
        return;

    getmaxyx(stdscr, rows, cols);

    for (size_t i = 0; i < windows.size(); ++i) {
        int new_h = rows;
        int new_w = cols;
        int new_y = 0;
        int new_x = 0;

        if (windows.size() == 2) {
            new_w = cols / 2;
            new_x = (i == 0) ? 0 : cols / 2;
        }

        if (bar) {
            new_h -= 2;
            new_y += 2;
        }

        wresize(windows[i], new_h, new_w);
        mvwin(windows[i], new_y, new_x);
        werase(windows[i]);
    }
}

void MainEngine::refresh_all_and_update()
{
    wnoutrefresh(stdscr);   // IMPORTANT
    for (WINDOW* win : windows) {
        wnoutrefresh(win);
    }
    doupdate();
}

bool MainEngine::detect_resizing()
{
    int old_rows = rows;
    int old_cols = cols;

    // OBLIGATOIRE avec ncurses
    resize_term(0, 0);
    getmaxyx(stdscr, rows, cols);

    if (rows != old_rows || cols != old_cols) {
        update_layout();
        return true;
    }
    return false;
}

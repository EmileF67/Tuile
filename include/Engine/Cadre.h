#ifndef CADRE_H
#define CADRE_H

#include <utility>
#include <ncurses.h>

class Cadre {
private:
    std::pair<int,int> x; // {row1, col1}
    std::pair<int,int> y; // {row2, col2}
    WINDOW* win;

public:
    Cadre(std::pair<int,int> x_, std::pair<int,int> y_, WINDOW* stdscr);
    void draw();
    void sep(int row);
};

#endif // CADRE_H

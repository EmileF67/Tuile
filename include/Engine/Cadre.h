#ifndef CADRE_H
#define CADRE_H

#include <utility>
#include <ncurses.h>


class Cadre {
    private:
        WINDOW* win;
        std::pair<int,int> x; // {row1, col1}
        std::pair<int,int> y; // {row2, col2}

        bool sharp;

    public:
        Cadre(WINDOW* stdscr, std::pair<int,int> x_, std::pair<int,int> y_, bool sharp_);
        void draw();
        void sep(int row);

};


#endif // CADRE_H

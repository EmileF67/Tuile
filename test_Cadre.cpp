#include <clocale>      // setlocale
#include <ncurses.h>
#include "Engine/Cadre.h"
#include <utility>
#include <iostream>

int main() {
    // active la locale UTF-8 du système (important pour afficher les caractères Unicode)
    setlocale(LC_ALL, "");

    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);

    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    // coins du cadre : {ligne, colonne}
    std::pair<int,int> topLeft {2, 4};
    std::pair<int,int> botRight {rows - 3, cols - 4};

    try {
        Cadre cadre(topLeft, botRight, stdscr);
        cadre.draw();
        int middle = (topLeft.first + botRight.first) / 2;
        cadre.sep(middle);
    } catch (const std::exception& e) {
        endwin();
        std::cerr << "Erreur: " << e.what() << "\n";
        return 1;
    }

    mvprintw(rows - 1, 2, "Appuyez sur une touche pour quitter...");
    refresh();
    getch();

    endwin();
    return 0;
}

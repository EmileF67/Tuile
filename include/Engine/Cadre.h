#ifndef CADRE_H
#define CADRE_H

#include <ncurses.h>
#include <string>
#include <stdexcept>
#include <utility>

class Cadre {
private:
    // Coordonnées du cadre
    std::pair<int, int> x; // {row1, col1}
    std::pair<int, int> y; // {row2, col2}
    WINDOW* win;           // Pointeur vers la fenêtre ncurses

public:
    // Constructeur
    Cadre(std::pair<int,int> x_, std::pair<int,int> y_, WINDOW* stdscr);

    // Dessine le cadre
    void draw();

    // Ajoute une séparation horizontale
    void sep(int row);
};

#endif // CADRE_H

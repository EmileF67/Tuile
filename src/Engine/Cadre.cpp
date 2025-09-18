#include <ncurses.h>
#include <string>
#include <stdexcept>




class Cadre {
// Cette classe permet de créer un cadre avec des bordures et éventuellement 
// des séparations horizontales.

private:
    // Déclarations
    std::pair<int, int> x; // {row1, col1}
    std::pair<int, int> y; // {row2, col2}
    WINDOW* win;           // Pointeur vers la fenêtre ncurses


public:
    // Constructeur
    Cadre(std::pair<int,int> x_, std::pair<int,int> y_, WINDOW* stdscr)
        : x(x_), y(y_), win(stdscr) {}


    // Méthode draw() : dessine le cadre
    void draw() {
        int r1 = x.first;
        int c1 = x.second;
        int r2 = y.first;
        int c2 = x.second;

        // Vérifications
        if (r2 - r1 < 1) throw std::runtime_error("Taille verticale trop petite");
        if (c2 - c1 < 1) throw std::runtime_error("Taille horizontale trop petite");

        // Ligne du haut
        mvwaddstr(win, r1, c1, ("╭" + std::string(c2 - c1 - 2, '─') + "╮").c_str());
        // Ligne du bas
        mvwaddstr(win, r2, c1, ("╰" + std::string(c2 - c1 - 2, '─') + "╯").c_str());

        // Colonnes verticales
        for (int r = r1 + 1 ; r < r2 ; r++) {
            mvwaddstr(win, r, c1, "│");
            mvwaddstr(win, r, c2 - 1, "│");
        }
    }


    // Méthode sep() : séparation horizontale
    void sep(int row) {
        mvwaddstr(win, row, x.second,
            ("├" + std::string(y.second - x.second - 2, '─') + "┤").c_str());
    }


};
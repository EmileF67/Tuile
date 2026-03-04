#ifndef BAR_H
#define BAR_H

#include <ncurses.h>
#include <vector>
#include <memory>
#include <utility>
#include "Engine/Components/Cadre.h"
#include "Apps/BarComponents/Module.h"


enum class BarArea {
    Left,
    Middle,
    Right
};


class Bar {
    private:
        // Va stocker les différents modules à afficher
        std::vector<std::unique_ptr<Module>> left;
        std::vector<std::unique_ptr<Module>> middle;
        std::vector<std::unique_ptr<Module>> right;
        
        // La fenêtre globale et sa taille
        WINDOW* stdscr;
        int rows, cols;
        bool is_linux_console;
        
        // La fenêtre dédiée à la barre
        WINDOW* win;

        // Le cadre qui représente la barre
        Cadre cadre;

    public:
        // Constructeur
        Bar(WINDOW* stdscr_, bool is_linux_console_);

        // Destructeur
        ~Bar();

        // Récupérer la window
        WINDOW* get_win() { return win; }

        // Permet d'ajouter un module à un des endroit disponible
        void ajout_module(std::unique_ptr<Module> module, BarArea area);

        // Affiche la barre
        void draw();


    private :
        // Afficher la partie gauche
        void draw_left();
        
        // Afficher la partie du milieu
        void draw_middle();

        // Afficher la partie droite
        void draw_right();
};





#endif // BAR_H
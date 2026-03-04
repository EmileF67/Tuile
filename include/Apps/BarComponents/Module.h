#ifndef MODULE_H
#define MODULE_H

#include <ctime>
#include <ncurses.h>

class Module {
    protected :
        WINDOW* win;
        std::time_t refresh_time;       // Temps d'attente en secondes
        std::time_t last_drawn;         // Timestamp

    public :
        int size;

        // Constructeur
        Module(std::time_t refresh_time_, int size_);

        // Donner une valeur à la fenêtre
        void set_win(WINDOW* win_);

        // Vérifie si le module doit être affiché car il a été réactualisé
        bool should_be_drawn();

        // Afficher le module
        virtual void draw(int x) = 0;

        // Destructeur virtuel
        virtual ~Module() = default;
};



#endif // MODULE_H
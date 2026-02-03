#include <ncurses.h>
#include <string>
#include <memory>
#include <vector>
#include "Engine/Cadre.h"

#ifndef MESSAGEBOX_H
#define MESSAGEBOX_H



class MessageBox {
    private:
        WINDOW* win;
        std::string message;
        bool is_linux_console;
        
        // std::pair<size_t, size_t> max_size;
        int rows, cols;
        int x1, y1, x2, y2;
        std::unique_ptr<Cadre> cadre;

        bool entered;


    public:
        // Constructeur    
        MessageBox(WINDOW* win_, const std::string& message_, bool is_linux_console_);

        // Affiche la message box
        void draw();

        // Gère la navigation clavier
        void handle_key(int key);

        // Permet de voir si on en a terminé avec la message box
        bool is_entered() const { return entered; }

    private :
        // Permet de séparer un texte std::string en tableau de std::string d'une taille définie.
        std::vector<std::string> split_text(size_t taille_max);
};



#endif // MESSAGEBOX_H
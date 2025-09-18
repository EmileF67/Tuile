#ifndef INPUT_H
#define INPUT_H

#include <ncurses.h>
#include <string>

class Input {
private:
    WINDOW* win;         // fenêtre ncurses
    int x, y;            // position (ligne, colonne) du début de l'input
    int length;          // longueur maximale affichée
    bool view_text;      // afficher vrai texte (true) ou masqué (false)

    std::string input_text; // contenu saisi
    bool entered = false;   // true si l'utilisateur a validé avec Entrée

public:
    // Constructeur
    Input(WINDOW* stdscr, int x_, int y_, int length_, bool view_text_ = false);

    // Affiche l'input à l'écran
    void draw();

    // Gère une touche clavier
    void handle_key(int key);

    // Accesseur : savoir si entrée validée
    bool is_entered() const { return entered; }

    // Récupérer le texte entré
    std::string get_text() const { return input_text; }
};

#endif // INPUT_H

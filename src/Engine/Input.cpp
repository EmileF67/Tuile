#include "Engine/Input.h"
#include <string>


// répète une chaîne UTF-8 n fois
static std::string repeatUtf8(const std::string& s, int n) {
    std::string out;
    for (int i = 0; i < n; ++i) out += s;
    return out;
}


// --- Constructeur ---
Input::Input(WINDOW* stdscr, int x_, int y_, int length_, bool view_text_)
    : win(stdscr), x(x_), y(y_), length(length_), view_text(view_text_)
{}

// --- Dessine l'input ---
void Input::draw() {
    // Efface la zone de l'input (remplie d'espaces)
    mvwaddstr(win, x, y, std::string(length, ' ').c_str());

    // Affiche le contenu à gauche
    if (view_text) {
        mvwaddstr(win, x, y, input_text.c_str());
    } else {
        mvwaddstr(win, x, y, repeatUtf8(u8"•", input_text.size()).c_str());
    }

    // Place le curseur juste après le texte
    wmove(win, x, y + input_text.size());
}

// --- Gère une touche clavier ---
void Input::handle_key(int key) {
    const int MINCHAR = 32;
    const int MAXCHAR = 126;

    if (MINCHAR <= key && key <= MAXCHAR) {
        if ((int)input_text.size() < length) { // empêche de dépasser la zone
            input_text += static_cast<char>(key);
        }
    } else if (key == KEY_BACKSPACE || key == 127) {
        if (!input_text.empty()) {
            input_text.pop_back();
        }
    } else if (key == 10 || key == 13 || key == KEY_ENTER) {
        entered = true;
    }
}

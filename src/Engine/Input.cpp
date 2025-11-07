#include "Engine/Input.h"
#include <string>
#include <algorithm>


// répète une chaîne UTF-8 n fois
static std::string repeatUtf8(const std::string& s, int n) {
    std::string out;
    for (int i = 0; i < n; ++i) out += s;
    return out;
}


// --- Constructeur ---
Input::Input(WINDOW* stdscr, int x_, int y_, int length_, bool view_text_)
    : win(stdscr), x(x_), y(y_), length(length_), view_text(view_text_)
{
    curs_set(1);
}

Input::~Input() {
    curs_set(0);
}

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
    wmove(win, x, y + cursor_pos);
}

// --- Gère une touche clavier ---
void Input::handle_key(int key) {
    const int MINCHAR = 32;
    const int MAXCHAR = 126;

    if (MINCHAR <= key && key <= MAXCHAR) {
        if ((int)input_text.size() < length) {
            // insertion au bon endroit
            input_text.insert(cursor_pos, 1, static_cast<char>(key));
            cursor_pos = std::min(cursor_pos + 1, static_cast<int>(input_text.size()));
        }
    } 
    else if (key == KEY_BACKSPACE || key == 127) {
        if (cursor_pos > 0 && !input_text.empty()) {
            input_text.erase(cursor_pos - 1, 1);
            cursor_pos = std::max(cursor_pos - 1, 0);
        }
    } 
    else if (key == 10 || key == 13 || key == KEY_ENTER) {
        entered = true;
    } 
    else if (key == KEY_LEFT) {
        cursor_pos = std::max(cursor_pos - 1, 0);
    } 
    else if (key == KEY_RIGHT) {
        cursor_pos = std::min(cursor_pos + 1, static_cast<int>(input_text.size()));
    }
    else if (key == KEY_DC) {
        if (cursor_pos <= static_cast<int>(input_text.size())) {
            input_text.erase(cursor_pos, 1);
        }
    }
    else if (key == KEY_END) {
        cursor_pos = static_cast<int>(input_text.size());
    }
    else if (key == KEY_HOME) {
        cursor_pos = 0;
    }
}

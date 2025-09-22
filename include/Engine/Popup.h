#ifndef POPUP_H
#define POPUP_H

#include <ncurses.h>
#include <string>
#include <utility>
#include "Engine/Input.h"

class Popup {
private:
    WINDOW* win;
    std::pair<int,int> size;   // {rows, cols}
    std::string title;

    // Texte demandé (mode input)
    std::string ask_text;

    // Deux choix possibles (clé, icône)
    std::pair<std::pair<std::string,std::string>, std::pair<std::string,std::string>> choices;

    bool view_text;
    bool sharp_edges;

    int selected = 0;
    bool entered = false;

    // Dans le cas d’un input
    Input* inputField = nullptr;

public:
    Popup(WINDOW* stdscr,
          std::pair<int,int> size_,
          std::string title_,
          std::pair<std::pair<std::string,std::string>, std::pair<std::string,std::string>> choices_,
          std::string ask_text_,
          bool view_text_,
          bool sharp_edges_);

    ~Popup();

    void draw();
    void handle_key(int key);

    bool is_entered() const { return entered; }
    std::string get_text();
    int get_selected() const { return selected; }
};

#endif

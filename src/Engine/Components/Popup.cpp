#include "Engine/Components/Popup.h"
#include "Engine/Components/Cadre.h"
#include <string>
#include <algorithm>

// --- Constructeur ---
Popup::Popup(WINDOW* stdscr,
             std::pair<int,int> size_,
             std::string title_,
             std::pair<std::pair<std::string,std::string>, std::pair<std::string,std::string>> choices_,
             std::string ask_text_,
             bool view_text_,
             bool sharp_edges_)
    : win(stdscr),
      size(size_),
      title(std::move(title_)),
      ask_text(std::move(ask_text_)),
      choices(std::move(choices_)),
      view_text(view_text_),
      sharp_edges(sharp_edges_)
{
    // Si pas de choix -> on prépare un champ input
    if (choices.first.first.empty() && choices.second.first.empty()) {
        int rows, cols;
        getmaxyx(stdscr, rows, cols);

        int h = size.first;
        int w = size.second;

        int x1 = (rows / 2) - (h / 2);
        int y1 = (cols / 2) - (w / 2);

        int input_y = x1 + h / 2;
        int input_x = y1 + 2;
        int box_width = w - 4;

        inputField = new Input(win, input_y + 2, input_x, box_width - 1, view_text);
    }
}

// --- Destructeur ---
Popup::~Popup() {
    if (inputField) {
        delete inputField;
        inputField = nullptr;
    }
}

// --- Dessine le popup ---
void Popup::draw() {
    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    int h = size.first;
    int w = size.second;

    int x1 = (rows / 2) - (h / 2);
    int y1 = (cols / 2) - (w / 2);
    int x2 = x1 + h;
    int y2 = y1 + w;

    // Efface zone
    for (int i = 0; i <= h; i++) {
        mvwaddstr(win, x1+i, y1, std::string(w, ' ').c_str());
    }

    // Cadre
    Cadre cadre(win, {x1, y1}, {x2, y2}, sharp_edges);
    cadre.draw();
    cadre.sep(x1 + 2);

    // Titre
    int max_len = w - 4;
    if (static_cast<int>(title.size()) > max_len) {
        title = title.substr(0, max_len-1);
    }
    mvwaddstr(win, x1+1, y1 + (w - static_cast<int>(title.size())) / 2, title.c_str());

    // Mode "choix"
    if (!choices.first.first.empty()) {
        auto [choiceValue1, choiceIcon1] = choices.first;
        auto [choiceValue2, choiceIcon2] = choices.second;

        int half1 = y1 + w / 4;
        int half2 = y1 + (3 * w) / 4;

        int item1 = half1 - static_cast<int>(choiceValue1.size()) / 2;
        int item2 = half2 - static_cast<int>(choiceValue2.size()) / 2;

        int color1 = (selected == 0) ? COLOR_PAIR(2) : COLOR_PAIR(5);
        int color2 = (selected == 1) ? COLOR_PAIR(2) : COLOR_PAIR(5);

        wattron(win, color1);
        mvwaddstr(win, x1+4, half1, choiceIcon1.c_str());
        mvwaddstr(win, x1+6, item1, choiceValue1.c_str());
        wattroff(win, color1);

        wattron(win, color2);
        mvwaddstr(win, x1+4, half2, choiceIcon2.c_str());
        mvwaddstr(win, x1+6, item2, choiceValue2.c_str());
        wattroff(win, color2);
    }
    else if (inputField) {
        // Mode input
        int input_y = x1 + h / 2;
        int input_x = y1 + 2;

        mvwaddstr(win, input_y, input_x, ask_text.c_str());

        inputField->draw();
    }
}

// --- Gère une touche de clavier ---
void Popup::handle_key(int key) {
    if (!choices.first.first.empty()) {
        // Mode "choix"
        if (key == KEY_LEFT) {
            selected = 0;
        }
        else if (key == KEY_RIGHT) {
            selected = 1;
        }
        else if (key == '\n' || key == KEY_ENTER) {
            entered = true;
        }
    }
    else if (inputField) {
        // Mode "input"
        inputField->handle_key(key);
        if (key == '\n' || key == KEY_ENTER) {
            entered = true;
        }
    }
}

// --- Récupère le texte saisi ---
std::string Popup::get_text() {
    if (inputField) {
        return inputField->get_text();
    }
    return "";
}

#include <clocale>
#include <ncurses.h>
#include <utility>
#include <iostream>
#include <memory>
#include <string>

#include "Engine/Cadre.h"
#include "Engine/Input.h"
#include "Engine/Popup.h"

int main() {
    setlocale(LC_ALL, "");

    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(1);

    if (has_colors()) {
        start_color();
        use_default_colors();
        init_pair(1, COLOR_WHITE, -1);
        init_pair(2, COLOR_GREEN, -1);
        init_pair(5, COLOR_CYAN, -1);
    }

    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    // --- Cadre principal ---
    std::pair<int,int> topLeft {2, 4};
    std::pair<int,int> botRight {rows - 3, cols - 4};
    Cadre cadre(stdscr, topLeft, botRight);
    int middle = (topLeft.first + botRight.first) / 2;

    // Sous-cadre pour l'input en haut
    int inputRow = topLeft.first + 2;
    int inputCol = topLeft.second + 4;
    int inputLen = 20;
    std::pair<int,int> inputTopLeft {inputRow - 1, inputCol - 2};
    std::pair<int,int> inputBotRight {inputRow + 1, inputCol + inputLen + 1};
    Cadre cadreInput(stdscr, inputTopLeft, inputBotRight);

    // Types pour rendre les choix lisibles
    using Choice = std::pair<std::string, std::string>;   // (label, icon)
    using Choices = std::pair<Choice, Choice>;

    // choices pour popup de confirmation
    Choices confirmation = { {"Oui", "✔"}, {"Non", "✘"} };

    // empty choices pour popup input
    Choices emptyChoices = { {"", ""}, {"", ""} };

    // Objets créés "à la demande"
    std::unique_ptr<Input> input = std::make_unique<Input>(stdscr, inputRow, inputCol, inputLen, true);
    std::unique_ptr<Popup> popup;           // créé après la saisie

    bool running = true;
    bool input_done = false;
    std::string texte;

    while (running) {
        // redessine tout
        clear();
        cadre.draw();
        cadre.sep(middle);
        cadreInput.draw();

        // si on n'a pas encore fini la saisie
        if (input && !input->is_entered()) {
            input->draw();
        } else if (input && !input_done) {
            // on récupère le texte une seule fois
            texte = input->get_text();
            input_done = true;

            // créer le popup de confirmation (ici on utilise 'confirmation')
            popup = std::make_unique<Popup>(
                stdscr,
                std::make_pair(10, 40),
                std::string("Confirmation"),
                confirmation,                 // <-- nested pair<string,string>
                std::string("Voulez-vous continuer ?"),
                true
            );
        }

        // si popup existant, l'afficher
        if (popup) {
            popup->draw();
        }

        // rafraîchir l'écran
        wrefresh(stdscr);

        // lire une touche
        int ch = getch();

        // dispatcher la touche à l'élément actif
        if (input && !input->is_entered()) {
            input->handle_key(ch);
        } else if (popup) {
            popup->handle_key(ch);

            if (popup->is_entered()) {
                // ici popup->get_selected() ou popup->get_text() selon mode
                int sel = popup->get_selected();
                // exemple : on affiche le choix puis on quitte
                mvwprintw(stdscr, rows - 4, 2, "Choice: %s", (sel == 0 ? "Oui" : "Non"));
                mvwprintw(stdscr, rows - 3, 2, "Input: %s", texte.c_str());
                wrefresh(stdscr);
                getch();
                running = false;
            }
        } else {
            // aucun widget actif -> on peut quitter par exemple sur 'q'
            if (ch == 'q' || ch == 'Q') running = false;
        }
    }

    endwin();
    return 0;
}

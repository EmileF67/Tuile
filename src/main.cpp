// #include <clocale>
// #include <ncurses.h>
// #include <utility>
// #include <iostream>
// #include <memory>
// #include <string>

// #include "Engine/Cadre.h"
// #include "Engine/Input.h"
// #include "Engine/Popup.h"
// #include "Engine/System.h"

// int main() {
//     setlocale(LC_ALL, "");

//     initscr();
//     cbreak();
//     noecho();
//     keypad(stdscr, TRUE);
//     curs_set(1);

//     if (has_colors()) {
//         start_color();
//         use_default_colors();
//         init_pair(1, COLOR_WHITE, -1);
//         init_pair(2, COLOR_GREEN, -1);
//         init_pair(5, COLOR_CYAN, -1);
//     }

//     bool sharp_edges = System::isLinuxConsole();

//     int rows, cols;
//     getmaxyx(stdscr, rows, cols);

//     // --- Cadre principal ---
//     std::pair<int,int> topLeft {2, 4};
//     std::pair<int,int> botRight {rows - 3, cols - 4};
//     Cadre cadre(stdscr, topLeft, botRight, sharp_edges);
//     int middle = (topLeft.first + botRight.first) / 2;

//     // Sous-cadre pour l'input en haut
//     int inputRow = topLeft.first + 2;
//     int inputCol = topLeft.second + 4;
//     int inputLen = 20;
//     std::pair<int,int> inputTopLeft {inputRow - 1, inputCol - 2};
//     std::pair<int,int> inputBotRight {inputRow + 1, inputCol + inputLen + 1};
//     Cadre cadreInput(stdscr, inputTopLeft, inputBotRight, sharp_edges);

//     // Types pour rendre les choix lisibles
//     using Choice = std::pair<std::string, std::string>;   // (label, icon)
//     using Choices = std::pair<Choice, Choice>;

//     // choices pour popup de confirmation
//     Choices confirmation = { {"Oui", "✔"}, {"Non", "✘"} };

//     // empty choices pour popup input
//     Choices emptyChoices = { {"", ""}, {"", ""} };

//     // Objets créés "à la demande"
//     std::unique_ptr<Input> input = std::make_unique<Input>(stdscr, inputRow, inputCol, inputLen - 1, true); // -1 pour avoir un espace de chaque côté du cadre
//     std::unique_ptr<Popup> popup;           // créé après la saisie

//     bool running = true;
//     bool input_done = false;
//     std::string texte;

//     while (running) {
//         // redessine tout
//         // clear();
//         cadre.draw();
//         cadre.sep(middle);
//         cadreInput.draw();

//         // si on n'a pas encore fini la saisie
//         if (input && !input->is_entered()) {
//             input->draw();
//         } else if (input && !input_done) {
//             // on récupère le texte une seule fois
//             texte = input->get_text();
//             input_done = true;

//             // créer le popup de confirmation (ici on utilise 'confirmation')
//             popup = std::make_unique<Popup>(
//                 stdscr,
//                 std::make_pair(10, 40),
//                 std::string("Confirmation"),
//                 confirmation,                 // <-- nested pair<string,string>
//                 std::string("Voulez-vous continuer ?"),
//                 true,
//                 sharp_edges
//             );
//         }

//         // si popup existant, l'afficher
//         if (popup) {
//             popup->draw();
//         }

//         // rafraîchir l'écran
//         wrefresh(stdscr);

//         // lire une touche
//         int ch = getch();

//         // dispatcher la touche à l'élément actif
//         if (input && !input->is_entered()) {
//             input->handle_key(ch);
//         } else if (popup) {
//             popup->handle_key(ch);

//             if (popup->is_entered()) {
//                 // ici popup->get_selected() ou popup->get_text() selon mode
//                 int sel = popup->get_selected();
//                 // exemple : on affiche le choix puis on quitte
//                 mvwprintw(stdscr, rows - 4, 2, "Choice: %s", (sel == 0 ? "Oui" : "Non"));
//                 mvwprintw(stdscr, rows - 3, 2, "Input: %s", texte.c_str());
//                 wrefresh(stdscr);
//                 getch();
//                 running = false;
//             }
//         } else {
//             // aucun widget actif -> on peut quitter par exemple sur 'q'
//             if (ch == 'q' || ch == 'Q') running = false;
//         }
//     }

//     endwin();
//     return 0;
// }

// // ------------------------------------
// // TEST FOR COLORS
// // ------------------------------------

// // #include <ncurses.h>

// // int main() {
// //     initscr();
// //     if (!has_colors()) {
// //         endwin();
// //         printf("Pas de support couleur.\n");
// //         return 1;
// //     }

// //     start_color();

// //     // Exemple : initier quelques paires de couleurs avec des indices 9, 12, etc.
// //     init_pair(1, 9, 0);   // texte rouge (code 9) sur noir (code 0)
// //     init_pair(2, 10, 0);  // texte “lime” (vert vif, code 10)
// //     init_pair(3, 8, 0);   // texte gris (code 8) sur noir
// //     init_pair(4, 15, 0);  // texte blanc (code 15) sur noir

// //     attron(COLOR_PAIR(1));
// //     printw("Texte rouge\n");
// //     attroff(COLOR_PAIR(1));

// //     attron(COLOR_PAIR(2));
// //     printw("Texte vert vif (lime)\n");
// //     attroff(COLOR_PAIR(2));

// //     attron(COLOR_PAIR(3));
// //     printw("Texte gris (code 8)\n");
// //     attroff(COLOR_PAIR(3));

// //     attron(COLOR_PAIR(4));
// //     printw("Texte blanc (code 15)\n");
// //     attroff(COLOR_PAIR(4));

// //     refresh();
// //     getch();
// //     endwin();
// //     return 0;
// // }


#include <clocale>
#include <ncurses.h>
#include <utility>
#include <iostream>
#include <string>
#include <memory>

#include "Engine/FileManager.h"
#include "Engine/System.h"

// Small test harness to exercise FileManager interactively.
// Usage: ./build/main [start-path]

int main(int argc, char** argv) {
    setlocale(LC_ALL, "");

    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    

    bool is_linux_console = System::isLinuxConsole();

    if (has_colors()) {
        start_color();
        use_default_colors();

        init_pair(1, COLOR_BLUE, -1);     // dossier
        init_pair(2, COLOR_GREEN, -1);    // fichier
        init_pair(3, COLOR_CYAN, -1); // dossier sélectionné
        init_pair(4, COLOR_YELLOW, -1); // fichier sélectionné
        init_pair(5, COLOR_WHITE, -1);   // taille
        init_pair(6, 8, -1);   // taille && cadre (gris)
        init_pair(7, COLOR_BLACK, COLOR_BLUE);
        init_pair(8, COLOR_BLACK, COLOR_GREEN);
        init_pair(9, COLOR_BLACK, COLOR_CYAN);
        init_pair(10, COLOR_BLACK, COLOR_YELLOW);
        // if (is_linux_console) {
        //     init_pair(6, 8, -1);   // taille && cadre (gris)
        // } else {
        //     init_pair(5, COLOR_WHITE, -1);   // taille
        // }
    }


    std::string start_path = std::filesystem::current_path().string();
    if (argc > 1) start_path = argv[1];

    int rows = 0, cols = 0;
    getmaxyx(stdscr, rows, cols);

    // Create FileManager with safe margins; we'll recreate it on resize.
    auto make_fm = [&](int r, int c, std::string spath) {
        std::pair<int,int> topLeft {2, 4};
        std::pair<int,int> botRight {std::max(10, r - 3), std::max(20, c - 4)};
        return std::make_unique<FileManager>(
            stdscr,
            topLeft,
            botRight,
            spath,
            true,   // display sizes
            false,  // hide dotfiles by default
            is_linux_console
        );
    };

    std::unique_ptr<FileManager> fm = make_fm(rows, cols, start_path);
    fm->refresh_entries();

    bool running = true;
    while (running) {
        // draw and refresh
        // clear();
        mvprintw(0, 2, "FileManager test - Quit: q | Resize terminal to test resize");
        fm->draw();
        refresh();

        int ch = getch();

        if (ch == 'q' || ch == 'Q') {
            running = false;
            break;
        }
        
        // TODO : changer la taille d'affichage des variables de la classe directement.
        // TODO : stocker les variables calculées si possible tant qu'on change pas de taille.
        if (ch == KEY_RESIZE) {
            clear();
            getmaxyx(stdscr, rows, cols);
            // recreate FileManager to adapt to new size
            fm = make_fm(rows, cols, fm.get_cwd());
            fm->refresh_entries();
            continue;
        }
        
        if (ch != ERR) {
            // forward key to FileManager
            fm->handle_key(ch);
            fm->draw();
            refresh();
        }
    }

    endwin();
    return 0;
}
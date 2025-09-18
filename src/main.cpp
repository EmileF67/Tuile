#include <clocale>      // setlocale
#include <ncurses.h>
#include <utility>
#include <iostream>
#include "Engine/Cadre.h"
#include "Engine/Input.h"

int main() {
    // Active la locale UTF-8 (affichage des caractères Unicode)
    setlocale(LC_ALL, "");

    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(1); // curseur visible car on saisit du texte

    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    // Définir les coins du grand cadre
    std::pair<int,int> topLeft {2, 4};
    std::pair<int,int> botRight {rows - 3, cols - 4};

    try {
        // Créer le cadre principal
        Cadre cadre(stdscr, topLeft, botRight);
        cadre.draw();

        // Séparer le cadre en deux horizontalement
        int middle = (topLeft.first + botRight.first) / 2;
        cadre.sep(middle);

        // Définir la zone d’input dans la partie supérieure
        int inputRow = topLeft.first + 2;   // 2 lignes sous le haut du cadre
        int inputCol = topLeft.second + 2;  // petit décalage à droite
        int inputLen = 20;

        // Petit sous-cadre pour l'input
        std::pair<int,int> inputTopLeft {inputRow - 1, inputCol - 2};
        std::pair<int,int> inputBotRight {inputRow + 1, inputCol + inputLen + 1};
        Cadre cadreInput(stdscr, inputTopLeft, inputBotRight);
        cadreInput.draw();

        // Créer l’objet Input
        Input input(stdscr, inputRow, inputCol, inputLen, true);

        // Boucle d’interaction
        int ch;
        while (!input.is_entered()) {
            input.draw();
            wrefresh(stdscr);

            ch = getch();
            input.handle_key(ch);
        }

        // Quand Entrée est pressé
        std::string texte = input.get_text();
        mvwprintw(stdscr, middle + 2, topLeft.second + 2, "Vous avez saisi: %s", texte.c_str());

    } catch (const std::exception& e) {
        endwin();
        std::cerr << "Erreur: " << e.what() << "\n";
        return 1;
    }

    mvprintw(rows - 1, 2, "Appuyez sur une touche pour quitter...");
    refresh();
    getch();

    endwin();
    return 0;
}

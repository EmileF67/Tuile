#include <clocale>
#include <ncurses.h>
#include <utility>
#include <iostream>
#include <string>
#include <memory>

#include "Engine/MainEngine.h"

#include "Apps/FileManager.h"
#include "Engine/System.h"

// Gérer le Ctrl + C
#include <signal.h>
#include <atomic>

// TODO ULTIME
// QUAND ON VEUT AFFICHER QUELQUE CHOSE, ON MODIFIE UN TABLEAU DEUX DIMENSIONS DE CARACTERES QUI REPRESENTE LE TERMINAL
// ON LE COMPARE AVEC LE PRECEDENT QUAND ON AFFICHE POUR SAVOIR QUOI AFFICHER ET ON LE FAIT EN UNE FOIS POUR EVITER DE SURCHARGER LE CPU

std::atomic<bool> stop(false);

void handle_sigint(int) {
    stop = true;
}

#define COLOR_GREY 8


// Programme Principal
int main(int argc, char** argv) {
    setlocale(LC_ALL, "");
    signal(SIGINT, handle_sigint);

    initscr();              // Initialise ncurses, crée stdscr
    cbreak();               // Entrée caractère par caractère
    noecho();               // Pas d'écho automatique du clavier
    keypad(stdscr, TRUE);   // Active les touches spéciales
    curs_set(0);            // Cache le curseur
    
    timeout(100); // getch() attend max 100 ms

    bool is_linux_console = System::isLinuxConsole();

    if (has_colors()) {
        start_color();
        use_default_colors();

        init_pair(1,  COLOR_BLUE,   -1);            // dossier
        init_pair(2,  COLOR_GREEN,  -1);            // fichier
        init_pair(3,  COLOR_CYAN,   -1);            // dossier sélectionné
        init_pair(4,  COLOR_YELLOW, -1);            // fichier sélectionné
        init_pair(5,  COLOR_WHITE,  -1);            // taille
        init_pair(6,  COLOR_GREY,   -1);            // taille && cadre (gris)
        init_pair(7,  COLOR_BLACK, COLOR_BLUE   );  // bg dossier
        init_pair(8,  COLOR_BLACK, COLOR_GREEN  );  // bg fichier
        init_pair(9,  COLOR_BLACK, COLOR_CYAN   );  // bg dossier sélectionné
        init_pair(10, COLOR_BLACK, COLOR_YELLOW );  // bg fichier sélectionné
    }

    // Déterminer le chemin de démarrage
    std::string start_path = std::filesystem::current_path().string();
    if (argc > 1) start_path = argv[1];

    // Notre seul et unique Main Engine
    std::unique_ptr<MainEngine> mEngine = std::make_unique<MainEngine>(stdscr);

    // On créer nos deux fenêtres
    WINDOW* win1 = mEngine->new_window("FileManager1");
    WINDOW* win2 = mEngine->new_window("FileManager2");

    // On créer notre FileManager n°1 avec les coordonnées relatives à sa fenêtre
    FileManager fm1(win1, start_path, true, false, is_linux_console);
    fm1.refresh_entries();

    // On créer notre FileManager n°2 avec les coordonnées relatives à sa fenêtre
    FileManager fm2(win2, start_path, true, false, is_linux_console);
    fm2.refresh_entries();

    // On met le focus sur la fenêtre n°1
    int focus = 1;
    fm1.toggle_focus();

    // On affiche les deux
    fm1.draw();
    fm2.draw();

    // On refresh le tout et on update l'affichage
    mEngine->refresh_all_and_update();





    // int rows = 0, cols = 0;     // On sécurise en métant 0
    // getmaxyx(stdscr, rows, cols);

    // Create FileManager with safe margins; we'll recreate it on resize.
    // auto make_fm = [&](int r, int c, std::string spath) {
    //     std::pair<int,int> topLeft {2, 4};
    //     std::pair<int,int> botRight {std::max(10, r - 3), std::max(20, c - 4)};
    //     return std::make_unique<FileManager>(
    //         stdscr,
    //         topLeft,
    //         botRight,
    //         spath,
    //         true,   // display sizes
    //         false,  // hide dotfiles by default
    //         is_linux_console
    //     );
    // };

    // std::unique_ptr<FileManager> fm = make_fm(rows, cols, start_path);
    // fm->refresh_entries();

    std::string text = "Change focus :      left -> press <a>      right -> press <b>      Ctrl + C -> stop process";
    mvwaddstr(stdscr, 0, 0, text.c_str());

    bool running = true;
    while (running && !stop) {
        // draw and refresh
        // clear();

        // mvprintw(0, 2, "FileManager test - Quit: Ctrl + C | Resize terminal to test resize");
        // fm->draw();
        // refresh();

        int ch = getch();

        // --- Ctrl + Q ---
        // if (ch == 17) {
        //     running = false;
        //     break;
        // }        
        
        // TODO : changer la taille d'affichage des variables de la classe directement.
        // TODO : stocker les variables calculées si possible tant qu'on change pas de taille.
        if (ch == KEY_RESIZE) {
            // clear();
            // getmaxyx(stdscr, rows, cols);
            // // recreate FileManager to adapt to new size
            // fm = make_fm(rows, cols, fm->get_cwd());
            // fm->refresh_entries();
            // continue;

            if (mEngine->detect_resizing()) {
                fm1.draw();
                fm2.draw();
                mEngine->refresh_all_and_update();
            }

        }
        
        if (ch != ERR) {
            // forward key to FileManager

            if (ch == 'a') {
                if (focus != 1) {
                    fm1.toggle_focus();
                    fm2.toggle_focus();
                    focus = 1;
                    fm2.draw();
                }
            } else if (ch == 'p') {
                if (focus != 2) {
                    fm1.toggle_focus();
                    fm2.toggle_focus();
                    focus = 2;
                    fm1.draw();
                }
            }

            switch (focus) {
                case 1:
                    fm1.handle_key(ch);
                    fm1.draw();
                    break;

                case 2:
                    fm2.handle_key(ch);
                    fm2.draw();
                    break;
            }

            mEngine->refresh_all_and_update();
            
            // fm->handle_key(ch);
            // fm->draw();
            // refresh();
        }
    }

    curs_set(1);
    echo();
    nocbreak();
    endwin();       // Restaure le terminal

    return 0;
}
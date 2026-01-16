#include <clocale>
#include <ncurses.h>
#include <utility>
#include <iostream>
#include <string>
#include <memory>

#include "Engine/FileManager.h"
#include "Engine/System.h"

// Gérer le Ctrl + C
#include <signal.h>
#include <atomic>

std::atomic<bool> stop(false);

void handle_sigint(int) {
    stop = true;
}


// Programme Principal
int main(int argc, char** argv) {
    setlocale(LC_ALL, "");
    signal(SIGINT, handle_sigint);

    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    
    timeout(100); // getch() attend max 100 ms

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
    }


    std::string start_path = std::filesystem::current_path().string();
    if (argc > 1) start_path = argv[1];

    int rows = 0, cols = 0;     // On sécurise en métant 0
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
    while (running && !stop) {
        // draw and refresh
        // clear();
        mvprintw(0, 2, "FileManager test - Quit: Ctrl + C | Resize terminal to test resize");
        fm->draw();
        refresh();

        int ch = getch();

        // --- Ctrl + Q ---
        // if (ch == 17) {
        //     running = false;
        //     break;
        // }
        
        // TODO : changer la taille d'affichage des variables de la classe directement.
        // TODO : stocker les variables calculées si possible tant qu'on change pas de taille.
        if (ch == KEY_RESIZE) {
            clear();
            getmaxyx(stdscr, rows, cols);
            // recreate FileManager to adapt to new size
            fm = make_fm(rows, cols, fm->get_cwd());
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

    curs_set(1);
    echo();
    nocbreak();
    endwin();

    return 0;
}
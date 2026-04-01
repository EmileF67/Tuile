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

        init_pair(1,  COLOR_BLUE,   -1);            // dossier                  && affichage pourcentage
        init_pair(2,  COLOR_GREEN,  -1);            // fichier                  && affichage pourcentage
        init_pair(3,  COLOR_CYAN,   -1);            // dossier sélectionné      && affichage pourcentage
        init_pair(4,  COLOR_YELLOW, -1);            // fichier sélectionné      && affichage pourcentage
        init_pair(5,  COLOR_WHITE,  -1);            // taille
        init_pair(6,  COLOR_GREY,   -1);            // taille                   && cadre (gris)
        init_pair(7,  COLOR_BLACK, COLOR_BLUE   );  // bg dossier
        init_pair(8,  COLOR_BLACK, COLOR_GREEN  );  // bg fichier
        init_pair(9,  COLOR_BLACK, COLOR_CYAN   );  // bg dossier sélectionné
        init_pair(10, COLOR_BLACK, COLOR_YELLOW );  // bg fichier sélectionné
        init_pair(11, COLOR_BLACK, COLOR_WHITE  );  // bg type data module
        init_pair(12, COLOR_RED,    -1);            // affichage pourcentage
    }

    // Déterminer le chemin de démarrage
    std::string start_path = std::filesystem::current_path().string();
    if (argc > 1) start_path = argv[1];

    // Notre seul et unique Main Engine
    std::unique_ptr<MainEngine> mEngine = std::make_unique<MainEngine>(stdscr, is_linux_console);

    // Vecteur pour stocker les FileManagers
    std::vector<std::unique_ptr<FileManager>> fileManagers;

    // On met le focus à -1 (aucun FileManager)
    int focus = -1;

    // On affiche la barre
    mEngine->draw_bar();

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

    // std::string text = "Change focus :      left -> press <a>      right -> press <b>      Ctrl + C -> stop process";
    // mvwaddstr(stdscr, 0, 0, text.c_str());

    bool running = true;
    while (running && !stop) {
        // draw and refresh
        // clear();

        // mvprintw(0, 2, "FileManager test - Quit: Ctrl + C | Resize terminal to test resize");
        // fm->draw();
        // refresh();

        int ch = getch();



        for (auto& fm : fileManagers) {
            fm->draw();
        }

        mEngine->refresh_all_and_update();

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
                werase(stdscr);

                for (auto& fm : fileManagers) {
                    fm->draw();
                }
                mEngine->draw_bar();
                mEngine->draw_popup();
                mEngine->refresh_all_and_update();
            }

        } else if (ch == '+' && 
                   !mEngine->has_active_popup() &&
                   (focus < 0 || !fileManagers[focus]->is_editing_path())) 
        {
            werase(stdscr);

            // Créer un nouveau FileManager
            WINDOW* new_win = mEngine->new_window("FileManager" + std::to_string(fileManagers.size() + 1));
            std::string new_fm_path = (focus >= 0) ? fileManagers[focus]->get_cwd() : start_path;
            auto new_fm = std::make_unique<FileManager>(new_win, *mEngine, new_fm_path, true, false, is_linux_console);
            new_fm->refresh_entries();
            new_fm->draw();
            mEngine->refresh_all_and_update();
            
            // Si c'est le premier FileManager, lui donner le focus
            if (fileManagers.empty()) {
                new_fm->toggle_focus();
                focus = 0;
            }
            
            fileManagers.push_back(std::move(new_fm));

        } else if (ch != ERR && focus >= 0) {
            // Gérer le changement de focus
            bool focus_changed = false;
            int new_focus = focus;

            if (ch >= '1' && ch <= '9' && !mEngine->has_active_popup() && !fileManagers[focus]->is_editing_path()) {
                // Changer le focus vers le FileManager numéro (ch - '1')
                int target_focus = ch - '1';
                if (target_focus >= 0 && target_focus < (static_cast<int>(fileManagers.size())) && target_focus != focus) {
                    new_focus = target_focus;
                    focus_changed = true;
                }
            }

            if (focus_changed) {
                // Changer le focus
                fileManagers[focus]->set_focused(false);
                focus = new_focus;
                fileManagers[focus]->set_focused(true);
            } else {
                // Le FileManager avec le focus gère les touches
                fileManagers[focus]->handle_key(ch);
            }

            // Redessiner tous les filemanagers
            for (auto& fm : fileManagers) {
                fm->draw();
            }

            // Afficher la barre
            mEngine->draw_bar();

            // Afficher le popup (s'il existe)
            mEngine->draw_popup();

            mEngine->refresh_all_and_update();

        } else if (focus >= 0 && fileManagers[focus]->is_editing_path()) {
            // Redessiner si on est en train d'éditer le chemin, même sans input
            for (auto& fm : fileManagers) {
                fm->draw();
            }
            mEngine->draw_bar();
            mEngine->draw_popup();
            mEngine->refresh_all_and_update();
        } else {
            // Timeout du getch - redessiner pour les modules dynamiques (DateTime etc)
            mEngine->draw_bar();
            mEngine->draw_popup();
            mEngine->refresh_all_and_update();
        }

        // Placer le curseur après chaque redessinage (surtout important pour editing_path)
        if (focus >= 0 && fileManagers[focus]->is_editing_path()) {
            fileManagers[focus]->place_cursor();
            wrefresh(fileManagers[focus]->get_win());
        }
    }

    curs_set(1);
    echo();
    nocbreak();
    endwin();       // Restaure le terminal

    return 0;
}
#ifndef MAINENGINE_H
#define MAINENGINE_H

#include "Engine/Components/Popup2.h"
#include "Apps/Bar.h"
#include <ncurses.h>
#include <string>
#include <vector>
#include <queue>
#include <memory>

//                            Logo        Label
using Choice  = std::pair<std::string, std::string>;
using Choices = std::pair<Choice, Choice>;

class MainEngine {
    private:
        WINDOW* stdscr;
        bool is_linux_console;

        // ORDRE GARANTI
        std::vector<WINDOW*> windows;
        std::vector<std::string> window_names;

        int rows;
        int cols;

        bool display_bar;

        // Type actuel du popup, None si aucun
        PopupType popup_type;

        // Les différents popups
        std::unique_ptr<PopupInfo> popup_info;
        std::unique_ptr<PopupInputText> popup_input_text;
        std::unique_ptr<PopupDoubleChoices> popup_double_choices;

        // La barre
        std::unique_ptr<Bar> bar;

        void update_layout();

    public:
        explicit MainEngine(WINDOW* stdscr_, bool is_linux_console_);
        ~MainEngine();

        WINDOW* new_window(const std::string& name);

        void refresh_all_and_update();

        bool detect_resizing();

        bool global_handle_key(int key);

        void load_bar_modules(WINDOW* stdscr);

        // $==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==
        // $ Popups 
        // $==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==

        // Créer un popup d'information
        void create_info_popup(std::string label);

        // Créer un popup d'entrée de valeur
        void create_input_popup(std::string label);

        // Créer un popup de choix double
        void create_double_choices_popup(std::string label, Choices choices);

        // Vérifie si un popup est terminé et récupère la valeur
        bool is_popup_done() const;

        // Vérifie s'il y a un popup actif
        bool has_active_popup() const { return popup_type != PopupType::None; };

        // Récupérer la valeur selon le type de popup
        std::string get_input_popup_value() const;
        short get_double_choices_popup_value() const;

        // Permet d'afficher le popup actuel s'il y en a un
        void draw_popup();

        // Dessine la barre
        void draw_bar();

        // Permet de remettre à 0 les status de popup
        void reset_popups();
};

#endif // MAINENGINE_H

#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <utility>
#include <ncurses.h>
#include <vector>
#include <string>
#include <filesystem>
#include <memory>

struct EntryDisplay {
    std::string icon;
    std::string size;
    attr_t color;
    attr_t color_bg;
    std::string permissions;
};

class Popup; // forward declaration

class FileManager {
    private:
        WINDOW* win;
        std::pair<int,int> x; // {row1, col1}
        std::pair<int,int> y; // {row2, col2}
        std::string start_path;
        bool display_size;
        bool display_dotfiles;
        bool is_linux_console;

        // State
        bool editing_path;
        std::string path_input;
        std::string cwd;
        std::vector<std::string> entries;

        int selected;
        int scroll_offset;

        // Interactive flags / helpers
        bool input_new;
        int nouveau;
        bool input_new_name;
        std::string new_name;
        bool remove_element;
        bool rename_element;
        std::string new_rename;
        bool copying;
        std::string copy_mode;
        std::string path_to_copy;
        std::string editor;
        std::unique_ptr<Popup> popup;
        bool cursor_on;
        bool aSpace;
        bool display_icons;
        bool display_perms;


    public:
        // Constructeur
        FileManager(WINDOW* stdscr, std::pair<int,int> x_, std::pair<int,int> y_, const std::string& start_path_, bool display_size_, bool display_dotfiles_, bool is_linux_console_);

        // Destructeur
        ~FileManager();

        // Affiche l'application
        void draw();

        //Gère la navigation clavier
        void handle_key(int key);

        // Génération de la liste des entrée du cwd
        void refresh_entries();

        // Obtenir le cwd
        std::string get_cwd() { return cwd; };
        

    private:
        // Copie un élément d'un endroit vers un autre (récursif)
        void copy_to_path();

        // Tri alphabétique d'abord des dossiers, puis fichiers
        std::vector<std::string> tri_dossiers_fichiers(const std::vector<std::string>& lst);

        // Retourne un nombre de bits sous format plus lisible par l'entier (Donc Mo, Go, etc.....)
        std::string human_readable_size(long long size);

        // Affiche les éléments principaux du filemanager
        void draw_header(int x1, int y1, int x2, int y2, std::string* display_text);

        // Affiche toutes les entrées qui sont visibles (en fonction du scroll)
        void draw_entries(int x1, int y1, int x2, int cols);

        // Affiche les popups si existants
        void draw_popups();

        // Renvoie les détails d'affichage d'une entrée
        EntryDisplay get_entry_display_info(std::string entry);


};


#endif // FILEMANAGER_H

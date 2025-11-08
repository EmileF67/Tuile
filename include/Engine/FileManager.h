#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <utility>
#include <ncurses.h>
#include <vector>
#include <string>
#include <filesystem>
#include <memory>


class Popup; // forward declaration

class FileManager {
    private:
        WINDOW* win;
        std::pair<int,int> x; // {row1, col1}
        std::pair<int,int> y; // {row2, col2}
        std::string start_path;
        bool display_size;
        bool display_dotfiles;
        bool sharp_edges;

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

    public:
    FileManager(WINDOW* stdscr, std::pair<int,int> x_, std::pair<int,int> y_, const std::string& start_path_, bool display_size_, bool display_dotfiles_, bool sharp_edges_);
    ~FileManager();

        // Affiche l'application
        void draw();

        //Gère la navigation clavier
    void handle_key(int key);

        // Retourne un nombre de bits sous format plus lisible par l'entier (Donc Mo, Go, etc.....)
    std::string human_readable_size(long long size);

        // Tri alphabétique d'abord des dossiers, puis fichiers
    std::vector<std::string> tri_dossiers_fichiers(const std::vector<std::string>& lst);

        // Génération de la liste des entrée du cwd
        void refresh_entries();

        // Copie un élément d'un endroit vers un autre (récursif)
        void copy_to_path();

};


#endif // FILEMANAGER_H

#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <utility>
#include <ncurses.h>
#include <vector>
#include <string>
#include <filesystem>
#include <memory>
#include <unordered_set>
#include "Engine/Components/MessageBox.h"

struct EntryDisplay {
    std::string icon;
    std::string size;
    attr_t color;
    attr_t color_bg;
    std::string permissions;
};


// Déclaration du clipboard
enum class ClipboardMode {
    None,
    Copy,
    Cut
};

struct ClipboardItem {
    std::filesystem::path path;
};

struct Clipboard {
    ClipboardMode mode = ClipboardMode::None;

    // Nouveau : set rapide
    std::unordered_set<std::filesystem::path> items;

    // Vide le set et remet le mode à None
    void clear() { 
        items.clear(); 
        mode = ClipboardMode::None;
    }

    // Vérifie si le clipboard est vide
    bool empty() const { 
        return items.empty(); 
    }

    // Vérifie si un élément est sélectionné
    bool contains(const std::filesystem::path& item) const {
        return items.find(item) != items.end();
    }

    // Toggle : ajoute si absent, retire si présent
    void toggle(const std::filesystem::path& item) {
        auto it = items.find(item);
        if (it != items.end())
            items.erase(it);
        else
            items.insert(item);
    }
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
        std::unique_ptr<MessageBox> msgbox;
        bool cursor_on;
        bool aSpace;
        bool display_icons;
        bool display_perms;

        Clipboard clipboard;


    public:
        // Constructeur
        FileManager(WINDOW* stdscr, std::pair<int,int> x_, std::pair<int,int> y_, const std::string& start_path_, bool display_size_, bool display_dotfiles_, bool is_linux_console_);

        // Destructeur
        ~FileManager();

        // Affiche l'application
        void draw();

        // Gère la navigation clavier
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

        // Affiche la messagebox
        void draw_messagebox();

        // Renvoie les détails d'affichage d'une entrée
        EntryDisplay get_entry_display_info(std::string entry);


};


#endif // FILEMANAGER_H

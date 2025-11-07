#include "Engine/FileManager.h"
#include <string>
#include <vector>
#include <filesystem>
#include <algorithm>
#include <iostream>
#include <fstream>

#include "Engine/Cadre.h"
#include "Engine/Popup.h"


// INIT COLORS :
// 1 | Blue   | None | init_pair(1, 12, 0)
// 2 | Green  | None | init_pair(1, 10, 0)
// 3 | Cyan   | None | init_pair(1, 14, 0)
// 4 | Yellow | None | init_pair(1, 11, 0)
// 5 | White  | None | init_pair(1, 15, 0)



namespace fs = std::filesystem;

// define destructor here so Popup is a complete type for destruction
FileManager::~FileManager() = default;

// --- Constructeur ---
FileManager::FileManager(WINDOW* stdscr, std::pair<int,int> x_, std::pair<int,int> y_, const std::string& start_path_, bool display_size_, bool display_dotfiles_, bool sharp_edges_)
    : win(stdscr),
      x(x_),
      y(y_),
      start_path(start_path_),
      display_size(display_size_),
      display_dotfiles(display_dotfiles_),
      sharp_edges(sharp_edges_)
{
    // initialize members
    cwd = start_path;
    path_input = "";
    selected = 0;
    scroll_offset = 0;

    input_new = false;
    nouveau = 0;
    input_new_name = false;
    new_name = "";
    remove_element = false;
    rename_element = false;
    new_rename = "";
    copying = false;
    copy_mode = "";
    path_to_copy = "";
    editor = "vim";
    popup.reset();
}

std::string rjust(const std::string& str, size_t width, char fill = ' ') {
    if (str.size() >= width)
        return str;
    return std::string(width - str.size(), fill) + str;
}

std::string FileManager::human_readable_size(long long size) {
    const char* units[] = {"B", "K", "M", "G", "T", "P"};
    double s = static_cast<double>(size);
    int unit = 0;
    while (s >= 1024.0 && unit < 5) {
        s /= 1024.0;
        ++unit;
    }
    char buf[64];
    if (unit == 0) {
        std::snprintf(buf, sizeof(buf), "%lld %s", static_cast<long long>(size), units[unit]);
    } else {
        std::snprintf(buf, sizeof(buf), "%.1f %s", s, units[unit]);
    }
    return std::string(buf);
}


std::vector<std::string> FileManager::tri_dossiers_fichiers(const std::vector<std::string>& lst) {
    std::vector<std::string> lstdir;
    std::vector<std::string> lstfile;

    for (const auto& e : lst) {
        fs::path p = fs::path(cwd) / e;
        if (fs::is_directory(p)) {
            lstdir.push_back(e);
        } else {
            lstfile.push_back(e);
        }
    }

    std::sort(lstdir.begin(), lstdir.end());
    std::sort(lstfile.begin(), lstfile.end());

    // Concaténer les deux vecteurs
    std::vector<std::string> result;
    result.reserve(lstdir.size() + lstfile.size());
    result.insert(result.end(), lstdir.begin(), lstdir.end());
    result.insert(result.end(), lstfile.begin(), lstfile.end());

    return result;
}


void FileManager::refresh_entries() {
    // Déclaration d'une variable pour accueillir les éléments du cwd.
    std::vector<std::string> temp;
    
    // On ajoute dès le début un moyen de revenir en arrière (sauf si on est à la racine)
    if (cwd != "/") {
        temp.push_back("..");
    }
    
    // Filtrer dynamiquement les entrées
    try {
        std::vector<std::string> items;

        // Lire le contenu du dossier
        for (const auto& entry : fs::directory_iterator(cwd)) {
            items.push_back(entry.path().filename().string());
        }

        // Appeler la fonction de tri (comme en Python)
        items = tri_dossiers_fichiers(items);

        if (editing_path) {
            // Filtrage dynamique
            if (display_dotfiles) {
                for (const auto& e : items) {
                    if (e.rfind(path_input, 0) == 0) { // commence par path_input
                        temp.push_back(e);
                    }
                }
            } else {
                for (const auto& e : items) {
                    if (e.rfind(path_input, 0) == 0 && e[0] != '.' && e != "__pycache__") {
                        temp.push_back(e);
                    }
                }
            }
        } else {
            // Filtrage normal
            if (display_dotfiles) {
                for (const auto& e: items) {
                    temp.push_back(e);
                }
            } else {
                for (const auto& e : items) {
                    if (e[0] != '.' && e != "__pycache__"){
                        temp.push_back(e);
                    }
                }
            }
        }

        // sending new entries from <temp> to <entries>
    entries = temp;


    } catch (const fs::filesystem_error& e) {
        entries.clear();
        entries.push_back("..");
        entries.push_back("PERMISSION ERROR");
    }

    // Finally we modify the selected entry
    if (!entries.empty()) {
        selected = std::min(selected, static_cast<int>(entries.size()) - 1);
    } else {
        selected = 0;
    }
}


void FileManager::copy_to_path() {
    // Recopier la fonction depuis python
    try {
        fs::path dest_path = fs::path(cwd) / fs::path(path_to_copy).filename();

        // Éviter d’écraser un fichier déjà existant
        if (fs::exists(dest_path))
            return;

        if (copy_mode == "c") {
            if (fs::is_directory(path_to_copy)) {
                fs::copy(path_to_copy, dest_path, fs::copy_options::recursive);
            } else {
                fs::copy_file(path_to_copy, dest_path);
            }
        } else if (copy_mode == "x") {
            fs::rename(path_to_copy, dest_path); // Déplacement
        }

    } catch (const std::exception& e) {
        std::cerr << "Erreur lors de la copie : " << e.what() << std::endl;
    }
}


void FileManager::draw() {
    int rows, cols;
    getmaxyx(win, rows, cols);

    int x1 = x.first;
    int y1 = x.second;
    int x2 = y.first;
    int y2 = y.second;

    int h = x2 - x1;
    int w = y2 - y1;

    // Efface zone
    for (int i = 0; i <= h; i++) {
        mvwaddstr(win, x1 + i, y1, std::string(w, ' ').c_str());
    }

    // Cadre
    Cadre cadre(win, {x1 - 1, y1 - 1}, {x2 + 1, y2 + 1}, sharp_edges);
    cadre.draw();
    cadre.sep(x1 + 1);

    // Afficher le chemin
    std::string display_text;
    if (editing_path) {
        if (cwd == "/") {
            display_text = " " + cwd + path_input;
        } else {
            display_text = " " + cwd + "/" + path_input;
        }
    } else {
        display_text = " " + cwd + " ";
    }
    mvwaddstr(win, x1, y1, display_text.c_str());

    // Afficher les entrées
    size_t start = static_cast<size_t>(scroll_offset);
    size_t end = std::min(entries.size(), start + static_cast<size_t>(rows));
    std::vector<std::string> visible_entries(entries.begin() + start, entries.begin() + end);

    for (std::size_t i = 0; i < visible_entries.size(); i++) {
        int abs_idx = static_cast<int>(start + i);
        std::string entry = visible_entries[i];
        std::string size_str = "";
        fs::path full_path = fs::path(cwd) / entry;

        int color = 5;
        std::string icon;

        if (fs::is_directory(full_path)) {
            color = 1;
            icon = " ";
            size_str = "";
        } else {
            color = 2;
            icon = " ";

            try {
                auto sz = std::filesystem::file_size(full_path);
                size_str = human_readable_size(static_cast<long long>(sz));
            } catch (const std::filesystem::filesystem_error&) {
                size_str = "?";
            }
        }

        std::string prefix;
        if (abs_idx == selected) {
            prefix = "> ";
            if (fs::is_directory(full_path)) {
                color = 3;
            } else {
                color = 4;
            }
        } else {
            prefix = "  ";
        }

        std::string text = prefix + icon + entry;
        wattron(win, COLOR_PAIR(color));
        mvwaddstr(win, x1 + 2 + static_cast<int>(i), y1, text.c_str());
        wattroff(win, COLOR_PAIR(color));
        
        // Afficher taille de l'élément
        if (abs_idx == selected) {
            color = 4;
        } else {
            color = 6;
        }
        wattron(win, COLOR_PAIR(color));
        mvwaddstr(win, x1 + 2 + static_cast<int>(i), y1 + cols - 10 - static_cast<int>(size_str.size()), size_str.c_str()); // TODO
        wattroff(win, COLOR_PAIR(color));
    }

    // Si l'on est entrain de modifier le chemin manuellement
    if (editing_path) {
        wmove(win, x1, y1 + static_cast<int>(display_text.size()));
    }

    // Si l'on est entrain de faire un choix pour un nouvel élément
    if (input_new) {
        if (!popup) {
            using Choice = std::pair<std::string, std::string>;
            using Choices = std::pair<Choice, Choice>;
            Choices choix = {{"Fichier", ""}, {"Dossier", ""}};
            popup = std::make_unique<Popup>(win, std::make_pair(9, 40), "Nouveau", choix, "", true, sharp_edges);
        }

        popup->draw();
    }

    // Si l'on est entrain de définir le nom du nouvel élément
    if (input_new_name) {
        if (popup) popup->draw();
    }

    // Si l'on est entrain de retirer un élément
    if (remove_element) {
        if (!popup) {
            std::string label;
            if (cwd == "/home/emile/.Corbeille") {
                label = "Supprimer " + entries[selected] + " définitivement";
            } else {
                label = std::string("Déplacer ") + entries[selected] + " dans la Corbeille";
            }
            using Choice = std::pair<std::string, std::string>;
            using Choices = std::pair<Choice, Choice>;
            Choices choix = {{"Oui", ""}, {"Non", ""}};
            popup = std::make_unique<Popup>(win, std::make_pair(9, 90), label, choix, "", true, sharp_edges);
            // default selected to 1 (second option)
        }
        if (popup) popup->draw();
    }

    // Si l'on est entrain de renommer un élément
    if (rename_element) {
        if (!popup) {
            std::string label = "Renommer " + entries[selected];
            popup = std::make_unique<Popup>(win, std::make_pair(9, 50), label, std::make_pair(std::make_pair(std::string(), std::string()), std::make_pair(std::string(), std::string())), "Nouveau nom :", true, sharp_edges);
        }
        if (popup) popup->draw();
    }

    // Si l'on est entrain de copier/couper un élément TODO
    if (copying) {
        if (!popup) {
            std::string label;
            if (copy_mode == "c") {
                label = "Copier et Coller " + std::filesystem::path(path_to_copy).filename().string() + " ici ?";
            } else {
                label = "Couper et Coller " + std::filesystem::path(path_to_copy).filename().string() + " ici ?";
            }
            using Choice = std::pair<std::string, std::string>;
            using Choices = std::pair<Choice, Choice>;
            Choices choix = {{"Oui", ""}, {"Non", ""}};
            popup = std::make_unique<Popup>(win, std::make_pair(9, 90), label, choix, "", true, sharp_edges);
        }
        if (popup) popup->draw();
    }
}




void FileManager::handle_key(int key) {
    int rows, cols;
    getmaxyx(win, rows, cols);
    const int MINCHAR = 32;
    const int MAXCHAR = 126;

    int scroll_margin = 3;

    if (editing_path) {
        if (key == KEY_BACKSPACE || key == 127) {
            if (!path_input.empty()) path_input = path_input.substr(0, path_input.size() - 1);
            selected = 0;
            scroll_offset = 0;
        } else if (MINCHAR <= key && key <= MAXCHAR) {
            path_input += static_cast<char>(key);
            selected = 0;
            scroll_offset = 0;
        }

        refresh_entries();
    }

    
    // Si l'on est pas entrain de toucher au clavier dans un popup
    if (!input_new &&
        !input_new_name &&
        !remove_element &&
        !rename_element &&
        !copying) {
        // --- Touche flèche haut et gauche ---
        if (key == KEY_UP || key == KEY_LEFT) {
            selected = std::max(0, selected-1);

            // Si on est trop proche du haut de la fenêtre visible
            if (selected < scroll_offset + scroll_margin) {
                scroll_offset = std::max(0, selected - scroll_margin);
            }
        }

        // --- Touche flèche bas et droite ---
        else if (key == KEY_DOWN || key == KEY_RIGHT) {
            selected = std::min(static_cast<int>(entries.size()) - 1, selected + 1);

            // Si on est trop proche du bas de la fenêtre visible
            if (selected >= scroll_offset + rows - scroll_margin) {
                scroll_offset = std::min(
                    std::max(0, static_cast<int>(entries.size()) - rows),
                    selected - rows + scroll_margin + 1
                );
            }
        }

        // --- Touche F1 ---
        else if (key == KEY_F(1)) {
            display_dotfiles = !display_dotfiles;
            refresh_entries();
        }

        // --- Touche Slash ---
        else if (key == static_cast<int>('/')) {
            editing_path = !editing_path;
            if (editing_path) {
                path_input = "";
            }

            refresh_entries();

            selected = 0;
            scroll_offset = 0;
        }

        // --- Touche Entrée ---
        else if (key == KEY_ENTER || key == 10 || key == 13) {
            if (entries.empty()) return;
            std::string target = entries[selected];

            if (target != "PERMISSION ERROR") {
                fs::path new_path = fs::path(cwd) / target;

                if (target == "..") {
                    cwd = fs::path(cwd).parent_path().string();

                    // Ajuster le scroll
                    if (selected >= scroll_offset + rows) {
                        scroll_offset = selected - rows + 1;
                    } else if (selected < scroll_offset) {
                        scroll_offset = selected;
                    }
                } else if (fs::is_directory(new_path)) {
                    cwd = new_path.string();
                    selected = 0;
                    scroll_offset = 0;
                } else { // Si c'est un fichier
                    if (editor == "vim") {
                        endwin();
                        std::string cmd = "vim \"" + new_path.string() + "\"";
                        system(cmd.c_str());
                        wrefresh(win);
                        doupdate();
                    } else if (editor == "vscode") {
                        std::string cmd = "code \"" + new_path.string() + "\"";
                        system(cmd.c_str());
                    }
                }
            }

            if (editing_path) {
                path_input = "";
            }

            refresh_entries();
        }

        // --- Touche Page Down ---
        if (key == KEY_NPAGE) {
            selected = std::min(static_cast<int>(entries.size()) - 1, selected + rows);
            scroll_offset = std::min(
                std::max(0, static_cast<int>(entries.size()) - rows),
                selected - rows + 1
            );
        }

        // --- Touche Page Up ---
        else if (key == KEY_PPAGE) {
            selected = std::max(0, selected - rows);
            scroll_offset = std::max(0, selected);
        }

        // --- Touche Début ---
        else if (key == KEY_HOME) {
            selected = 0;
            scroll_offset = 0;
        }

        // --- Touche Fin ---
        else if (key == KEY_END) {
            selected = static_cast<int>(entries.size()) - 1;
            scroll_offset = std::max(0, static_cast<int>(entries.size()) - rows);
        }

        // --- Touche n ---
        else if (key == static_cast<int>('n') && !editing_path) {
            editing_path = false;
            input_new = true;
        }

        // --- Touche Suppr ---
        else if (key == KEY_DC && !editing_path) {
            remove_element = true;
        }

        // --- Touche r ---
        else if (key == static_cast<int>('r') && !editing_path) {
            rename_element = true;
        }

        // --- Touche c ---
        else if (key == static_cast<int>('c') && !editing_path) {
            copy_mode = "c";
            if (entries[selected] != "..") {
                path_to_copy = (fs::path(cwd) / entries[selected]).string();
            }
        }

        // --- Touche x ---
        else if (key == static_cast<int>('x') && !editing_path) {
            copy_mode = "x";
            if (entries[selected] != "..") {
                path_to_copy = (fs::path(cwd) / entries[selected]).string();
            }
        }

        // --- Touche v ---
        else if (key == static_cast<int>('v') && !editing_path) {
            if (!path_to_copy.empty()) {
                copying = true;
            }
        }

    } else {

        // --- Touche Entrée ---

        if (key == KEY_ENTER || key == 10 || key == 13) {

            // Choix de la nature du nouvel élément 
            if (input_new) {
                if (popup) {
                    nouveau = popup->get_selected();
                } else {
                    nouveau = 0;
                }

                if (nouveau == 0) {             // Fichier
                    using Choice = std::pair<std::string, std::string>;
                    using Choices = std::pair<Choice, Choice>;
                    popup = std::make_unique<Popup>(win, std::make_pair(9, 40), "Nouveau fichier ", std::make_pair(Choice{}, Choice{}), "Nom :", true, sharp_edges);
                } else if (nouveau == 1) {      // Dossier
                    using Choice = std::pair<std::string, std::string>;
                    using Choices = std::pair<Choice, Choice>;
                    popup = std::make_unique<Popup>(win, std::make_pair(9, 40), "Nouveau dossier ", std::make_pair(Choice{}, Choice{}), "Nom :", true, sharp_edges);
                }

                input_new = false;
                input_new_name = true;
            }

            // Choix du nom du nouvel élément
            else if (input_new_name) {
                if (popup) new_name = popup->get_text();

                if (nouveau == 0) {
                    std::ofstream file(fs::path(cwd) / new_name, std::ios::out);
                    file << "";
                    file.close();
                } else if (nouveau == 1) {
                    fs::create_directory(fs::path(cwd) / new_name);
                }

                popup.reset();
                input_new_name = false;

                refresh_entries();
            }

            // Suppression d'un élément
            else if (remove_element) {
                if (popup && popup->get_selected() == 0) {
                    if (std::string((fs::path(cwd) / entries[selected]).string()).find("/home/emile/.Corbeille") == std::string::npos) {
                        fs::rename(fs::path(cwd) / entries[selected], fs::path("/home/emile/.Corbeille") / entries[selected]);
                    } else {
                        try {
                            fs::remove_all(fs::path(cwd) / entries[selected]);
                        } catch (const fs::filesystem_error&) {
                            if (fs::is_regular_file(fs::path(cwd) / entries[selected])) {
                                fs::remove(fs::path(cwd) / entries[selected]);
                            }
                        }
                    }
                }

                popup.reset();
                remove_element = false;

                refresh_entries();
            }

            // Renommage d'un élément
            else if (rename_element) {

                if (popup) new_rename = popup->get_text();
                std::filesystem::rename(fs::path(cwd) / entries[selected], fs::path(cwd) / new_rename);

                popup.reset();
                rename_element = false;

                refresh_entries();
            }

            else if (copying) {
                copy_to_path();
                path_to_copy.clear();
                copying = false;
                popup.reset();

                refresh_entries();
            }
        }
        if (popup) {
            popup->handle_key(key);
        }
    }
}
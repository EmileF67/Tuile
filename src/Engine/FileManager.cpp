#include "Engine/FileManager.h"
#include <string>
#include <vector>
#include <filesystem>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <set>
#include <unordered_set>

#include "Engine/Cadre.h"
#include "Engine/Popup.h"


// TODO
// compatibilité git (raccourcis pour clone, commit, ect....) ??

// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==

// INIT COLORS :
// 1  | fg | Bleu  | dossier
// 2  | fg | Vert  | fichier
// 3  | fg | Cyan  | dossier sélectionné
// 4  | fg | Jaune | fichier sélectionné
// 5  | fg | Blanc | taille
// 6  | fg | Gris  | taille && cadre (gris)

// 7  | bg | Bleu  | dossier
// 8  | bg | Vert  | fichier
// 9  | bg | Cyan  | dossier sélectionné
// 10 | bg | Jaune | fichier sélectionné

// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==

// Décalage normal -> sélectionné : +2
// Décalage fg     -> bg          : +6

// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==

// #define FG_BLUE   COLOR_PAIR(1)
// #define FG_GREEN  COLOR_PAIR(2)
// #define FG_CYAN   COLOR_PAIR(3)
// #define FG_YELLOW COLOR_PAIR(4)
// #define FG_WHITE  COLOR_PAIR(5)
// #define FG_GREY   COLOR_PAIR(6)
// #define BG_BLUE   COLOR_PAIR(7)
// #define BG_GREEN  COLOR_PAIR(8)
// #define BG_CYAN   COLOR_PAIR(9)
// #define BG_YELLOW COLOR_PAIR(10)

// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==

namespace fs = std::filesystem;

// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==
// Clipboard

// void Clipboard::clear() {
//     mode = ClipboardMode::None;
//     items.clear();
// }

// bool Clipboard::empty() const {
//     return items.empty();
// }

// bool Clipboard::contains(const fs::path& item) const {
//     return std::any_of(items.begin(), items.end(),
//         [&](const ClipboardItem& it) {
//             return it.path == item;
//         });
// }

// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==

#define OFFSET_SELECTIONNE 2


// define destructor here so Popup is a complete type for destruction
FileManager::~FileManager() = default;

// --- Constructeur ---
FileManager::FileManager(WINDOW* stdscr, std::pair<int,int> x_, std::pair<int,int> y_, const std::string& start_path_, bool display_size_, bool display_dotfiles_, bool is_linux_console_)
    : win(stdscr),
      x(x_),
      y(y_),
      start_path(start_path_),
      display_size(display_size_),
      display_dotfiles(display_dotfiles_),
      is_linux_console(is_linux_console_)
{
    // On initialise chaque variable
    cwd             = start_path;       // Le chemin vers de dossier actuel
    path_input      = "";               // La chaine qui représente la recherche de l'utilisateur
    selected        = 0;                // L'indice de l'élément selectionné (entries)
    scroll_offset   = 0;                // Le scroll à ajouter lors de l'affichage des entrées

    input_new       = false;            // Si on veut le popup pour un nouvel élément
    nouveau         = 0;                // Le choix du type du nouvel élément
    input_new_name  = false;            // Si on veut le popup pour demander le nom du nouvel élément
    new_name        = "";               // Le nom entré pour le nouvel élément
    remove_element  = false;            // Si on veut retirer un élément
    rename_element  = false;            // Si on veut renommer un élément
    new_rename      = "";               // Le nom donné pour renommer l'élément
    // copying         = false;            // Si on est entrain de copier
    // copy_mode       = "";               // Le type de copie
    // path_to_copy    = "";               // Le chemin à copier
    editor          = "vim";            // L'éditeur défini par défaut
    popup.reset();                      // Va contenir toute instance de popup
    msgbox.reset();                     // Va contenir toute instance de MessageBox
    cursor_on       = false;            // Si le curseur est activé ou non
    aSpace          = true;             // Si on veut un expace en plus après l'icône
    display_icons   = true;             // Si on affiche les icônes
    display_perms   = false;            // Si on affiche les permissions de chaque élément
    Clipboard clipboard;                // Un instance du Clipboard
}


// Renvoie true dans le cas où le fichier passé en argument est une image
//  vérifie si l'extension est connue dans une liste d'extensions d'images
bool is_image_file(const std::string& filename) 
{
    // Déclaration
    static const std::set<std::string> image_exts = {
        "bmp","dib","jpg","jpeg","jpe","png","gif",
        "tif","tiff","webp","heif","heic","ico","cur",
        "svg","svgz","psd","cr2","nef","arw","dng","rw2"
    };
    std::string ext;

    // On trouve la dernière occurence d'un '.' dans le string
    size_t pos = filename.find_last_of('.');
    if (pos == std::string::npos) return false;     // Si il n'est pas trouvé on retourne false

    // On extrait les derniers caractères après le '.' trouvé.
    ext = filename.substr(pos + 1);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);     // On transforme en lowercase l'extension

    return image_exts.count(ext) > 0;
}

// Transforme un nombre d'octets en version lisible
std::string FileManager::human_readable_size(long long size)
{
    // Déclaration
    const char* units[] = {"B", "K", "M", "G", "T", "P"};
    double s = static_cast<double>(size);     // pour garder les décimales
    int unit = 0;
    char buf[64];

    // La boucle va trouver l'unitée adéquoite en divisant le nombre tant que possible
    while (s >= 1024.0 && unit < 5) {
        s /= 1024.0;
        unit++;
    }

    // Si on a que des octets, on les affiche simplement
    if (unit == 0) {
        std::snprintf(
            buf,
            sizeof(buf),
            "%lld %s",
            static_cast<long long>(size),
            units[unit]
        );

    // Sinon on l'affiche avec un seul chiffre après la virgule
    } else {
        std::snprintf(
            buf,
            sizeof(buf),
            "%.1f %s",
            s,
            units[unit]
        );
    }

    return std::string(buf);    // Convertit buffer en string
}


// On compare le texte mais aussi les chiffres. Donc file2 apparaitra avant file10.
// Comparaison naturelle != Comparaison lexicographique
static bool naturalCompare(const std::string& a, const std::string& b)
{
    // Déclaration
    size_t i = 0, j = 0;

    // On ne dépasser pas la taille max des deux chaines
    while (i < a.size() && j < b.size()) {
        // Si les deux caractères sont des nombres
        if (std::isdigit(a[i]) && std::isdigit(b[j])) {
            // Lire les nombres pour en connaître les coordonnées de début et de fin dans chaque string
            size_t i_start = i;
            while (i < a.size() && std::isdigit(a[i])) ++i;
            size_t j_start = j;
            while (j < b.size() && std::isdigit(b[j])) ++j;
            
            // On les transforme en nombres entier
            int numA = std::stoi(a.substr(i_start, i - i_start));
            int numB = std::stoi(b.substr(j_start, j - j_start));
            
            // Si ils ne sont pas égaux
            if (numA != numB)
                return numA < numB;
        } else {
            // Comparaison char par char classique
            if (a[i] != b[j])
                return a[i] < b[j];
            // On avance
            ++i;
            ++j;
        }
    }
    // Si une chaine est un préfixe de l'autre : "file" < "file1"
    return a.size() < b.size();
}


// Renvoie une liste triée de dossiers, puis de fichiers (avec comparaison naturelle)
std::vector<std::string> FileManager::tri_dossiers_fichiers(const std::vector<std::string>& lst)
{
    // Déclaration
    std::vector<std::string> lstdir;
    std::vector<std::string> lstfile;

    // Sépare les fichiers et les dossiers
    for (const auto& e : lst) {
        fs::path p = fs::path(cwd) / e;
        std::error_code ec;
        bool is_dir = fs::is_directory(p, ec);
        if      (ec)     { lstfile.push_back(e); } 
        else if (is_dir) { lstdir.push_back(e);  } 
        else             { lstfile.push_back(e); }
    }

    // Trie la liste des fichiers, puis des dossiers indépendemment et avec la comparaison naturelle
    std::sort(lstdir.begin(), lstdir.end(), naturalCompare);
    std::sort(lstfile.begin(), lstfile.end(), naturalCompare);

    // Créer un nouveau vecteur et alloue de la mémoire pour combiner la liste des dossiers et des fichiers
    std::vector<std::string> result;
    result.reserve(lstdir.size() + lstfile.size());
    result.insert(result.end(), lstdir.begin(), lstdir.end());
    result.insert(result.end(), lstfile.begin(), lstfile.end());

    return result;
}


// Recréer la liste des entrées (dossiers et fichiers contenu dans le chemin actuel (cwd))
void FileManager::refresh_entries()
{
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

        // Appeler la fonction de tri
        items = tri_dossiers_fichiers(items);

        // Filtrage dynamique en fonction de la chaine entrée dans l'input
        if (editing_path) {
            // Si on veut les fichiers cachés
            if (display_dotfiles) {
                for (const auto& e : items) {
                    // Si l'élément actuel commence par <path_input>
                    if (e.rfind(path_input, 0) == 0) {
                        temp.push_back(e);
                    }
                }
            } else {
                for (const auto& e : items) {
                    // Si l'élément actuel commence par <path_input>
                    if (e.rfind(path_input, 0) == 0 && e[0] != '.' && e != "__pycache__") {
                        temp.push_back(e);
                    }
                }
            }

        // Filtrage normal
        } else {
            // Si on veut les fichiers cachés
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

        // On actualise la liste des entrées
        entries = temp;

    } catch (const fs::filesystem_error& e) {
        entries.clear();                                                      // Si une erreur se présente on efface la liste des entrées
        entries.push_back("..");                                              // On ajoute un moyen de retour
        entries.push_back("Permission Error, or another one, idk. (gl :3)");  // On ajoute l'indication d'une erreur
    }

    // Finally we modify the selected entry
    if (!entries.empty()) {
        selected = std::min(selected, static_cast<int>(entries.size()) - 1);
    } else {
        selected = 0;
    }
}


void FileManager::copy_to_path()
{
    // Recopier la fonction depuis python
    try {
        fs::path path_to_copy;
        fs::path dest_path;

        // Éviter d’écraser un fichier déjà existant
        // if (fs::exists(dest_path))
            // return; // TODO changer ça pour afficher le problème

        for (const auto& path_to_copy : clipboard.items) {
            fs::path dest_path = fs::path(cwd) / path_to_copy.filename();
            switch (clipboard.mode) {
                case ClipboardMode::Copy:
                    if (fs::is_directory(path_to_copy)) 
                        fs::copy(path_to_copy, dest_path, fs::copy_options::recursive);
                    else 
                        fs::copy_file(path_to_copy, dest_path);
                    break;
                case ClipboardMode::Cut:
                    if (fs::exists(dest_path)) {
                        throw std::runtime_error("Impossible de couper coller [" + path_to_copy.string() + "] vers [" + dest_path.string() + "] : l'élément existe déjà");
                    }
                    fs::rename(path_to_copy, dest_path);
                    break;

                default :
                    break;
            }
        }
        
        clipboard.clear();

    } catch (const std::exception& e) {
        // std::cerr << "Erreur lors de la copie : " << e.what() << std::endl;
        if (!msgbox) {
            msgbox = std::make_unique<MessageBox>(win, std::string("Erreur lors de la copie :\n") + e.what(), is_linux_console);
        }
        clipboard.clear();
    }
}


std::string obtenir_permissions(const fs::path& p)
{
    std::error_code ec;
    fs::file_status status = fs::status(p, ec);

    if (ec) {
        std::cerr << "Erreur : " << ec.message() << '\n';
        return std::string("");
    }

    fs::perms perm = status.permissions();

    auto check = [&](fs::perms bit) -> bool {
        return (perm & bit) != fs::perms::none;
    };


    std::string tempperm;
    tempperm += (check(fs::perms::owner_read)   ? 'r' : '-');
    tempperm += (check(fs::perms::owner_write)  ? 'w' : '-');
    tempperm += (check(fs::perms::owner_exec)   ? 'x' : '-');
    tempperm += " ";
    tempperm += (check(fs::perms::group_read)   ? 'r' : '-');
    tempperm += (check(fs::perms::group_write)  ? 'w' : '-');
    tempperm += (check(fs::perms::group_exec)   ? 'x' : '-');
    tempperm += " ";
    tempperm += (check(fs::perms::others_read)  ? 'r' : '-');
    tempperm += (check(fs::perms::others_write) ? 'w' : '-');
    tempperm += (check(fs::perms::others_exec)  ? 'x' : '-');
    tempperm += " ";

    return tempperm;
}


void FileManager::draw()
{
    // Déclaration & initialisation
    int _, cols;
    getmaxyx(win, _, cols);

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

    // Affiche le principale du filemanager
    std::string display_text = "";
    this->draw_header(x1, y1, x2, y2, &display_text);

    // Afficher les entrées
    this->draw_entries(x1, y1, x2, cols);

    // Si l'on est entrain de modifier le chemin manuellement
    if (editing_path) {
        wmove(win, x1, y1 + static_cast<int>(display_text.size()));
    }

    this->draw_popups();

    this->draw_messagebox();
    
}

void FileManager::draw_messagebox()
{
    if (msgbox) {
        msgbox->draw();
        if (msgbox->is_entered()) {
            msgbox.reset();
        }
    }
}


void FileManager::draw_header(int x1, int y1, int x2, int y2, std::string* display_text)
{
    // Cadre
    Cadre cadre(win, {x1 - 1, y1 - 1}, {x2 + 1, y2 + 1}, is_linux_console);
    cadre.draw();
    cadre.sep(x1 + 1);

    // Afficher le chemin
    if (editing_path) {
        if (cwd == "/") {
            *display_text = " " + cwd + path_input;
        } else {
            *display_text = " " + cwd + "/" + path_input;
        }
    } else {
        *display_text = " " + cwd + " ";
    }
    mvwaddstr(win, x1, y1, (*display_text).c_str());
}

void FileManager::draw_entries(int x1, int y1, int x2, int cols)
{
    // Déclaration
    int color = 0;
    std::string icon;
    int abs_idx;
    std::string entry;
    EntryDisplay details;
    size_t start = static_cast<size_t>(scroll_offset);
    size_t end = std::min(entries.size(), start + static_cast<size_t>(x2-x1 - 1));
    std::vector<std::string> visible_entries(entries.begin() + start, entries.begin() + end);
    
    for (std::size_t i = 0; i < visible_entries.size(); i++) {
        abs_idx = static_cast<int>(start + i);
        entry   = visible_entries[i];
        details = this->get_entry_display_info(entry);

        // On détermine si l'élément est sélectionné et on applique des changements en fonction
        std::string prefix = "";

        fs::path total_path = fs::path(cwd) / entries[abs_idx];
        if (clipboard.contains(total_path)) {
            prefix += "*";
        } else {
            prefix += " ";
        }
        
        std::string offset = "";
        if (abs_idx == selected) {
            prefix += ">";
            details.color += OFFSET_SELECTIONNE; // cyan ou vert
            details.color_bg += OFFSET_SELECTIONNE; // cyan ou vert mais en background
            offset = " ";
        } else {
            prefix += "";
        }

        prefix += " ";


        // Afficher la ligne de l'entrée
        color = details.color;
        std::string text = prefix + details.permissions + "    " + entry;
        wattron(win, COLOR_PAIR(color));
        mvwaddstr(win, x1 + 2 + static_cast<int>(i), y1, text.c_str());
        wattroff(win, COLOR_PAIR(color));

        // Déterminer le type d'affichage de l'icone et appliquer des changements
        color = details.color;
        icon = details.icon;
        if (is_linux_console || !display_icons)
        {
            color = details.color_bg;
            icon = icon.substr(0, 1);
        }
        
        // Si on affiche les permissions de chaque élément, on décale l'icone de 12 cases
        // (exemple de perms : "rwx rwx rwx ")
        int enPlus = 0;
        if (display_perms) {
            enPlus = 12;
        }

        // Afficher l'icone
        wattron(win, COLOR_PAIR(color));
        mvwaddstr(win, x1 + 2 + static_cast<int>(i), y1 + 3 + enPlus, icon.c_str());
        wattroff(win, COLOR_PAIR(color));

        // Calculer couleur de l'affichage de la taille de l'élément
        if (abs_idx == selected) {
            color = 4; // jaune
        } else {
            if (is_linux_console) {
                color = 5; // gris
            } else {
                color = 6; // blanc
            }
        }

        // Afficher taille de l'élément
        wattron(win, COLOR_PAIR(color));
        mvwaddstr(win, x1 + 2 + static_cast<int>(i), y1 + cols - 10 - static_cast<int>(details.size.size()), details.size.c_str()); // TODO
        wattroff(win, COLOR_PAIR(color));
    }
}

void FileManager::draw_popups()
{
    // Si l'on est entrain de faire un choix pour un nouvel élément
    if (input_new) {
        if (!popup) {
            using Choice = std::pair<std::string, std::string>;
            using Choices = std::pair<Choice, Choice>;
            Choices choix = {{"Fichier", " "}, {"Dossier", " "}};
            popup = std::make_unique<Popup>(win, std::make_pair(9, 40), "Nouveau", choix, "", true, is_linux_console);
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
            popup = std::make_unique<Popup>(win, std::make_pair(9, 90), label, choix, "", true, is_linux_console);
            // default selected to 1 (second option)
        }
        if (popup) popup->draw();
    }

    // Si l'on est entrain de renommer un élément
    if (rename_element) {
        if (!popup) {
            std::string label = "Renommer " + entries[selected];
            popup = std::make_unique<Popup>(win, std::make_pair(9, 50), label, std::make_pair(std::make_pair(std::string(), std::string()), std::make_pair(std::string(), std::string())), "Nouveau nom :", true, is_linux_console);
        }
        if (popup) popup->draw();
    }

    // Si l'on est entrain de copier/couper un élément TODO
    if (copying) {
        if (!popup) {
            std::string label;
            switch (clipboard.mode) {
                case ClipboardMode::Copy :
                    label = "Copier et Coller " + std::filesystem::path(path_to_copy).filename().string() + " ici ?";
                    break;

                case ClipboardMode::Cut :
                    label = "Couper et Coller " + std::filesystem::path(path_to_copy).filename().string() + " ici ?";
                    break;

                default :
                    break;
            }

            using Choice = std::pair<std::string, std::string>;
            using Choices = std::pair<Choice, Choice>;
            Choices choix = {{"Oui", ""}, {"Non", ""}};
            popup = std::make_unique<Popup>(win, std::make_pair(9, 90), label, choix, "", true, is_linux_console);
        }
        if (popup) popup->draw();
    }
}

EntryDisplay FileManager::get_entry_display_info(std::string entry)
{
    // Déclaration & initialisation
    fs::path full_path = fs::path(cwd) / entry;
    std::string size_str = "";
    int color = 5;
    int color_bg = 5;
    std::string icon = "";


    // Use the non-throwing overload to detect directory status. If we
    // can't stat the entry (permission denied), we display it but mark
    // its size as unknown.
    std::error_code ec;
    bool is_dir = fs::is_directory(full_path, ec);

    // Si il y a une erreur
    if (ec)
    {
        icon = "? ";
        if (is_linux_console || !display_icons) {
            icon = "  ";
        }

        size_str = "?";
    }

    // Si c'est un dossier
    else if (is_dir)
    {
        color = 1;
        color_bg = 7;

        icon = "🖿  ";
        if (is_linux_console || !display_icons) {
            icon = "D ";
        }

        size_str = "";
    }

    // Si c'est un fichier
    else
    {
        // Essayer d'obtenir la taille de l'élément.
        long long temp_size = 0;
        try {
            auto sz = std::filesystem::file_size(full_path);
            temp_size = static_cast<long long>(sz);
            size_str = human_readable_size(temp_size);
        } catch (const std::filesystem::filesystem_error&) {
            size_str = "?";
        }

        // Identification exacte de l'élément
        color = 2;
        color_bg = 8;
        if (is_linux_console || !display_icons) {
            icon = "F ";
        } else {
            // TODO : utilser la ligne juste en dessous ?
            // std::string ext = std::filesystem::path(entry).extension().string();
            if (temp_size == 0) {
                icon = "🗋 ";
            } else if (is_image_file(entry)) {
                icon = "🖻 ";
            } else {
                icon = "🗎 ";
            }

            if (aSpace) {
                icon = icon + " ";
            }
        }
    }

    // Obtenir les permissions de l'élément
    std::string permissions = "";
    if (!ec && display_perms) {
        permissions = obtenir_permissions(full_path);
    }

    // On créer une nouvelle instance des détails de <entry>
    EntryDisplay entry_details;

    // On remplis les détails
    entry_details.icon = icon;
    entry_details.size = size_str;
    entry_details.color = color;
    entry_details.color_bg = color_bg;
    entry_details.permissions = permissions;

    return entry_details;
}



void FileManager::handle_key(int key)
{

    // On gère pour la message box
    if (msgbox) {
        msgbox->handle_key(key);
        if (msgbox->is_entered()) {
            msgbox.reset();
        }
        // Tant qu'une messagebox est affichée, on ne propage pas la touche au FileManager
        return;
    }

    int rows, cols;
    getmaxyx(win, rows, cols);
    // Compute the number of visible rows inside this FileManager window
    // The draw() function uses (x2 - x1 - 1) as the number of visible entries,
    // so keep the same value here to keep scrolling logic consistent.
    int x1 = x.first;
    int x2 = y.first;
    int view_rows = x2 - x1 - 1;
    if (view_rows <= 0) {
        // Fallback to terminal rows if the computed area is invalid
        view_rows = rows;
    }
    const int MINCHAR = 32;
    const int MAXCHAR = 126;

    int scroll_margin = 3;

    if (key == KEY_F(2)) {
        display_icons = !display_icons;
        refresh_entries();
    }

    if (key == KEY_F(3)) {
        display_perms = !display_perms;
        refresh_entries();
    }

    if (editing_path) {
        if (key == KEY_BACKSPACE || key == 127) {
            if (!path_input.empty()) path_input = path_input.substr(0, path_input.size() - 1);
            selected = 1;
            scroll_offset = 0;
        } else if (MINCHAR <= key && key <= MAXCHAR) {
            path_input += static_cast<char>(key);
            selected = 1;
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
            if (selected >= scroll_offset + view_rows - scroll_margin) {
                scroll_offset = std::min(
                    std::max(0, static_cast<int>(entries.size()) - view_rows),
                    selected - view_rows + scroll_margin + 1
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
            if (editing_path && !cursor_on) {
                curs_set(1);
                cursor_on = true;
            } else {
                curs_set(0);
                cursor_on = false;
            }
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

                } else {
                    // Try to detect if target is a directory. fs::is_directory
                    // can throw (e.g. permission denied). Protect the UI from
                    // exceptions and show a permission error instead.
                    try {
                        if (fs::is_directory(new_path)) {
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
                    } catch (const fs::filesystem_error& e) {
                        std::cerr << "handle_key: cannot open directory '" << new_path.string() << "': " << e.what() << std::endl;
                        entries.clear();
                        entries.push_back("..");
                        entries.push_back("PERMISSION ERROR");
                        selected = 0;
                        scroll_offset = 0;
                    }
                }
            }

            if (editing_path) {
                path_input = "";
            }

            refresh_entries();
        }

        // --- Touche Page Down ---
        // if (key == KEY_NPAGE) {
        //     selected = std::min(static_cast<int>(entries.size()) - 1, selected + view_rows);
        //     scroll_offset = std::min(
        //         std::max(0, static_cast<int>(entries.size()) - view_rows),
        //         selected - view_rows + 1
        //     );
        // }

        // --- Touche Page Down ---
        // if (key == KEY_NPAGE) {
        //     int entry_count = static_cast<int>(entries.size());


        //     if (entry_count == 0)
        //         return;

        //     selected = std::min(max_selected, selected + view_rows);

        //     scroll_offset = std::clamp(
        //         selected - view_rows + 1,
        //         0,
        //         std::max(0, entry_count - view_rows)
        //     );
        // }

        // --- Touche Page Down ---
        if (key == KEY_NPAGE) {
            int entry_count = static_cast<int>(entries.size());
            if (entry_count == 0)
                return;

            selected = std::min(entry_count - 1, selected + view_rows);

            scroll_offset = std::clamp(
                selected - view_rows + 1,
                0,
                std::max(0, entry_count - view_rows)
            );
        }

        // --- Touche Page Up ---
        else if (key == KEY_PPAGE) {
            selected = std::max(0, selected - view_rows);
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
            scroll_offset = std::max(0, static_cast<int>(entries.size()) - view_rows);
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
            clipboard.mode = ClipboardMode::Copy;
        }

        // --- Touche x ---
        else if (key == static_cast<int>('x') && !editing_path) {
            clipboard.mode = ClipboardMode::Cut;
        }

        // --- Touche v ---
        else if (key == static_cast<int>('v') && !editing_path) {
            if (!path_to_copy.empty()) {
                copying = true;
            }
        }

        // --- Touche Espace ---
        else if (key == ' ' && !editing_path) {
            if (entries[selected] != "..") {
                path_to_copy = fs::path(cwd) / entries[selected];
                clipboard.toggle(path_to_copy);
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
                    popup = std::make_unique<Popup>(win, std::make_pair(9, 40), "Nouveau fichier ", std::make_pair(Choice{}, Choice{}), "Nom :", true, is_linux_console);
                } else if (nouveau == 1) {      // Dossier
                    using Choice = std::pair<std::string, std::string>;
                    popup = std::make_unique<Popup>(win, std::make_pair(9, 40), "Nouveau dossier ", std::make_pair(Choice{}, Choice{}), "Nom :", true, is_linux_console);
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

#include "Engine/Components/Popup2.h"
#include "Engine/Utils/StringUtils.h"
#include <ncurses.h>
#include <string>

#define FG_GREEN  COLOR_PAIR(2)
#define FG_WHITE  COLOR_PAIR(5)



// $==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==
// $ Partie base d'un popup
// $==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==
Popup::Popup(WINDOW* stdscr_, std::string titre_, std::string label_, bool is_linux_console_)
    :
    stdscr(stdscr_),
    entered(false),
    is_linux_console(is_linux_console_),
    titre(titre_),
    label(label_)
{
    // On récupère la taille du terminal pour calculer la taille de la fenêtre
    getmaxyx(stdscr, rows, cols);

    // On fait les calculs de taille
    height = rows / 3;
    width  = 2 * cols / 3;

    int y = rows/3;
    int x = (cols/3)/2;

    // On créer notre fenêtre
    win = newwin(height, width, y, x);

    // On créer notre instance du cadre
    cadre = std::make_unique<Cadre>(
        win,
        std::make_pair(0, 0),
        std::make_pair(height-1, width-1),
        is_linux_console
    );
    cadre->sep(2);
}


Popup::~Popup() {
    delwin(win);
}


void Popup::draw_base()
{
    // On affiche le cadre
    cadre->draw();

    // On affiche le titre du popup
    mvwaddstr(win, 1, 3, titre.c_str());

    // On prépare le texte pour qu'il rentre dans le popup
    std::vector<std::string> texte_final = string_utils::split_text(label, width - 6);

    // On affiche le label
    int y = 4;
    for (const auto& ligne : texte_final) {
        mvwaddstr(win, y, 3, ligne.c_str());
        y++;
    }
}


bool Popup::handle_key_base(int key)
{
    bool res = false;

    // --- Touche Entrer ---
    if (key == KEY_ENTER || key == 10 || key == 13) {
        entered = !entered;
        res = true;
    }

    return res;
}


// $==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==
// $ Partie popup d'information
// $==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==
PopupInfo::PopupInfo(WINDOW* stdscr_, std::string label_, bool is_linux_console_)
    :
    Popup(stdscr_, "Information", label_, is_linux_console_)
{
    
}

PopupInfo::~PopupInfo() = default;

void PopupInfo::draw()
{
    // On affiche la base du popup
    draw_base();

    // On prépare le message indiquant qu'il faut appuyer sur entrer pour continuer
    std::string message = "Appuyez sur <Entrer> pour continuer";

    // On affiche le message indiquant qu'il faut appuyer sur entrer pour continuer
    mvwaddstr(win, height - 2, width - 2 - static_cast<int>(message.size()), message.c_str());
}


void PopupInfo::handle_key(int key) 
{
    handle_key_base(key);
}


// $==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==
// $ Partie popup d'entrée de texte
// $==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==
PopupInputText::PopupInputText(WINDOW* stdscr_, std::string label_, bool is_linux_console_)
    :
    Popup(stdscr_, "Demande de texte", label_, is_linux_console_)
{
    input = std::make_unique<Input>(win, 6, 2, width - 4, true);
}

PopupInputText::~PopupInputText() = default;

void PopupInputText::draw()
{
    draw_base();

    input->draw();
}


void PopupInputText::handle_key(int key)
{
    // --- Touches de base ---
    bool base = handle_key_base(key);

    // Si aucune touches de base n'ont été reconnues
    if (!base) {
        // --- Touches de l'input ---
        input->handle_key(key);
    }
}


// $==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==
// $ Partie popup de choix double
// $==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==
PopupDoubleChoices::PopupDoubleChoices(WINDOW* stdscr_, std::string label_, Choices choices_, bool is_linux_console_)
    :
    Popup(stdscr_, "Choix entre deux possibilités", label_, is_linux_console_),
    choices(choices_),
    selected(0)
{
    
}

PopupDoubleChoices::~PopupDoubleChoices() = default;

void PopupDoubleChoices::draw()
{
    // On affiche la base du popup
    draw_base();

    // On affiche les deux choix
    draw_both_choices();
}



void PopupDoubleChoices::draw_both_choices()
{
    // On calcule l'endroit centré de nos deux choix
    int pos_left     = width/3   + 2;
    int pos_right    = 2*width/3 + 2;

    // On calcule le décalage en fonction du texte dans .second de chaque Choice
    int start_left   = pos_left  - static_cast<int>(choices.first.second.size())/2;
    int start_right  = pos_right - static_cast<int>(choices.second.second.size())/2;

    // On calcule le milieu vertical du popup
    int mid_vertical = height / 2;

    // On calcule la couleur en fonction de qui est séléctionné
    auto first_color  = (selected == 0) ? FG_GREEN : FG_WHITE;
    auto second_color = (selected == 1) ? FG_GREEN : FG_WHITE;

    // On affiche le premier choix
    wattron(win, first_color);
    mvwaddstr(win, mid_vertical-1, pos_left, choices.first.first.c_str());
    mvwaddstr(win, mid_vertical, start_left, choices.first.second.c_str());
    wattroff(win, first_color);

    // On affiche le second choix
    wattron(win, second_color);
    mvwaddstr(win, mid_vertical-1, pos_right, choices.second.first.c_str());
    mvwaddstr(win, mid_vertical, start_right, choices.second.second.c_str());
    wattroff(win, second_color);
}


void PopupDoubleChoices::handle_key(int key)
{
    // --- Touches de base ---
    bool base = handle_key_base(key);

    // Si aucune touches de base n'ont été reconnues
    if (!base) {
        // --- Touche Flèche gauche ---
        if (key == KEY_LEFT) {
            selected = 0;
        }
    
        // --- Touche Flèche droite ---
        else if (key == KEY_RIGHT) {
            selected = 1;
        }
    }

}
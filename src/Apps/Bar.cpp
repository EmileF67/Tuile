#include "Apps/Bar.h"



Bar::Bar(WINDOW* stdscr_)
    : stdscr(stdscr_), is_linux_console(false)
{
    // On récupère la taille actuelle de la fenêtre
    getmaxyx(stdscr, rows, cols);

    // On créer un window qui correspond à la barre
    int h = 3;
    int w = cols-1;
    int y = 0;
    int x = 0;

    win = newwin(h, w, y, x);

    // On créer le cadre qui représente la barre
    cadre = Cadre(win, {0, 0}, {2, cols-2}, is_linux_console);
}


Bar::~Bar() {
    delwin(win);
}


void Bar::ajout_module(std::unique_ptr<Module> m, BarArea area)
{
    m->set_win(win);

    switch (area) {
        case BarArea::Left:
            left.push_back(std::move(m));
            break;

        case BarArea::Middle:
            middle.push_back(std::move(m));
            break;

        case BarArea::Right:
            right.push_back(std::move(m));
            break;
    }
}




void Bar::draw()
{
    // Nettoyer la fenêtre
    // werase(win);

    // On dessine le cadre
    cadre.draw();

    // On affiche les modules de gauche
    draw_left();

    // On affiche les modules du milieu
    draw_middle();

    // On affiche les modules de droite
    draw_right();
}




void Bar::draw_left()
{
    int cursor = 1;  // Commencer après le bord gauche du cadre
    for (auto& m : left) {
        if (m->should_be_drawn()) {
            m->draw(cursor);
        }
        cursor += m->size;
        if (cursor < cols - 1) {  // Vérifier qu'on ne dépasse pas
            cadre.cut_vertical(cursor);
        }
        cursor++;
    }
}


void Bar::draw_middle()
{
    // Calcul de la taille totale accumulée des modules du milieu
    int total_size = 0;
    for (auto& m : middle) {
        total_size += m->size + 1;  // +1 pour l'espaceur
    }
    if (total_size > 0) total_size--;  // Retirer le dernier espaceur

    // Largeur disponible pour le contenu (entre les bords)
    int available_width = cols - 3;  // cols pour la fenêtre, -2 pour les bords, -1 pour marge
    
    // Le curseur commence au milieu moins la moitié de la taille totale
    int cursor = 1 + (available_width / 2) - (total_size / 2);

    cadre.cut_vertical(cursor-1);

    // On affiche chaque module un par un
    for (auto& m : middle) {
        if (m->should_be_drawn()) {
            m->draw(cursor);
        }
        cursor += m->size;
        if (cursor < cols - 1) {
            cadre.cut_vertical(cursor);
        }
        cursor++;
    }
}


void Bar::draw_right()
{
    // Commencer par la droite et remplir vers la gauche
    int cursor = cols - 2 - 1;  // Position de droite               -1 car décalage, jsp
    for (int i = static_cast<int>(right.size()) - 1; i >= 0; i--) {
        auto& m = right[i];
        cursor -= m->size;  // Décrémenter d'abord pour obtenir la position gauche du module
        if (cursor > 1) {  // Vérifier qu'on ne dépasse pas le bord gauche
            if (m->should_be_drawn()) {
                m->draw(cursor);
            }
            cursor--;
            cadre.cut_vertical(cursor);  // Séparation à gauche du module
        }
        cursor--;  // Espacement
    }
}
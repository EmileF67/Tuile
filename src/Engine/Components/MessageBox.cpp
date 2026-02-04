#include "Engine/Components/MessageBox.h"
#include "ncurses.h"
#include <vector>
#include <sstream>
#include <algorithm>
#include "Engine/Components/Cadre.h"





MessageBox::MessageBox(WINDOW* win_, const std::string& message_, bool is_linux_console_) 
    : win(win_), 
      message(message_),
      is_linux_console(is_linux_console_),
      rows(0), cols(0), x1(0), y1(0), x2(0), y2(0), entered(false)
{
    // Nothing heavy in constructor: size & placement are computed at draw time
    // so the message box adapts to terminal resizes and message length.
}



// Gère les sauts de ligne explicites et le découpage en mots
std::vector<std::string> MessageBox::split_text(size_t taille_max)
{
    std::vector<std::string> res;
    if (taille_max == 0) return res;

    size_t start = 0;
    const std::string& s = message;
    while (start <= s.size()) {
        size_t pos = s.find('\n', start);
        std::string paragraph = (pos == std::string::npos) ? s.substr(start) : s.substr(start, pos - start);

        // Utilise un istringstream pour itérer par mot
        std::istringstream iss(paragraph);
        std::string word;
        std::string line;
        while (iss >> word) {
            if (line.empty()) {
                if (word.size() <= taille_max) {
                    line = word;
                } else {
                    // Mot trop long -> on le coupe
                    size_t p = 0;
                    while (p < word.size()) {
                        size_t len = std::min(taille_max, word.size() - p);
                        res.push_back(word.substr(p, len));
                        p += len;
                    }
                }
            } else {
                if (line.size() + 1 + word.size() <= taille_max) {
                    line += " " + word;
                } else {
                    res.push_back(line);
                    if (word.size() <= taille_max) {
                        line = word;
                    } else {
                        size_t p = 0;
                        while (p < word.size()) {
                            size_t len = std::min(taille_max, word.size() - p);
                            res.push_back(word.substr(p, len));
                            p += len;
                        }
                        line.clear();
                    }
                }
            }
        }

        if (!line.empty()) {
            res.push_back(line);
        }

        if (pos == std::string::npos) break;
        // Conserver un saut de ligne explicite (ligne vide)
        res.push_back(std::string());
        start = pos + 1;
    }

    if (res.empty()) res.push_back(std::string());
    return res;
}




void MessageBox::draw() {
    // Recalculer la taille du terminal (gestion du redimensionnement)
    getmaxyx(win, rows, cols);

    // On prépare le découpage du texte avec une largeur maximale raisonnable
    size_t max_content_width = (cols > 8) ? static_cast<size_t>(cols - 8) : 1;
    std::vector<std::string> lines = this->split_text(max_content_width);

    // Calculer la largeur effective du contenu
    int content_width = 0;
    for (const auto& l : lines) content_width = std::max(content_width, static_cast<int>(l.size()));

    // Taille de la boite avec marges
    int box_w = std::min(cols - 4, content_width + 4) + 2;
    int box_h = std::min(rows - 4, static_cast<int>(lines.size()) + 4);

    // Construire et dessiner le cadre centré
    x1 = std::max(1, (rows - box_h) / 2);
    y1 = std::max(1, (cols - box_w) / 2);
    x2 = x1 + box_h - 1;
    y2 = y1 + box_w - 1;

    cadre = std::unique_ptr<Cadre>(new Cadre(win, {x1, y1}, {x2, y2}, is_linux_console));
    
    // Effacer la zone intérieure avant d'écrire
    for (int r = x1 + 1; r < x2; ++r) {
        mvwaddstr(win, r, y1 + 1, std::string(std::max(0, box_w - 2), ' ').c_str());
    }
    
    // Afficher le message (limité à la hauteur disponible)
    int max_lines = box_h - 4;
    for (int i = 0; i < std::min(static_cast<int>(lines.size()), max_lines); ++i) {
        mvwaddstr(win, x1 + 2 + i, y1 + 2, lines[i].c_str());
    }
    
    cadre->draw();
}



void MessageBox::handle_key(int key) {
    if (key == KEY_ENTER || key == 10 || key == 13) {
        entered = true;
    }
}
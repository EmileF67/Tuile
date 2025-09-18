#include "Engine/Cadre.h"
#include <string>
#include <stdexcept>

// Helper local function: répète une chaîne UTF-8 'n' fois
static std::string repeatUtf8(const std::string& s, int n) {
    std::string out;
    out.reserve(s.size() * (n > 0 ? n : 0));
    for (int i = 0; i < n; ++i) out += s;
    return out;
}

Cadre::Cadre(std::pair<int,int> x_, std::pair<int,int> y_, WINDOW* stdscr)
    : x(x_), y(y_), win(stdscr) { }

void Cadre::draw() {
    int r1 = x.first;
    int c1 = x.second;
    int r2 = y.first;
    int c2 = y.second;

    if (r2 - r1 < 1) throw std::runtime_error("Taille verticale trop petite");
    if (c2 - c1 < 2) throw std::runtime_error("Taille horizontale trop petite (>=2)");

    int inner = c2 - c1 - 2; // nombre de '─' à dessiner entre les coins
    std::string horiz = repeatUtf8(u8"─", inner);

    std::string top = std::string(u8"╭") + horiz + std::string(u8"╮");
    std::string bot = std::string(u8"╰") + horiz + std::string(u8"╯");

    mvwaddstr(win, r1, c1, top.c_str());
    mvwaddstr(win, r2, c1, bot.c_str());

    // Colonnes verticales
    for (int r = r1 + 1; r < r2; ++r) {
        mvwaddstr(win, r, c1, u8"│");
        mvwaddstr(win, r, c2 - 1, u8"│");
    }
}

void Cadre::sep(int row) {
    int inner = y.second - x.second - 2;
    if (inner < 0) return;
    std::string line = std::string(u8"├") + repeatUtf8(u8"─", inner) + std::string(u8"┤");
    mvwaddstr(win, row, x.second, line.c_str());
}

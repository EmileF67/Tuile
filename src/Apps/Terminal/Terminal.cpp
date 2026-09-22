#include "Apps/Terminal/Terminal.h"
#include <algorithm>
#include <array>
#include <cstring>
#include <unordered_map>

namespace {
constexpr short terminal_pair_base = 20;

std::unordered_map<unsigned int, short> terminal_pairs;
short next_terminal_pair = 84;

short color_pair_for(const Cell& cell)
{
    const short fg = static_cast<short>(std::clamp(cell.fg, static_cast<short>(0), static_cast<short>(255)));
    const short bg = static_cast<short>(std::clamp(cell.bg, static_cast<short>(0), static_cast<short>(255)));
    const unsigned int key = (static_cast<unsigned int>(fg) << 16) |
                             static_cast<unsigned int>(bg);
    const auto existing = terminal_pairs.find(key);
    if (existing != terminal_pairs.end())
        return existing->second;

    if (next_terminal_pair >= COLOR_PAIRS)
        return terminal_pair_base;

    const short pair_id = next_terminal_pair++;
    if (init_extended_pair(pair_id, fg, bg) == ERR)
        return terminal_pair_base;
    terminal_pairs.emplace(key, pair_id);
    return pair_id;
}

void send_sequence(Pty& pty, const char* sequence)
{
    pty.write_input(sequence, std::strlen(sequence));
}
}

Terminal::Terminal(WINDOW* window, MainEngine& main_engine, int row_count,
                   int column_count, const std::string& shell)
    : engine(main_engine), win(window), rows(std::max(1, row_count)),
      cols(std::max(1, column_count)), focused(false),
      pty(std::make_unique<Pty>()),
      buf(std::make_unique<TermBuffer>(rows, cols)),
      parser(std::make_unique<VtParser>(*buf))
{
    if (win != nullptr)
        getmaxyx(win, rows, cols);
    rows = std::max(1, rows);
    cols = std::max(1, cols);
    buf->resize(rows, cols);
    pty->spawn(rows, cols, shell);
}

// Terminal::Terminal() { }

void Terminal::poll()
{
    if (pty == nullptr || parser == nullptr)
        return;

    char buffer[4096];

    while (true) {
        ssize_t n = pty->read_output(buffer, sizeof(buffer));

        if (n <= 0)
            break;

        parser->feed(buffer, n);
    }
}


void Terminal::draw()
{
    if (win == nullptr || buf == nullptr)
        return;

    werase(win);
    const bool use_colors = has_colors() && COLORS >= 256 && COLOR_PAIRS > 84;
    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            const Cell& cell = buf->at(row, col);
            if (use_colors)
                wattrset(win, COLOR_PAIR(color_pair_for(cell)) |
                              (cell.bold ? A_BOLD : A_NORMAL));
            else
                wattrset(win, cell.bold ? A_BOLD : A_NORMAL);
            mvwaddnwstr(win, row, col, &cell.ch, 1);
        }
    }

    if (focused) {
        const int cursor_row = buf->get_cursor_row();
        const int cursor_col = buf->get_cursor_col();
        if (cursor_row >= 0 && cursor_row < rows &&
            cursor_col >= 0 && cursor_col < cols) {
            wattron(win, A_REVERSE);
            mvwaddnwstr(win, cursor_row, cursor_col, &buf->at(cursor_row, cursor_col).ch, 1);
            wattroff(win, A_REVERSE);
        }
    }
    wnoutrefresh(win);
}

void Terminal::handle_key(int key)
{
    if (pty == nullptr)
        return;

    switch (key) {
    case KEY_UP: send_sequence(*pty, "\033[A"); return;
    case KEY_DOWN: send_sequence(*pty, "\033[B"); return;
    case KEY_RIGHT: send_sequence(*pty, "\033[C"); return;
    case KEY_LEFT: send_sequence(*pty, "\033[D"); return;
    case KEY_HOME: send_sequence(*pty, "\033[H"); return;
    case KEY_END: send_sequence(*pty, "\033[F"); return;
    case KEY_PPAGE: send_sequence(*pty, "\033[5~"); return;
    case KEY_NPAGE: send_sequence(*pty, "\033[6~"); return;
    case KEY_IC: send_sequence(*pty, "\033[2~"); return;
    case KEY_DC: send_sequence(*pty, "\033[3~"); return;
    case KEY_BACKSPACE: send_sequence(*pty, "\177"); return;
    case KEY_ENTER: send_sequence(*pty, "\r"); return;
    case KEY_RESIZE: return;
    default: break;
    }

    if (key >= 0 && key <= 255) {
        const char character = static_cast<char>(key);
        pty->write_input(&character, 1);
    }
}

void Terminal::resize(int row_count, int column_count)
{
    rows = std::max(1, row_count);
    cols = std::max(1, column_count);
    if (win != nullptr)
        wresize(win, rows, cols);
    if (buf != nullptr)
        buf->resize(rows, cols);
    if (parser != nullptr && buf != nullptr)
        parser = std::make_unique<VtParser>(*buf);
    if (pty != nullptr)
        pty->resize(rows, cols);
}

void Terminal::toggle_focus()
{
    focused = !focused;
}

WINDOW* Terminal::get_win() const
{
    return win;
}
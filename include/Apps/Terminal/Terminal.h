#ifndef TERMINAL_H
#define TERMINAL_H

#include "Engine/MainEngine.h"
#include "Apps/Terminal/Pty.h"
#include "Apps/Terminal/TermBuffer.h"
#include "Apps/Terminal/VtParser.h"
#include <string>

class Terminal {
    private:
        MainEngine& engine;
        WINDOW* win;
        int rows;
        int cols;
        bool focused;
        std::unique_ptr<Pty> pty;
        std::unique_ptr<TermBuffer> buf;
        std::unique_ptr<VtParser> parser;

    public:
        Terminal(WINDOW* window, MainEngine& main_engine, int row_count,
                 int column_count, const std::string& shell = "/bin/zsh");
        void poll();
        void draw();
        void handle_key(int key);
        void resize(int rows, int cols);
        void toggle_focus();
        WINDOW* get_win() const;
};



#endif // TERMINAL_H
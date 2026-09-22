#ifndef TERMBUFFER_H
#define TERMBUFFER_H

#include <vector>
#include "Apps/Terminal/Cell.h"

class TermBuffer {
    private :
        int rows;
        int cols;
        std::vector<std::vector<Cell>> grid;
        int cursor_row;
        int cursor_col;

    public :
        TermBuffer(int row_count = 24, int column_count = 80);
        Cell& at(int r, int c);

        int get_rows() const;
        int get_cols() const;

        int get_cursor_row() const;
        int get_cursor_col() const;


        void set_cursor(int r, int c);
        void scroll_up(int top, int bot);
        void resize(int rown, int cols);
        void clear(int r, int c, int row_count, int column_count,
               short fg = 7, short bg = 0, bool bold = false);
        void put(int r, int c, wchar_t ch, short fg, short bg, bool bold);
};



#endif // TERMBUFFER_H
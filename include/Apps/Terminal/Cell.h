#ifndef CELL_H
#define CELL_H

struct Cell {
    wchar_t ch;
    short fg;
    short bg;
    bool bold;
    bool dirty;
};

#endif // CELL_H
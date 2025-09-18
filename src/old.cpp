#include <ncurses.h>

int main() {
    initscr();
    printw("Hello ncurses sous Arch !\nAppuyez sur une touche...");
    refresh();
    getch();
    endwin();
    return 0;
}

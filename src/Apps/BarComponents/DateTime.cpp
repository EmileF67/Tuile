#include "Apps/BarComponents/DateTime.h"
#include "ncurses.h"
#include <ctime>
#include <cstring>


DateTime::DateTime(DrawType drawtype_)
    : Module(1, 21), drawtype(drawtype_), now(std::time(nullptr))
{
    switch (drawtype) {
        case DrawType::Time :
            size = 8;
            break;

        case DrawType::Date :
            size = 10;
            break;
    }
}




void DateTime::draw(int x)
{
    // On actualise <now> avec l'heure actuelle
    now = std::time(nullptr);

    // On affiche le cas prévu
    switch (drawtype) {
        case DrawType::Time :
            draw_time(x);
            break;
            
        case DrawType::Date :
            draw_date(x);
            break;
            
        case DrawType::DateTime:
            draw_datetime(x);
            break;
            
        case DrawType::TimeDate:
            draw_timedate(x);
            break;
    }
}




void DateTime::draw_time(int x)
{
    std::tm* tm = std::localtime(&now);

    char buffer[100];
    std::strftime(buffer, sizeof(buffer), "%H:%M:%S", tm);

    mvwaddstr(win, 1, x, buffer);
}


void DateTime::draw_date(int x)
{
    std::tm* tm = std::localtime(&now);

    char buffer[100];
    std::strftime(buffer, sizeof(buffer), "%d/%m/%Y", tm);

    mvwaddstr(win, 1, x, buffer);
}


void DateTime::draw_datetime(int x)
{
    std::tm* tm = std::localtime(&now);

    char buffer[100];
    std::strftime(buffer, sizeof(buffer), "%d/%m/%Y - %H:%M:%S", tm);

    mvwaddstr(win, 1, x, buffer);
}


void DateTime::draw_timedate(int x)
{
    std::tm* tm = std::localtime(&now);

    char buffer[100];
    std::strftime(buffer, sizeof(buffer), "%H:%M:%S - %d/%m/%Y", tm);

    mvwaddstr(win, 1, x, buffer);
}


void DateTime::update_draw_type(DrawType drawtype_)
{
    drawtype = drawtype_;
}
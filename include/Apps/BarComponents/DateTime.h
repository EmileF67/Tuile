#ifndef DATETIME_H
#define DATETIME_H

#include "Apps/BarComponents/Module.h"


enum class DrawType {
    Time,
    Date,
    DateTime,
    TimeDate
};


class DateTime : public Module {
    private :
        DrawType drawtype;
        std::time_t now;

    public :
        // Constructeur
        DateTime(DrawType drawtype_);

        // Permet de changer la valeur de drawtype
        void update_draw_type(DrawType drawtype_);

        // Affiche la date et/ou l'heure sous le format choisi de drawtype
        void draw(int x);


    private :
        // afficher sous le format Time
        void draw_time(int x);

        // afficher sous le format Date
        void draw_date(int x);

        // afficher sous le format DateTime
        void draw_datetime(int x);

        // afficher sous le format TimeDate
        void draw_timedate(int x);

};

#endif // DATETIME_H
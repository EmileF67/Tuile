#ifndef SYSTEMUSAGE_H
#define SYSTEMUSAGE_H


#include "Apps/BarComponents/Module.h"
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <ncurses.h>

#define PATH_CPU_STATS "/proc/stat"
#define PATH_RAM_SWAP_STATS "/proc/meminfo"


enum class DataType {
    Cpu,
    Gpu,
    Ram,
    Swap
};


enum class UsagePercentageColor : int {
    Bleu = 0,
    Cyan = 20,
    Vert = 40,
    Jaune = 60,
    Rouge = 80
};


class SystemUsage : public Module {
    private :
        DataType datatype;


    public :
        SystemUsage(DataType datatype_);

        // Affiche la donnée en fonction du type de donnée choisi
        void draw(int x);

    private :
        // Extraire les différentes données possibles
        short extract_cpu_data();
        short extract_gpu_data();
        short extract_ram_data();
        short extract_swap_data();

        // Déterminer la couleur du pourcentage à afficher
        short compute_percentage_color(short percentage);

        // Afficher les différentes données possibles
        void draw_cpu_data(int x);
        void draw_gpu_data(int x);
        void draw_ram_data(int x);
        void draw_swap_data(int x);


};



#endif // SYSTEMUSAGE_H
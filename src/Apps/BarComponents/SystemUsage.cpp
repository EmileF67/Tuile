#include "Apps/BarComponents/SystemUsage.h"

#include <glob.h>

#define REFRESH_TIME_SYSTEMUSAGE 1
#define SIZE_OF_SYSTEM_USAGE 5


SystemUsage::SystemUsage(DataType datatype_)
    : Module(REFRESH_TIME_SYSTEMUSAGE, SIZE_OF_SYSTEM_USAGE), datatype(datatype_)
{

}




void SystemUsage::draw(int x)
{
    switch (datatype) {
        case DataType::Cpu :
            draw_cpu_data(x);
            break;

        case DataType::Gpu :
            draw_gpu_data(x);
            break;

        case DataType::Ram :
            draw_ram_data(x);
            break;

        case DataType::Swap :
            draw_swap_data(x);
            break;

        default :
            break;

    (void)x; // Suppress unused parameter warning
    }
}


short SystemUsage::compute_percentage_color(short percentage)
{
    short color_number = 1;

    if (percentage >= static_cast<short>(UsagePercentageColor::Rouge))
    {
        color_number = 12;
    }
    else if (percentage >= static_cast<short>(UsagePercentageColor::Jaune))
    {
        color_number = 4;
    }
    else if (percentage >= static_cast<short>(UsagePercentageColor::Vert))
    {
        color_number = 2;
    }
    else if (percentage >= static_cast<short>(UsagePercentageColor::Cyan))
    {
        color_number = 3;
    }

    return color_number;
}


void SystemUsage::draw_cpu_data(int x)
{
    short percentage = extract_cpu_data();
    char data[5];
    snprintf(data, sizeof(data), "%2d%%", percentage);

    short color = this->compute_percentage_color(percentage);


    wattron(win, COLOR_PAIR(color));
    mvwaddstr(win, 1, x, data);
    wattroff(win, COLOR_PAIR(color));

    mvwaddstr(win, 1, x+4, "C");
}



void SystemUsage::draw_gpu_data(int x)
{
    short percentage = extract_gpu_data();
    char data[5];
    snprintf(data, sizeof(data), "%2d%%", percentage);

    short color = this->compute_percentage_color(percentage);


    wattron(win, COLOR_PAIR(color));
    mvwaddstr(win, 1, x, data);
    wattroff(win, COLOR_PAIR(color));

    mvwaddstr(win, 1, x+4, "G");}



void SystemUsage::draw_ram_data(int x)
{
    short percentage = extract_ram_data();
    char data[5];
    snprintf(data, sizeof(data), "%2d%%", percentage);

    
    short color = this->compute_percentage_color(percentage);


    wattron(win, COLOR_PAIR(color));
    mvwaddstr(win, 1, x, data);
    wattroff(win, COLOR_PAIR(color));

    mvwaddstr(win, 1, x+4, "R");
}



void SystemUsage::draw_swap_data(int x)
{
    short percentage = extract_swap_data();
    char data[5];
    snprintf(data, sizeof(data), "%2d%%", percentage);


    short color = this->compute_percentage_color(percentage);


    wattron(win, COLOR_PAIR(color));
    mvwaddstr(win, 1, x, data);
    wattroff(win, COLOR_PAIR(color));

    mvwaddstr(win, 1, x+4, "S");}






short SystemUsage::extract_cpu_data()
{
    std::ifstream file(PATH_CPU_STATS);
    if (!file.is_open()) {
        return 0;
    }

    std::string line;
    std::getline(file, line);
    file.close();

    // Format attendu :
    // cpu  user nice system idle iowait irq softirq steal guest guest_nice
    std::istringstream iss(line);
    std::string cpu_label;

    unsigned long long user = 0, nice = 0, system = 0, idle = 0;
    unsigned long long iowait = 0, irq = 0, softirq = 0, steal = 0;

    iss >> cpu_label >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal;

    if (cpu_label != "cpu") {
        return 0;
    }

    // idle total = idle + iowait
    unsigned long long idle_time = idle + iowait;

    // total = somme de tous les temps pertinents
    unsigned long long total_time = user + nice + system + idle + iowait + irq + softirq + steal;

    // Valeurs précédentes conservées entre les appels
    static unsigned long long prev_idle = 0;
    static unsigned long long prev_total = 0;
    static bool first_call = true;

    // Premier appel : on stocke juste la base de référence
    if (first_call) {
        prev_idle = idle_time;
        prev_total = total_time;
        first_call = false;
        return 0; // ou -1 si tu veux signaler "pas encore calculable"
    }

    unsigned long long delta_idle = idle_time - prev_idle;
    unsigned long long delta_total = total_time - prev_total;

    prev_idle = idle_time;
    prev_total = total_time;

    if (delta_total == 0) {
        return 0;
    }

    double usage = 100.0 * (1.0 - static_cast<double>(delta_idle) / static_cast<double>(delta_total));

    // Clamp entre 0 et 100
    usage = std::max(0.0, std::min(99.0, usage));

    return static_cast<short>(usage + 0.5); // arrondi propre
}





static short parse_meminfo_usage(bool use_available)
{
    std::ifstream file(PATH_RAM_SWAP_STATS);
    if (!file.is_open()) {
        return 0;
    }

    unsigned long long total_value = 0;
    unsigned long long free_value = 0;
    std::string line;

    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string key;
        unsigned long long value = 0;

        if (!(iss >> key >> value)) {
            continue;
        }

        if (use_available) {
            if (key == "MemTotal:") {
                total_value = value;
            } else if (key == "MemAvailable:") {
                free_value = value;
            }
        } else {
            if (key == "SwapTotal:") {
                total_value = value;
            } else if (key == "SwapFree:") {
                free_value = value;
            }
        }

        if (total_value && free_value) {
            break;
        }
    }

    if (total_value == 0 || free_value > total_value) {
        return 0;
    }

    double usage = 100.0 * static_cast<double>(total_value - free_value) / static_cast<double>(total_value);
    usage = std::max(0.0, std::min(99.0, usage));
    return static_cast<short>(usage + 0.5);
}


short SystemUsage::extract_gpu_data()
{
    glob_t glob_result;
    const char* pattern = "/sys/class/drm/card*/device/gpu_busy_percent";
    if (glob(pattern, 0, nullptr, &glob_result) != 0) {
        return 0;
    }

    short gpu_usage = 0;
    for (size_t i = 0; i < glob_result.gl_pathc; ++i) {
        std::ifstream file(glob_result.gl_pathv[i]);
        if (!file.is_open()) {
            continue;
        }

        int value = 0;
        file >> value;
        if (file.fail() || value < 0) {
            continue;
        }

        gpu_usage = static_cast<short>(std::max(0, std::min(99, value)));
        break;
    }

    globfree(&glob_result);
    return gpu_usage;
}


short SystemUsage::extract_ram_data()
{
    return parse_meminfo_usage(true);
}


short SystemUsage::extract_swap_data()
{
    return parse_meminfo_usage(false);
}
#ifndef APPS_H
#define APPS_H

#include <string_view>

enum class Apps {
    FileManager,
    Terminal
};


std::string_view toString(Apps app)
{
    switch (app) {
        case Apps::FileManager:
            return "File Manager";

        case Apps::Terminal:
            return "Terminal";
    }

    return "Unknown";
}

std::string_view toIcon(Apps app)
{
    switch (app) {
        case Apps::FileManager:
            return "🖿 ";
        
        case Apps::Terminal:
            return ">_";
    }

    return "?";
}


#endif // APPS_H
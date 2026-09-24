#include "Apps/AppLauncher/AppLauncher.h"


AppLauncher::AppLauncher(WINDOW* win_, bool is_linux_console_) {
    win = win_;
    is_linux_console = is_linux_console_;
    i_selected = 0;
    scroll_offset = 0;

    // On remplis la liste des apps
    apps.push_back(Apps::FileManager);
    apps.push_back(Apps::Terminal);

    
    // On récupère la taille actuelle de la fenêtre
    int rows, cols;
    getmaxyx(win, rows, cols);

    // On créer le cadre
    cadre = Cadre(win, {0, 0}, {rows-1, cols-1}, is_linux_console);
}




void AppLauncher::launch(Apps app) {
    switch (app)
    {
    case Apps::FileManager:
        // Instantiate & Start FileManager
        break;

    case Apps::Terminal:
        // Instantiate & Start Terminal
        break;
    
    default:
        break;
    }
}




void AppLauncher::draw() {
    cadre.draw();

    // Afficher le titre de la fenêtre
    std::string text = "Choisissez une application à lancer :";
    mvwaddstr(win, 2, 2, text.c_str());

    // Les choix
    std::size_t y = 4;
    int color = 0;
    for (std::size_t i = 0; i < apps.size(); i++) {

        std::string line = "  " + static_cast<std::string>(toIcon(apps[i])) + static_cast<std::string>(toString(apps[i])); 
        
        color = COLOR_UNSELECTED;
        if (i == i_selected) {
            color = COLOR_SELECTED;
            std::string line = "→ " + static_cast<std::string>(toIcon(apps[i])) + " " + static_cast<std::string>(toString(apps[i]));
        }
        
        wattron(win, COLOR_PAIR(color));
        mvwaddstr(win, y, 2, line.c_str());
        wattroff(win, COLOR_PAIR(color));
        y++;
    }
}




void AppLauncher::handle_key(int key) {
    if (key == KEY_UP) {
        i_selected = static_cast<std::size_t>(std::max(0, static_cast<int>(i_selected) - 1));
    }

    else if (key == KEY_DOWN) {
        i_selected = static_cast<std::size_t>(std::min(static_cast<int>(apps.size()) - 1, static_cast<int>(i_selected) - 1));
    }

    else if (key == KEY_ENTER || key == 10 || key == 13) {
        launch(apps[i_selected]);
    }
}
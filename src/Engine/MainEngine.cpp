#include "Engine/MainEngine.h"
#include "Apps/BarComponents/DateTime.h"
#include <ncurses.h>

#define MAX_DISPLAYED_WINDOWS 10

MainEngine::MainEngine(WINDOW* stdscr_, bool is_linux_console_)
    : stdscr(stdscr_), is_linux_console(is_linux_console_), rows(0), cols(0), display_bar(true), popup_type(PopupType::None)
{
    getmaxyx(stdscr, rows, cols);

    if (display_bar) {
        this->load_bar_modules(stdscr_);
    }

}

MainEngine::~MainEngine() {
    for (WINDOW* win : windows) {
        delwin(win);
    }
}


void MainEngine::load_bar_modules(WINDOW* stdscr)
{
    bar = std::make_unique<Bar>(stdscr, is_linux_console);
        
    std::unique_ptr<DateTime> dt = std::make_unique<DateTime>(DrawType::DateTime);
    bar->ajout_module(std::move(dt), BarArea::Middle);

    std::unique_ptr<DateTime> dt2 = std::make_unique<DateTime>(DrawType::Time);
    bar->ajout_module(std::move(dt2), BarArea::Left);

    std::unique_ptr<DateTime> dt3 = std::make_unique<DateTime>(DrawType::Date);
    bar->ajout_module(std::move(dt3), BarArea::Right);
}

WINDOW* MainEngine::new_window(const std::string& name)
{
    if (windows.size() >= MAX_DISPLAYED_WINDOWS)
        return nullptr;

    // Mise à jour layout AVANT création
    update_layout();

    int h = rows;
    int w = cols;
    int y = 0;
    int x = 0;

    // [TODO] Diviser par windows.size()+1 pour répartir la taille équitablement
    // if (windows.size() == 1) {
    //     w = cols / 2;
    //     x = cols / 2;
    // }

    int num_windows = static_cast<int>(windows.size());
    w = (num_windows == 0) ? cols : cols / num_windows;
    x = (num_windows == 0) ? 0    : (cols / num_windows) * num_windows;

    if (display_bar) {
        h -= 3;
        y += 3;
    }
    
    
    WINDOW* win = newwin(h, w, y, x);
    windows.push_back(win);
    window_names.push_back(name);
    
    // Recalcul complet après ajout
    update_layout();

    return win;
}

void MainEngine::update_layout()
{
    if (display_bar) {
        this->load_bar_modules(stdscr);
    }

    if (windows.empty())
        return;

    getmaxyx(stdscr, rows, cols);

    // Si je met pas ça, c'est pas beau, allez savoir...
    if (cols % 2 == 1) {
        cols -= 1;
    }

    for (size_t i = 0; i < windows.size(); ++i) {
        int new_h = rows;
        int new_w = cols;
        int new_y = 0;
        int new_x = 0;

        // if (windows.size() == 2) {
        //     new_w = cols / 2;
        //     new_x = (i == 0) ? 0 : cols / 2;
        // }
        
        int num_windows = static_cast<int>(windows.size());
        // new_w = cols / num_windows - 1;
        // new_x = (cols / num_windows) * i;
        new_w = (num_windows == 0) ? cols : cols / num_windows;
        new_x = (num_windows == 0) ? 0    : (cols / num_windows) * i;

        if (display_bar) {
            new_h -= 3;
            new_y += 3;
        }

        wresize(windows[i], new_h, new_w);
        mvwin(windows[i], new_y, new_x);
        werase(windows[i]);
    }

    if (popup_type != PopupType::None) {
        switch (popup_type) {
            case PopupType::Info : {
                this->popup_info = std::make_unique<PopupInfo>(
                    stdscr,
                    popup_info->label,
                    is_linux_console
                );
                break;
            }
    
            case PopupType::InputText : {
                std::string old_text = popup_input_text->get_text();
                this->popup_input_text = std::make_unique<PopupInputText>(
                    stdscr,
                    popup_input_text->label,
                    is_linux_console
                );
                popup_input_text->get_input()->set_text(old_text);
                break;
            }
    
            case PopupType::DoubleChoices : {
                short old_indice = popup_double_choices->get_selected();
                this->popup_double_choices = std::make_unique<PopupDoubleChoices>(
                    stdscr,
                    popup_double_choices->label,
                    popup_double_choices->get_choices(),
                    is_linux_console
                );
                popup_double_choices->set_selected(old_indice);
                break;
            }
    
            default :
                break;
        }
    }
}

void MainEngine::refresh_all_and_update()
{
    // On met à jour la fenêtre principale
    wnoutrefresh(stdscr);   // IMPORTANT

    // On met à jour chaque fenêtre d'apps
    for (WINDOW* win : windows) {
        wnoutrefresh(win);
    }

    // On met à jour la barre
    if (display_bar) {
        wnoutrefresh(bar->get_win());
    }

    // On met à jour le popup en dernier
    switch (this->popup_type) {
        case PopupType::None :
            break;

        case PopupType::Info :
            wnoutrefresh(this->popup_info->get_win());
            break;

        case PopupType::InputText :
            wnoutrefresh(this->popup_input_text->get_win());
            break;

        case PopupType::DoubleChoices :
            wnoutrefresh(this->popup_double_choices->get_win());
            break;

        default :
            break;
    }
    

    doupdate();
}

bool MainEngine::detect_resizing()
{
    int old_rows = rows;
    int old_cols = cols;

    // OBLIGATOIRE avec ncurses
    resize_term(0, 0);
    getmaxyx(stdscr, rows, cols);

    if (rows != old_rows || cols != old_cols) {
        update_layout();
        return true;
    }
    return false;
}


bool MainEngine::global_handle_key(int key)
{
    bool res = false;

    switch (this->popup_type) {
        case PopupType::Info :
            this->popup_info->handle_key(key);
            res = true;
            break;

        case PopupType::InputText :
            this->popup_input_text->handle_key(key);
            res = true;
            break;

        case PopupType::DoubleChoices :
            this->popup_double_choices->handle_key(key);
            res = true;
            break;

        default :
            break;
    }

    return res;
}


void MainEngine::draw_popup()
{
    switch (this->popup_type) {
        case PopupType::Info :
            this->popup_info->draw();
            break;

        case PopupType::InputText :
            this->popup_input_text->draw();
            break;

        case PopupType::DoubleChoices :
            this->popup_double_choices->draw();
            break;

        default :
            break;
    }
}


void MainEngine::draw_bar()
{
    if (bar) {
        bar->draw();
    }
}


void MainEngine::create_info_popup(std::string label)
{
    // On ne fait rien si un popup existe déjà
    if (this->popup_type == PopupType::None) {
        // On indique qu'un popup d'information est en cours
        this->popup_type = PopupType::Info;

        // On créer notre instance de popup
        this->popup_info = std::make_unique<PopupInfo>(stdscr, label, this->is_linux_console);
    };
}


void MainEngine::create_input_popup(std::string label)
{
    if (this->popup_type == PopupType::None) {
        this->popup_type = PopupType::InputText;

        this->popup_input_text = std::make_unique<PopupInputText>(stdscr, label, this->is_linux_console);
    }
}


void MainEngine::create_double_choices_popup(std::string label, Choices choices)
{
    if (this->popup_type == PopupType::None) {
        this->popup_type = PopupType::DoubleChoices;

        this->popup_double_choices = std::make_unique<PopupDoubleChoices>(stdscr, label, choices, this->is_linux_console);
    }
}


void MainEngine::reset_popups()
{
    this->popup_info.reset();
    this->popup_input_text.reset();
    this->popup_double_choices.reset();

    this->popup_type = PopupType::None;
}




bool MainEngine::is_popup_done() const {
    switch(this->popup_type) {
        case PopupType::InputText:
            return this->popup_input_text->is_entered();

        case PopupType::DoubleChoices:
            return this->popup_double_choices->is_entered();

        case PopupType::Info:
            return this->popup_info->is_entered();

        default:
            return false;
    }
}

std::string MainEngine::get_input_popup_value() const {
    if (this->popup_input_text)
        return this->popup_input_text->get_text();
    return "";
}

short MainEngine::get_double_choices_popup_value() const {
    if (this->popup_double_choices)
        return this->popup_double_choices->get_selected();
    return -1;
}

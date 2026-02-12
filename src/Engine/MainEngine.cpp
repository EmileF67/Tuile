#include "Engine/MainEngine.h"
#include <ncurses.h>

#define MAX_DISPLAYED_WINDOWS 2

MainEngine::MainEngine(WINDOW* stdscr_, bool is_linux_console_)
    : stdscr(stdscr_), is_linux_console(is_linux_console_), rows(0), cols(0), bar(true), popup_type(PopupType::None)
{
    getmaxyx(stdscr, rows, cols);
}

MainEngine::~MainEngine() {
    for (WINDOW* win : windows) {
        delwin(win);
    }
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

    if (windows.size() == 1) {
        w = cols / 2;
        x = cols / 2;
    }

    if (bar) {
        h -= 2;
        y += 2;
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
    if (windows.empty())
        return;

    getmaxyx(stdscr, rows, cols);

    for (size_t i = 0; i < windows.size(); ++i) {
        int new_h = rows;
        int new_w = cols;
        int new_y = 0;
        int new_x = 0;

        if (windows.size() == 2) {
            new_w = cols / 2;
            new_x = (i == 0) ? 0 : cols / 2;
        }

        if (bar) {
            new_h -= 2;
            new_y += 2;
        }

        wresize(windows[i], new_h, new_w);
        mvwin(windows[i], new_y, new_x);
        werase(windows[i]);
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

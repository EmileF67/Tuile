#ifndef POPUP2_H
#define POPUP2_H


#include <ncurses.h>
#include "Engine/Components/Cadre.h"
#include "Engine/Components/Input.h"
#include <string>
#include <utility>
#include <vector>
#include <memory>

//                            Logo        Label
using Choice  = std::pair<std::string, std::string>;
using Choices = std::pair<Choice, Choice>;


// Représente les 3 différentes possibilités de popup
enum class PopupType {
    None,
    Info,
    InputText,
    DoubleChoices
};


// Représente un popup général, ainsi que ce que les 3 différentes déviations ont en commun
class Popup {
    protected :
        std::unique_ptr<Cadre> cadre;
        WINDOW* win;
        WINDOW* stdscr;
        int rows, cols;
        bool entered;
        bool is_linux_console;
        int height;
        int width;


    public :
        std::string titre;
        std::string label;
        

    public :
        // Constructeur
        Popup(WINDOW* stdscr_, std::string titre_, std::string label_, bool is_linux_console_);

        // Destructeur
        virtual ~Popup();
        
        // Permet d'accéder à la fenêtre pour l'affichage
        WINDOW* get_win() { return win; };

        // Permet de savoir si l'utilisateur à validé le popup
        bool is_entered() { return entered; };
        
        
    protected :
        // Affiche le cadre, le titre et le label
        void draw_base();

        // Gérer les touches communes
        bool handle_key_base(int key);

        // polymorphisme propre
        // virtual void draw() = 0;
        // virtual void handle_key(int key) = 0;

};

// Uniquement dans le but d'informer, cette classe affiche le texte via label de la classe Popup
// mais aussi un bouton dont le nom peut être choisi
class PopupInfo : public Popup {
    public :
        std::string button_name;


    public :
        // Constructeur
        PopupInfo(WINDOW* stdscr_, std::string label_, bool is_linux_console_);

        // Destructeur
        ~PopupInfo();

        // Afficher les particularitées de ce type de popup
        void draw();

        // Gérer les touches
        void handle_key(int key);
};



// Permet de demander du texte avec un input, la question sera dans le label
class PopupInputText : public Popup {
    private :
        std::unique_ptr<Input> input;


    public :
        // Constructeur
        PopupInputText(WINDOW* stdscr_, std::string label_, bool is_linux_console_);

        // Destructeur
        ~PopupInputText();

        // Permet de récupérer le texte entré
        std::string get_text() { return input->get_text(); };

        // Permet de récupérer l'input (retourne un pointeur brut vers l'input détenu)
        Input* get_input() { return input.get(); };

        // Afficher les particularitées de ce type de popup
        void draw();
        
        // Gérer les touches
        void handle_key(int key);
};



// Permet de proposer un choix entre deux possibilités :
// Un choix est une paire composé d'un logo et du texte représentant ici l'option
class PopupDoubleChoices : public Popup {
    private :
        Choices choices;
        short selected;

    public :
        // Constructeur
        PopupDoubleChoices(WINDOW* stdscr_, std::string label_, Choices choices_, bool is_linux_console_);

        // Destructeur
        ~PopupDoubleChoices();

        // Permet d'obtenir l'indice du choix séléctionné
        short get_selected() { return selected; };

        // Permet de définir l'indice du choix séléctionné
        void set_selected(short indice) { selected = indice; };

        // Permet de lire les choix du popup
        Choices get_choices() { return choices; };

        // Afficher les particularitées de ce type de popup
        void draw();

        // Afficher les deux choix
        void draw_both_choices();

        // Gérer les touches
        void handle_key(int key);
};





#endif // POPUP2_H
#ifndef SYSTEM_H
#define SYSTEM_H

#include <string>

class System {
public:
    // Retourne true si le terminal courant est la console linux (tty texte)
    static bool isLinuxConsole();
};

#endif // SYSTEM_H

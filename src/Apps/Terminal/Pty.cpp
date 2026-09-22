#include "Apps/Terminal/Pty.h"

#include <unistd.h>


Pty::Pty() { }


bool Pty::spawn(int rows, int cols, const std::string shell)
{
    // On formate la taille du terminal
    struct winsize ws{};
    ws.ws_row = rows;
    ws.ws_col = cols;

    // On lance le fork
    child_pid = forkpty(
        &master_fd,
        nullptr,
        nullptr,
        &ws
    );

    // Si on a une erreur
    if (child_pid == -1) {
        perror("forkpty");
        return false;
    }

    // Le processus fils doit remplacer immédiatement l'image courante par le shell.
    if (child_pid == 0) {
        execlp(shell.c_str(), shell.c_str(), nullptr);
        perror("execlp");
        std::exit(127);
    }

    // On rend master_fd non bloquant dans le processus parent uniquement.
    int flags = fcntl(master_fd, F_GETFL, 0);

    if (flags == -1) {
        return false;
    }

    if (fcntl(master_fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        return false;
    }

    return true;
}



ssize_t Pty::read_output(char* buf, std::size_t n)
{
    ssize_t result = read(master_fd, buf, n);

    if (result == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return 0;

        perror("read");
        return -1;
    }

    return result;
}



void Pty::write_input(const char* buf, std::size_t n)
{
    std::size_t written = 0;

    while (written < n) {
        ssize_t result = write(
            master_fd,
            buf + written,
            n - written
        );

        if (result == -1) {
            if (errno == EINTR)
                continue;

            perror("write");
            return;
        }

        written += result;
    }
}



void Pty::resize(int rows, int cols) {
    // On formate la taille
    struct winsize ws{};
    ws.ws_row = rows;
    ws.ws_col = cols;

    // On envoie le redimensionnement au fils
    ioctl(master_fd, TIOCSWINSZ, ws);
}
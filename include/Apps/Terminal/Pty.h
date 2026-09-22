#ifndef PTY_H
#define PTY_H

#include <string>
#include <pty.h>
#include <fcntl.h>
#include <cerrno>
#include <cstdio>
#include <unistd.h>

class Pty {
    private:
        int master_fd;
        pid_t child_pid;

    public:
        Pty();
        bool spawn(int rows, int cols, const std::string shell);
        ssize_t read_output(char* buf, std::size_t n);
        void write_input(const char* buf, std::size_t n);
        void resize(int rows, int cols);
};



#endif // PTY_H
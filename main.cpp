#include <iostream>
#include <fstream>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <sys/types.h>
#include <sys/stat.h>

#include "process_io.h"

int main() {
    // Create FIFOs (named pipes)
    showdown::io::FIFO fifo;
    if (fifo.GetErrorCode() != 0) {
        return 1;
    }

    // Fork a child process to run the target executable
    pid_t pid = fork();
    if (pid == -1) {
        std::cerr << "Failed to fork: " << strerror(errno) << std::endl;
        return 1;
    }

    if (pid == 0) {  // Child process
        fifo.ExecuteProcess({"grep", "example"});
        _exit(1);
    } else {  // Parent process
        // Open FIFOs
        std::ofstream fifo_in = fifo.CreateInputStream();
        std::ifstream fifo_out = fifo.CreateOutputStream();

        if (!fifo_in || !fifo_out) {
            std::cerr << "Failed to open FIFOs in parent: " << strerror(errno) << std::endl;
            return 1;
        }

        // Write data to FIFO (input to the child process)
        fifo_in << "This is an example input.\nThis line should be filtered.\n";
        fifo_in.close();

        // Read the output from FIFO (output from the child process)
        std::string line;
        while (std::getline(fifo_out, line)) {
            std::cout << "Output from child process: " << line << std::endl;
        }
        fifo_out.close();

        // Wait for the child process to finish
        wait(NULL);
    }

    return 0;
}


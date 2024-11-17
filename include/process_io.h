#pragma once

#include <cstdarg>
#include <fstream>
#include <fcntl.h>
#include <string>
#include <unistd.h>
#include <vector>

namespace showdown::io {

namespace {

}  // namespace
 
class FIFO {
public:
    FIFO() : errcode_() {}

    ~FIFO() {
        // Clean up FIFOs
        // unlink(kFifoInPath);
        // unlink(kFifoOutPath);
    }

    int GetErrorCode() const {return errcode_;}

    int ExecuteProcess(const std::vector<const char*>& command) {
        std::cout << "Opening FIFO for child process:" << std::endl;
        for (const auto& arg : command) {
            std::cout << arg << " ";
        }
        std::cout << std::endl;

        // Open FIFOs
        int fifo_in_fd = open(kFifoInPath, O_RDONLY);
        int fifo_out_fd = open(kFifoOutPath, O_WRONLY);
        
        if (fifo_in_fd == -1 || fifo_out_fd == -1) {
            std::cerr << "Failed to open FIFOs in child: " << strerror(errno) << std::endl;
            return 1;
        }

        std::cout << "Redirect stdin and stdout" << std::endl;

        dup2(fifo_in_fd, STDIN_FILENO);
        dup2(fifo_out_fd, STDOUT_FILENO);
        
        close(fifo_in_fd);
        close(fifo_out_fd);

        
        // Execute the target process (e.g., `grep` command)
        execvp(command[0], const_cast<char* const*>(command.data()));
        std::cerr << "Failed to exec: " << strerror(errno) << std::endl;
        return 1;
    }

    std::ifstream CreateInputStream() const {
        std::ifstream stream(kFifoInPath);
        if (!stream) {
            std::cerr << "Failed to open output FIFO: " << strerror(errno) << std::endl;
        } else {
            std::cout << "Opened output" << std::endl;
        }

        return stream;
    }

    std::ofstream CreateOutputStream() const {
        std::ofstream stream;
        std::cout << "Creating output stream.." << std::endl;
        stream.open(kFifoOutPath, std::ofstream::app);
        if (!stream.is_open()) {
            std::cerr << "Failed to open input FIFO: " << strerror(errno) << std::endl;
        } else {
            std::cout << "Opened output" << std::endl;
        }

        return stream;
    }

private:
    // TODO: Replace status with absl statusor
    // Note: Only the output of FIFOs are marked.
    int CreateFIFOs() {
        // Create FIFOs (named pipes)
        if (mkfifo(kFifoOutPath, 0666) == -1) {
            std::cerr << "Failed to create FIFOs: " << strerror(errno) << std::endl;
            return 1;
        }
        return 0;
    }

    static constexpr char kFifoInPath[] = "/tmp/fifo_to_bot";
    static constexpr char kFifoOutPath[] = "/tmp/fifo_from_bot";
    int errcode_ = 0;
};

} // namespace showdown::io

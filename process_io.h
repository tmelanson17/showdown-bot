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
    FIFO() : errcode_(CreateFIFOs()) {}

    ~FIFO() {
        // Clean up FIFOs
        unlink(kFifoInPath);
        unlink(kFifoOutPath);
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

    std::ifstream CreateOutputStream() const {
        std::ifstream stream(kFifoOutPath);
        if (!stream) {
            std::cerr << "Failed to open output FIFO: " << strerror(errno) << std::endl;
        }

        return stream;
    }

    std::ofstream CreateInputStream() const {
        std::ofstream stream(kFifoInPath);
        if (!stream) {
            std::cerr << "Failed to open input FIFO: " << strerror(errno) << std::endl;
        }

        return stream;
    }

private:
    // TODO: Replace status with absl statusor
    int CreateFIFOs() {
        // Create FIFOs (named pipes)
        if (mkfifo(kFifoInPath, 0666) == -1 || mkfifo(kFifoOutPath, 0666) == -1) {
            std::cerr << "Failed to create FIFOs: " << strerror(errno) << std::endl;
            return 1;
        }
        return 0;
    }

    static constexpr char kFifoInPath[] = "/tmp/my_fifo_in";
    static constexpr char kFifoOutPath[] = "/tmp/my_fifo_out";
    int errcode_ = 0;
};

} // namespace showdown::io

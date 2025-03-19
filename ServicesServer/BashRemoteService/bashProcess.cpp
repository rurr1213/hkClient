#include <stdio.h>
#include <iostream>
#include <unistd.h>
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>

#include <sys/wait.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/prctl.h>
#include <poll.h>

#include "bashProcess.h"
#include "Logger.h"

using namespace std;

BashProcess::BashProcess(const std::string name) : processName(name), processPid(0) {
    inputPipe[0] = 0;
    inputPipe[1] = 0;
    outputPipe[0] = 0;
    outputPipe[1] = 0;
}

BashProcess::~BashProcess() {
}

ssize_t BashProcess::readWithTimeout(int fd, char *buffer, size_t size, int timeout) {
    fd_set readfds;
    struct timeval tv;
    int retval;

    FD_ZERO(&readfds);
    FD_SET(fd, &readfds);

    tv.tv_sec = timeout;
    tv.tv_usec = 0;

    retval = select(fd + 1, &readfds, NULL, NULL, &tv);

    if (retval == -1) {
        std::cerr << "select() error\n";
        return -1;
    } else if (retval == 0) {
        // Timeout
        return 0;
    } else {
        if (FD_ISSET(fd, &readfds)) {
            int available;
            ioctl(fd, FIONREAD, &available);
            if (available > size) {
                LOG_WARNING("BashProcess::readWithTimeout()", "buffer size is too small", 0);
                return read(fd, buffer, std::min(size, static_cast<size_t>(available)));
            }
            if (available > 0) {
                return read(fd, buffer, std::min(size, static_cast<size_t>(available)));
            }
        }
    }

    return -1;
}

bool BashProcess::start(void) {
    if (pipe(inputPipe) == -1 || pipe(outputPipe) == -1 || pipe(errorPipe) == -1) { // Initialize error pipe
        std::cerr << "Pipe creation failed\n";
        return false;
    }

    processPid = fork();
    if (processPid == -1) {
        std::cerr << "Fork failed\n";
        return false;
    }

    if (processPid == 0) {
        // Child process
        close(inputPipe[1]);  // Close unused write end
        close(outputPipe[0]); // Close unused read end
        close(errorPipe[0]);  // Close unused read end of error pipe

        dup2(inputPipe[0], STDIN_FILENO);  // Redirect stdin
        dup2(outputPipe[1], STDOUT_FILENO); // Redirect stdout
        dup2(outputPipe[1], STDERR_FILENO); // Redirect stderr

        // Set the process name
        prctl(PR_SET_NAME, BASHSERVICE, 0, 0, 0);

        execl("/bin/bash", "bash", NULL);
//        execl("/bin/bash", "bash", "--rcfile", "/home/ravi/.bashrc", NULL);
//        execl("/bin/bash", "bash", "-c", "source ~/.bashrc; exec bash --rcfile ~/.bashrc", NULL);

        // If execl fails
        write(errorPipe[1], "1", 1);
        close(errorPipe[1]);
        _exit(1);
    } else {
        // Parent process
        close(inputPipe[0]);  // Close unused read end
        close(outputPipe[1]); // Close unused write end
        close(errorPipe[1]);  // Close unused write end of error pipe

        // Check for error signal from child process
        char errorSignal;
        const int ERROR_SIGNAL_TIMEOUT = 1;
        ssize_t bytesRead = readWithTimeout(errorPipe[0], &errorSignal, 1, ERROR_SIGNAL_TIMEOUT); // 5 seconds timeout
        close(errorPipe[0]);

        if (bytesRead > 0 && errorSignal == '1') {
            std::cerr << "execl failed\n";
            LOG_WARNING("BashProcess::start()", "Failed to start process: " + processName + " pid: " + std::to_string(processPid), processPid);
            return false;
        }

        LOG_INFO("BashProcess::start()", "started process: " + processName + " pid: " + std::to_string(processPid), processPid);
    }

    return true;
}

bool BashProcess::stop(void) {
    if (processPid != 0) {
        // Send SIGTERM to the bash process
        kill(processPid, SIGTERM);

        // Wait for the process to terminate
        waitpid(processPid, NULL, 0);

        // Close the pipes
        close(inputPipe[1]);
        close(outputPipe[0]);

        processPid = 0;

        LOG_INFO("BashProcess::stop()", "stopped bash process", 0);

        return true;
    }
    return false;
}

std::string BashProcess::execute(const std::string &input, int timeoutSecs, int default_buffer_size) {
    std::string output = executeRaw(input, timeoutSecs, default_buffer_size);
    output += prompt();
    return output;
}

std::string BashProcess::prompt(void) {
    return executeRaw("echo $USER@$HOSTNAME:$PWD", DEFAULT_CMD_TIMEOUT, DEFAULT_BUFFER_SIZE);
}

std::string BashProcess::executeRaw(const std::string &input, int timeoutSecs, int default_buffer_size) {
    if (processPid == 0) {
        LOG_ERROR("BashProcess::executeRaw()", "bash process is not running", 0);
        return "";
    }
    std::string output;
    try{

        // Check if the input pipe is still open and writable
        struct pollfd pfd;
        pfd.fd = inputPipe[1];
        pfd.events = POLLOUT;

        int pollResult = poll(&pfd, 1, 0); // Zero timeout for non-blocking check
        if (pollResult == -1) {
            LOG_ERROR("BashProcess::execute()", "poll() error: ", errno);
            return "";
        } else if (pollResult == 0) {
            LOG_ERROR("BashProcess::execute()", "poll() timeout: input pipe is not writable", 0);
            return "";
        } else if (pfd.revents & POLLHUP) {
            LOG_ERROR("BashProcess::execute()", "poll() error: input pipe is closed", 0);
            return "";
        }

        ssize_t bytesWritten = write(inputPipe[1], input.c_str(), input.length());
        if (bytesWritten == -1) {
            LOG_ERROR("BashProcess::execute()", "write() error: ", errno);
            return "";
        }

        write(inputPipe[1], "\n", 1);

        // Flush the input pipe
        fsync(inputPipe[1]);

        char buffer[default_buffer_size];
        ssize_t bytesRead;

        // if input string is "cd ", ie a change directory, then there is no output, so dont wait for anything.
        if (input.find("cd ") == 0) {
            timeoutSecs = 0;
        }
        while(true) {
            bytesRead = readWithTimeout(outputPipe[0], buffer, sizeof(buffer) - 1, timeoutSecs);
            if (bytesRead > 0) {
                buffer[bytesRead] = '\0';
                output += buffer;
                timeoutSecs = 0;    // if there is more to be read, don't wait
            } else {
                break;
            }
        }
    } catch(...) {
        LOG_ERROR("BashProcess::execute()", "Exception caught in BashProcess::execute()",0);
    }
    return output;
}

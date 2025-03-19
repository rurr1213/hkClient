#pragma once

#define BASHSERVICE "hkShellBashService"

class BashProcess
{
    static const int DEFAULT_CMD_TIMEOUT = 10;
    static const int DEFAULT_BUFFER_SIZE = 1024*64;
    private:

        std::string processName;
        int processPid = 0;
        int inputPipe[2];
        int outputPipe[2];
        int errorPipe[2];
        ssize_t readWithTimeout(int fd, char *buffer, size_t size, int timeout);
        std::string executeRaw(const std::string &input, int timeoutSecs = DEFAULT_CMD_TIMEOUT, int default_buffer_size = DEFAULT_BUFFER_SIZE);

    public:
        BashProcess(const std::string name = BASHSERVICE);
        ~BashProcess();
        bool start(void);
        std::string execute(const std::string &input, int timeoutSecs = DEFAULT_CMD_TIMEOUT, int default_buffer_size = DEFAULT_BUFFER_SIZE);
        std::string prompt(void);
        bool stop(void); // Add this line
};

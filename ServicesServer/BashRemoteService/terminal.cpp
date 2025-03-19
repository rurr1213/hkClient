#include "terminal.h"
#include <termios.h>
#include <unistd.h>

Terminal::Terminal(const std::string &prompt) : prompt(prompt) {}

void removeCarriageReturn(std::string &str) {
    if (!str.empty() && str.back() == '\n') {
        str.pop_back();
    }
}

void Terminal::run() {
    std::string command;
    char ch;

    // Set terminal to raw mode to capture each character
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    while (true) {
        std::cout << prompt;
        command.clear();

        while (true) {
            ch = getchar();

            if (ch == '\n') {
                std::cout << std::endl;
                break;
            } else if (ch == 127 || ch == '\b') { // Handle backspace
                if (!command.empty()) {
                    command.pop_back();
                    std::cout << "\b \b"; // Move cursor back, print space, move cursor back again
                }
            } else {
                command += ch;
                std::cout << ch;
            }
        }

        if (command == "exit") {
            break;
        }

        std::string result = execute(command);
        removeCarriageReturn(result);
        std::cout << result;
//        std::cout << result << std::endl;
    }

    // Restore terminal settings
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
}

std::string Terminal::execute(const std::string &command, bool waitForResponse) {
    // Default execute method, just returns the command
    return command;
}
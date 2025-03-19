#pragma once

#include <iostream>
#include <string>

class Terminal {
public:
    Terminal(const std::string &prompt = "> ");
    void run(void);
    virtual std::string execute(const std::string &command, bool waitForResponse = true) = 0;

private:
    std::string prompt;
};
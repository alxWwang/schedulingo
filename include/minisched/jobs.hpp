#pragma once

#include <iostream>     // std::cout, std::endl (printJob)
#include <sstream>      // std::istringstream
#include <string>       // std::string, std::stoi
#include <vector>       // std::vector

class Jobs {
    public:
        std::string title = "";
        int timeLimit = 0;
        bool gpu = false;
        std::vector<std::string> command;

        Jobs(std::istringstream& in){
            std::string timeLimit; std::string word;
            if (!(in >> this->title >> timeLimit)) throw "No title or time limit" ;
            this->timeLimit = std::stoi(timeLimit);
            while (in >> word){
                if (word == "[gpu]"){
                    this->gpu = true;
                    continue;
                }
                this->command.push_back(std::move(word));
            }
            if (this->command.size() == 0) throw "No command";
        }
        Jobs() = default;

        // Member Function (Method) defined inside the class
        void printJob() const {
            std::cout << "Job title: " << title << std::endl;
            std::cout << "Time limit: " << timeLimit << std::endl;
            std::cout << "Uses gpu? :" << (gpu ? "True": "False") << std::endl;
            std::cout << "Command: ";
            for (const std::string& cmd : command){
                std::cout << cmd << " ";
            }
            std::cout << std::endl << std::endl;
        }
};

#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <sstream>

using namespace std;

class Jobs {
    public:
        string title = "";
        int timeLimit = 0;
        bool gpu = false;
        vector<string> command;

        Jobs(istringstream& in){
            string timeLimit; string word;
            if (!(in >> this->title >> timeLimit)) throw "No title or time limit" ;
            this->timeLimit = stoi(timeLimit);
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
            cout << "Job title: " << title << endl;
            cout << "Time limit: " << timeLimit << endl;
            cout << "Uses gpu? :" << (gpu ? "True": "False") << endl;
            cout << "Command: ";
            for (const string& cmd : command){
                cout << cmd << " ";
            }
            cout << endl << endl;
        }
};
#pragma once

#include <string>
#include <vector>
#include <iostream>

using namespace std;

class Jobs {
    public:
        string title = "";
        int timeLimit = 0;
        bool gpu = false;
        vector<string> command;

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
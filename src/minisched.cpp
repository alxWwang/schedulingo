#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>

#include "minisched/jobs.hpp"

using namespace std;

void run_funcs(vector<string>& args){
    printf("Job Title: %s\n", args[0].c_str());
    printf("Time limit: %d\n", stoi(args[1]));
    printf("Command: ");
    for (size_t i = 2; i < args.size(); i ++ ){
        cout << (args[i]) << " ";
    }
    printf("\n\n");
}

int main(int argc, char *argv[]){
    if (argc < 2) {
        cout << "usage: " << argv[0] << "<file>";
        return 1;
    }
    string path = argv[1];
    ifstream incoming_job(path);
    string line;
    if (!incoming_job) {
        cerr << "File " << argv[1] << " is unavailable\n";
        return 1;
    }

    while (getline(incoming_job, line)){
        if (line.size() == 0 || line[0] == '#') continue;
        istringstream in(line);
        string timeLimit = "0"; string word = "";
        Jobs jb;

        try{
            if (!(in >> jb.title >> timeLimit)) continue ;
            jb.timeLimit = stoi(timeLimit);
            while (in >> word){
                if (word == "[gpu]"){
                    jb.gpu = true;
                    continue;
                }
                jb.command.push_back(std::move(word));
            }
            if (jb.command.size() == 0) continue;
        }catch(...){
            continue;
        }
        jb.printJob();
    }
    return 0;
}


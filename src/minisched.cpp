#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>

#include "minisched/jobs.hpp"
#include "minisched/runner.hpp"

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

void gather_result(unordered_map<pid_t, Jobs>& jobMap){
    for (size_t i = 0; i < jobMap.size(); i++){
        int status = 0;
        pid_t done = waitpid(-1, &status, 0);
    
        if (WIFEXITED(status)){
            std::cout << "Finished Job: " << jobMap[done].title << " with code: " << WIFEXITED(status) << " status: " << WEXITSTATUS(status) << std::endl;
        }
    }
}

int main(int argc, char *argv[]){
    if (argc < 2) {
        cout << "usage: " << argv[0] << "<file>";
        return 1;
    }
    string path = argv[1];
    ifstream incoming_job(path);
    string line;
    unordered_map<pid_t, Jobs> jobMap;

    if (!incoming_job) {
        cerr << "File " << argv[1] << " is unavailable\n";
        return 1;
    }

    while (getline(incoming_job, line)){
        if (line.size() == 0 || line[0] == '#') continue;
        istringstream in(line);
        string timeLimit; string word;
        Jobs jb;

        try{
            if (!(in >> jb.title >> timeLimit)) continue ;
            jb.timeLimit = stoi(timeLimit);
            string rest;
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
        runner(jb, jobMap);
    }
    gather_result(jobMap);
    return 0;
}


#include <iostream>
#include <unistd.h>     // fork, execvp, getpid
#include <sys/wait.h>   // waitpid, WIFEXITED, WEXITSTATUS

#include <unordered_map>
#include <vector>
#include "minisched/jobs.hpp"

void c_process(Jobs& job){
    std::vector<char *> chars;
    for(auto& st: job.command){
        chars.push_back(st.data());
    }
    chars.push_back(nullptr);
    execvp(job.command[0].c_str(), chars.data());
}

int runner(Jobs& job, unordered_map<pid_t, Jobs>& jobMap){
    pid_t pid = fork();
    if (pid < 0){
        perror("Fork failed");
        return 1;
    }

    jobMap[pid] = job;
    if (pid == 0) { // Child process
        std::cout.flush();
        c_process(job);
        _exit(127);
    } else {        // Parent Process
        std::cout << "Started Job as parent with title: " << job.title << std::endl;
    }
    return 0;
};


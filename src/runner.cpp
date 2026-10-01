#include <iostream>
#include <unistd.h>     // fork, execvp, getpid
#include <sys/wait.h>   // waitpid, WIFEXITED, WEXITSTATUS
#include <unordered_map>

#include "minisched/jobs.hpp"

int runner(Jobs& job, unordered_map<pid_t, Jobs>& jobMap){
    pid_t pid = fork();
    jobMap[pid] = job;
    if (pid < 0){
        perror("Fork failed");
        return 1;
    }
    if (pid == 0) { // Child process
        sleep(job.timeLimit);
        _exit(127);
    } else {        // Parent Process
        std::cout << "Started Job as parent with title: " << job.title << std::endl;
    }
    return 0;
};


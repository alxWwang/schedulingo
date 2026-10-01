#include <iostream>
#include <unistd.h>     // fork, execvp, getpid
#include <sys/wait.h>   // waitpid, WIFEXITED, WEXITSTATUS

#include <unordered_map>
#include <vector>
#include "minisched/jobs.hpp"
#include "minisched/colors.hpp"

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
    if (pid == 0) { // Child process
        std::cout.flush();
        c_process(job);
        perror("execvp");
        _exit(127);
    } else {        // Parent Process
        std::cout << color::YELLOW << "Started Job as parent with title: " << job.title << color::RESET << std::endl;
        jobMap[pid] = job;
    }
    return 0;
};

int runner_no_map(Jobs& job){
    std::vector<char*> args;
    for (auto& s : job.command) args.push_back(s.data());
    args.push_back(nullptr);
    
    pid_t pid = fork();
    if (pid < 0){
        return -1;
    }
    if (pid == 0) { // Child process
        execvp(args[0], args.data());
        const char msg[] = "execvp failed\n";
        write(STDERR_FILENO, msg, sizeof msg - 1);
        _exit(127);
    }
    // Parent Process
    int status;
    waitpid(pid, &status, 0);
    if (WIFEXITED(status)){
        return WEXITSTATUS(status);
    }
    return -1;
};

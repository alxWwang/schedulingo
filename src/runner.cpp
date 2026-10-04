#include "minisched/runner.hpp"

#include <cstdio>           // perror
#include <iostream>         // std::cout
#include <unordered_map>    // std::unordered_map
#include <vector>           // std::vector

#include <sys/wait.h>       // waitpid, WIFEXITED, WEXITSTATUS
#include <unistd.h>         // fork, execvp, write, _exit, STDERR_FILENO

#include <thread>
#include <chrono>
#include <signal.h>

#include "minisched/colors.hpp"
#include "minisched/jobs.hpp"

void c_process(Jobs& job){
    std::vector<char *> chars;
    for(auto& st: job.command){
        chars.push_back(st.data());
    }
    chars.push_back(nullptr);
    execvp(job.command[0].c_str(), chars.data());
}

int runner(Jobs& job, std::unordered_map<pid_t, Jobs>& jobMap){
    
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
        return RUN_ERROR;
    }
    if (pid == 0) { // Child process
        execvp(args[0], args.data());
        const char msg[] = "execvp failed\n";
        write(STDERR_FILENO, msg, sizeof msg - 1);
        _exit(127);
    }
    // Parent Process
    int status = 0;
    auto time_limit = std::chrono::steady_clock::now() + std::chrono::seconds(job.timeLimit);
    auto grace_period_tl = time_limit + std::chrono::seconds(2);
    bool timed_out = false;

    pid_t ret_pid;
    while ((ret_pid = waitpid(pid, &status, WNOHANG))== 0){
        if (!timed_out && time_limit < std::chrono::steady_clock::now()){
            kill(pid, SIGTERM);
            timed_out = true;
        }
        if (timed_out && grace_period_tl < std::chrono::steady_clock::now()){
            kill(pid, SIGKILL);
            ret_pid = waitpid(pid, &status, 0);
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    if (ret_pid == -1) return RUN_ERROR;                     // -1
    if (timed_out)     return EXIT_TIMEOUT;                  // -2
    if (WIFEXITED(status))   return WEXITSTATUS(status);     // 0 success, rest failed
    if (WIFSIGNALED(status)) return 128 + WTERMSIG(status);  // crashed / killed by someone else
    return RUN_ERROR;
};

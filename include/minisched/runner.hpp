#pragma once

#include <sys/types.h>      // pid_t
#include <unordered_map>    // std::unordered_map

#include "minisched/jobs.hpp"

// Runs job.command as a child process (fork -> execvp -> waitpid).
// Returns the child's exit code, or -1 if it could not be run or was killed.
int runner(Jobs& job, std::unordered_map<pid_t, Jobs>& jobMap);
int runner_no_map(Jobs& job);
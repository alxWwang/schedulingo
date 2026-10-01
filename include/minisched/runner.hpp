#pragma once

#include "minisched/jobs.hpp"

// Runs job.command as a child process (fork -> execvp -> waitpid).
// Returns the child's exit code, or -1 if it could not be run or was killed.
int runner(Jobs& job, unordered_map<pid_t, Jobs>& jobMap);

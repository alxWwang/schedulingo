#pragma once

#include <sys/types.h>      // pid_t
#include <unordered_map>    // std::unordered_map

#include "minisched/jobs.hpp"

// Special results from runner_no_map (real exit codes are always 0..255).
inline constexpr int RUN_ERROR    = -1;   // fork/waitpid failed: the job never ran properly
inline constexpr int EXIT_TIMEOUT = -2;   // killed because it exceeded job.timeLimit

int runner(Jobs& job, std::unordered_map<pid_t, Jobs>& jobMap);

// Runs job.command as a child process and waits for it, enforcing job.timeLimit.
// Returns:
//   0          the job succeeded
//   1..255     the job exited with that error code (127 = command not found)
//   128 + N    the job was killed by signal N (crash, or killed by someone else)
//   EXIT_TIMEOUT / RUN_ERROR  (see above)
int runner_no_map(Jobs& job);
#pragma once
#include <iostream>         // std::cout, std::cerr
#include <iomanip>
#include "minisched/spinlock.hpp"

// Name of the shared memory region. minisched creates it, schedtop opens it.
inline constexpr const char* SHM_NAME = "/scheduling_status";

struct Stats {
    int done = 0;
    int failed = 0;
    double time_elapsed = 0;
};

enum class JobStatus {
    WAITING,
    DONE,
    RUNNING,
    FAILED
};

// `inline` lets this function body live in a header that several .cpp files include.
inline const char* to_string(JobStatus s){
    switch (s){
        case JobStatus::WAITING: return "Waiting";
        case JobStatus::DONE:    return "Done";
        case JobStatus::RUNNING: return "Running";
        case JobStatus::FAILED:  return "Failed";
    }
    return "?";
}

struct Row {
    char title[64];
    int timelimit;
    double start_time;
    bool gpu;
    JobStatus status_;
};

inline void print_ssq(Row& r){
    const int name_length = 30;
    const int time_length = 5;
    const int gpu_length = 8;
    const int status_length = 8;

    std::cout << std::left << std::setw(name_length) << r.title << '|';
    std::cout << std::left << std::setw(time_length) << r.timelimit << '|';
    std::cout << std::left << std::setw(gpu_length) << (r.gpu ? "Use GPU" : "No GPU") << '|';
    std::cout << std::left << std::setw(status_length) << to_string(r.status_) << '|';
    std::cout << std::endl;
}

struct SchedulingStatus {
    Stats stats_;
    SpinLock lock;
    bool running = false;
    
    int row_count = 0;
    Row rows[512];
};
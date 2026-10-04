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
    FAILED,
    TIMEOUT
};

// `inline` lets this function body live in a header that several .cpp files include.
inline const char* to_string(JobStatus s){
    switch (s){
        case JobStatus::WAITING: return "Waiting";
        case JobStatus::DONE:    return "Done";
        case JobStatus::RUNNING: return "Running";
        case JobStatus::FAILED:  return "Failed";
        case JobStatus::TIMEOUT: return "Timeout";
    }
    return "?";
}

struct Row {
    char title[64];
    int timelimit;
    double start_time = -1;
    double end_time = -1;
    bool gpu;
    JobStatus status_;
};

inline void print_ssq(Row& r, double time_now){
    const int name_length = 30;
    const int time_length = 5;
    const int gpu_length = 8;
    const int status_length = 8;
    const int runtime_length = 10;

    std::cout << std::left << std::setw(name_length) << r.title << '|';
    std::cout << std::left << std::setw(time_length) << r.timelimit << '|';
    std::cout << std::left << std::setw(gpu_length) << (r.gpu ? "Use GPU" : "No GPU") << '|';
    std::cout << std::left << std::setw(status_length) << to_string(r.status_) << '|';
    if (r.status_ != JobStatus::WAITING){
        double seconds_total = (r.status_ == JobStatus::RUNNING ? time_now : r.end_time) - r.start_time;
        std::cout << std::left << std::setw(runtime_length) << seconds_total << '|'; 
    }

    // if status is WAITING dont use at all
    // if status is RUNNING use time now
    // if status is done timeout or failed use end time
    std::cout << std::endl;
}

struct SchedulingStatus {
    Stats stats_;
    SpinLock lock;
    bool running = false;
    
    int row_count = 0;
    Row rows[512];
};
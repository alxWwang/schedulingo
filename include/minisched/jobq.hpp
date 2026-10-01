#pragma once
#include <minisched/jobs.hpp>
#include <queue>
#include <mutex>
#include <unordered_map>

class JobQueue{
    public:
        void push(Jobs job);
        bool pop(Jobs& out);

    private:
        std::queue<Jobs> jobs_;
        std::mutex lock_;
};

// Test helper: runs a few jobs through a shared JobQueue using threads.
void run_job_queue(vector<Jobs>& JobsList);

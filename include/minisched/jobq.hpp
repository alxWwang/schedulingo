#pragma once
#include <minisched/jobs.hpp>
#include <minisched/colors.hpp>
#include <queue>
#include <mutex>
#include <unordered_map>
#include <ostream>

class JobQueue{
    public:
        void push(Jobs job);
        bool pop(Jobs& out);
        void minirunner(int id);
        
        private:
        std::queue<Jobs> jobs_;
        std::mutex lock_;
        std::mutex write_lock;
        
};
void run_job_queue(vector<Jobs>& JobsList);

// Test helper: runs a few jobs through a shared JobQueue using threads.


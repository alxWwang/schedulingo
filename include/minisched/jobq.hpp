#pragma once
#include <minisched/jobs.hpp>
#include <minisched/colors.hpp>
#include <queue>
#include <mutex>
#include <unordered_map>
#include <ostream>
#include <semaphore>

class JobQueue{
    public:
        void push(Jobs job);
        bool pop(Jobs& out);
        void minirunner(int id);
        
        private:
        std::queue<Jobs> jobs_;
        std::mutex lock_;
        std::mutex write_lock;
        std::counting_semaphore<2> gpu_sem{2};

        int get_process_ct();
        void run_with_print(Jobs& out, int id);
        int decrement_process_ct();
        int active_process_ct = 0;

};
void run_job_queue(vector<Jobs>& JobsList);

// Test helper: runs a few jobs through a shared JobQueue using threads.


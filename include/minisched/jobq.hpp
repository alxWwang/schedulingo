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

        std::condition_variable thread_wake;
        std::queue<Jobs> cpu_jobs;
        std::queue<Jobs> gpu_jobs;

        int active_process_ct = 0;
        int gpu_resources_ct = 2;

        void run_with_print(Jobs& out, int id);

        bool gpuRunnable(){
            // wake if gpu_jobs is not empty && there are resources available
            return (!gpu_jobs.empty() && gpu_resources_ct > 0);
        }

        void gather_resources(Jobs& out){
            if (out.gpu) gpu_resources_ct --;
            active_process_ct++;
        }
        void relief_resources(Jobs& out){
            if (out.gpu) gpu_resources_ct ++;
            active_process_ct--;
        }


};
void run_job_queue(vector<Jobs>& JobsList);

// Test helper: runs a few jobs through a shared JobQueue using threads.


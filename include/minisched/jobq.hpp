#pragma once

#include <condition_variable>   // std::condition_variable
#include <iostream>             // std::cout (print_status)
#include <mutex>                // std::mutex
#include <queue>                // std::queue
#include <thread>               // std::thread::hardware_concurrency
#include <vector>               // std::vector

#include <minisched/jobs.hpp>
#include <minisched/spinlock.hpp>
#include <minisched/status.hpp>



class JobQueue{
    public:
        void push(Jobs job);
        bool pop(Jobs& out);
        void minirunner(int id);
        void print_status(){
            spin_lock.lock();
            std::cout << "Done: " << stats_.done << " Failed: " <<  stats_.failed << " Time elapsed: " << stats_.time_elapsed;
            spin_lock.unlock();
        }
        void run_job_queue(std::vector<Jobs>& JobsList, SchedulingStatus* ssq);

        JobQueue() = default;
        void set_gpu(int gpu_resources_ct){
            this->gpu_resources_ct = gpu_resources_ct;
        }
        void set_worker(int worker_ct){
            this->worker_ct = worker_ct;
        }
        
    private:
        std::queue<Jobs> jobs_;
        std::mutex lock_;
        std::mutex write_lock;

        std::condition_variable thread_wake;
        std::queue<Jobs> cpu_jobs;
        std::queue<Jobs> gpu_jobs;

        int active_process_ct = 0;
        int worker_ct = std::thread::hardware_concurrency();
        int gpu_resources_ct = 2;
        Stats stats_;
        SpinLock spin_lock;

        int run_with_print(Jobs& out, int id);

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

        void update_status(bool success, double time_elapsed){
            spin_lock.lock();
            if (success){
                stats_.done++;
            }else{
                stats_.failed++;
            }
            stats_.time_elapsed += time_elapsed;
            spin_lock.unlock();
        }
};

// Test helper: runs a few jobs through a shared JobQueue using threads.


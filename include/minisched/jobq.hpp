#pragma once
#include <minisched/jobs.hpp>
#include <minisched/colors.hpp>
#include <queue>
#include <mutex>
#include <unordered_map>
#include <ostream>
#include <semaphore>
#include <atomic>

struct Stats {
    int done = 0;
    int failed = 0;
    double time_elapsed = 0;
};

class SpinLock{

    // THIS IS LITERALLY A MUTEX LOCK BUT IT DOES NOT SLEEP THE THREAD
    // if we were to implement this as a mutex, we can add another line before the while loop to yield to the OS
    public:
        void lock(){
            while(is_taken.test_and_set(std::memory_order_acquire)){}
            // test and set returns old value and sets it to true
            // if the original value was false (NOT is_taken) then it will break the loop
            // if the original value was true (is_taken) then it will keep looping 
        }
        void unlock(){
            is_taken.clear(std::memory_order_release);
            // sets is_taken to false
        }
        bool try_lock(){
            return !is_taken.test_and_set(std::memory_order_acquire);
            // return true if the original value was false (NOT is_taken)
            // locking was successful because it was free
            // locking was unsiccessful because it was not free
        }
    private:
        std::atomic_flag is_taken = ATOMIC_FLAG_INIT;
};

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
        
    private:
        std::queue<Jobs> jobs_;
        std::mutex lock_;
        std::mutex write_lock;

        std::condition_variable thread_wake;
        std::queue<Jobs> cpu_jobs;
        std::queue<Jobs> gpu_jobs;

        int active_process_ct = 0;
        int gpu_resources_ct = 4;
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
void run_job_queue(vector<Jobs>& JobsList);

// Test helper: runs a few jobs through a shared JobQueue using threads.


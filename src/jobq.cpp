#include "minisched/jobq.hpp"

#include <chrono>       // std::chrono (job and run timing)
#include <iostream>     // std::cout
#include <mutex>        // std::lock_guard, std::unique_lock
#include <thread>       // std::thread
#include <vector>       // std::vector

#include "minisched/colors.hpp"
#include "minisched/runner.hpp"


void JobQueue::push(Jobs job){
    std::lock_guard<std::mutex> lock(lock_);
    job.gpu ? gpu_jobs.push(job) : cpu_jobs.push(job);
    thread_wake.notify_one();
}

bool JobQueue::pop(Jobs& out){
    std::unique_lock<std::mutex> lock(lock_);
    // at this point, either jobs > 0 or active process == 0
    thread_wake.wait(lock, [&]{return this->gpuRunnable() || (!cpu_jobs.empty()) || active_process_ct == 0; });

    // if both q's are empty and no process is running any more were done.
    if (active_process_ct == 0 && (gpu_jobs.empty() && cpu_jobs.empty())){ 
        thread_wake.notify_all();
        return false;
    }

    // if jobs exist, and there is an active process or not -> we work
    if (this->gpuRunnable()){
        out = std::move(gpu_jobs.front());
        gpu_jobs.pop();
    }else{
        out = std::move(cpu_jobs.front());
        cpu_jobs.pop();
    }
    gather_resources(out);
    return true;
}

int JobQueue::run_with_print(Jobs& out, int id){
    {
        std::lock_guard<std::mutex> lock(write_lock);
        std::cout << color::YELLOW << id << " -> Started Job as parent with title: " << out.title << color::RESET << std::endl;
    }
    int runner_status = runner_no_map(out);
    {
        std::lock_guard<std::mutex> lock(write_lock);
        std::cout << color::RED << id << " -> Finished Job: " << out.title << " status: " << runner_status << color::RESET << std::endl;
    }
    return runner_status;
}

void JobQueue::minirunner(int id){
    Jobs out;
    while(this->pop(out)){ // while not line 26
        auto start_time = std::chrono::system_clock::now();
        int runner_status = this->run_with_print(out, id);
        auto end_time = std::chrono::system_clock::now();
        std::chrono::duration<double> t_elapsed = (end_time-start_time);
        {
            std::lock_guard<std::mutex> lock(lock_);
            relief_resources(out);
        }
        update_status(runner_status != 127, t_elapsed.count());
        thread_wake.notify_one();
    }
}

void JobQueue::run_job_queue(std::vector<Jobs>& JobsList, SchedulingStatus* ssq){
    std::cout << "hello world: starting " << this->worker_ct <<" workers" << std::endl;

    std::vector<std::thread> threads_l;
    auto start_time = std::chrono::system_clock::now();

    for (const auto& job: JobsList){
        std::lock_guard<SpinLock> loc(ssq->lock);
        this->push(job);
    }
    
    for (int i = 0; i< this->worker_ct; i ++){
        threads_l.emplace_back(&JobQueue::minirunner, this, i);
    }
    for (std::thread& t: threads_l){
        if(t.joinable()){
            t.join();
        }
    }
    auto end_time = std::chrono::system_clock::now();
    std::chrono::duration<double> t_elapsed = (end_time-start_time);


    this->print_status();
    std::cout << "Real time: " << t_elapsed.count();
} 
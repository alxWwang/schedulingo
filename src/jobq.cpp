#include "minisched/jobq.hpp"

#include <chrono>       // std::chrono (job and run timing)
#include <cstring>      // std::strncpy
#include <iostream>     // std::cout
#include <mutex>        // std::lock_guard, std::unique_lock
#include <thread>       // std::thread
#include <vector>       // std::vector

#include "minisched/colors.hpp"
#include "minisched/runner.hpp"

// Wall-clock seconds since the epoch: a plain double that minisched and the
// monitor (a different process) interpret the same way.
static double now_secs(){
    return std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count();
}

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

void JobQueue::minirunner(int id, SchedulingStatus* ssq){
    Jobs out;
    while(this->pop(out)){ // while not line 26
        Row* pRow = &ssq->rows[out.row];    // this job's row in shared memory

        double start_time = now_secs();
        {
            std::lock_guard<SpinLock> loc(ssq->lock);
            pRow->start_time = start_time;
            pRow->status_ = JobStatus::RUNNING;
        }

        int runner_status = this->run_with_print(out, id);
        double t_elapsed = now_secs() - start_time;

        {
            std::lock_guard<std::mutex> lock(lock_);
            relief_resources(out);
        }
        update_status(runner_status != 127, t_elapsed);
        {
            std::lock_guard<SpinLock> loc(ssq->lock);
            pRow->status_ = (runner_status!=127 ? JobStatus::DONE : JobStatus::FAILED);
        }
        thread_wake.notify_one();
    }
}

void JobQueue::run_job_queue(std::vector<Jobs>& JobsList, SchedulingStatus* ssq){
    std::cout << "hello world: starting " << this->worker_ct <<" workers" << std::endl;

    std::vector<std::thread> threads_l;
    double start_time = now_secs();

    constexpr int max_rows = sizeof(ssq->rows) / sizeof(ssq->rows[0]);
    {
        std::lock_guard<SpinLock> loc(ssq->lock);
        for (Jobs& job: JobsList){                      // not const: we record job.row
            if (ssq->row_count >= max_rows){
                std::cerr << "status table full, skipping " << job.title << std::endl;
                continue;
            }
            job.row = ssq->row_count++;
            Row* pRow = &ssq->rows[job.row];
            std::strncpy(pRow->title, job.title.c_str(), sizeof(pRow->title) - 1);
            pRow->title[sizeof(pRow->title) - 1] = '\0';   // always terminated, even if truncated
            pRow->timelimit = job.timeLimit;
            pRow->gpu = job.gpu;
            pRow->status_ = JobStatus::WAITING;
        }
        ssq->running = true;
    }
    for (const Jobs& job: JobsList){
        if (job.row >= 0) this->push(job);
    }

    for (int i = 0; i< this->worker_ct; i ++){
        threads_l.emplace_back(&JobQueue::minirunner, this, i, ssq);
    }
    for (std::thread& t: threads_l){
        if(t.joinable()){
            t.join();
        }
    }
    {
        std::lock_guard<SpinLock> loc(ssq->lock);
        ssq->running = false;                         // tells the monitor to stop
    }
    double t_elapsed = now_secs() - start_time;

    this->print_status();
    std::cout << "Real time: " << t_elapsed << std::endl;
}

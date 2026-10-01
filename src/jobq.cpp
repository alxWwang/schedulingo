#include <minisched/jobq.hpp>
#include <minisched/runner.hpp>
#include "minisched/colors.hpp"
#include <thread>
#include <unistd.h> 
#include <semaphore>


void JobQueue::push(Jobs job){
    std::lock_guard<std::mutex> lock(lock_);
    jobs_.push(job);
}

bool JobQueue::pop(Jobs& out){
    std::lock_guard<std::mutex> lock(lock_);
    if(jobs_.size() > 0){
        out = std::move(jobs_.front());
        jobs_.pop();
        return true;
    }
    return false;
}

void JobQueue::minirunner(int id){
    Jobs out;
    while(this->pop(out)){
        {
            std::lock_guard<std::mutex> lock(write_lock);
            std::cout << color::YELLOW << id << "-> Started Job as parent with title: " << out.title << color::RESET << std::endl;
        }
        int runner_status;

        if (out.gpu){
            gpu_sem.acquire();
            runner_status = runner_no_map(out);
            gpu_sem.release();
        }else{
            runner_status = runner_no_map(out);
        }

        {
            std::lock_guard<std::mutex> lock(write_lock);
            std::cout << color::RED << id << "-> Finished Job: " << out.title << " status: " << runner_status << color::RESET << std::endl;
        }
    }
}

void run_job_queue(vector<Jobs>& JobsList){
    int n_threads = std::thread::hardware_concurrency();
    std::cout << "hello world: starting " << n_threads <<" workers" << std::endl;

    std::vector<std::thread> threads_l;
    JobQueue jq;
    for (const auto& job: JobsList){
        jq.push(job);
    }
    for (int i = 0; i< n_threads; i ++){
        threads_l.emplace_back(&JobQueue::minirunner, &jq, i);
    }
    for (thread& t: threads_l){
        if(t.joinable()){
            t.join();
        }
    }
}
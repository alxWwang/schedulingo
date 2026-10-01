#include <minisched/jobq.hpp>
#include <minisched/runner.hpp>
#include "minisched/colors.hpp"
#include <thread>
#include <unistd.h> 


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
        int runner_status = runner_no_map(out);
        {
            std::lock_guard<std::mutex> lock(write_lock);
            std::cout << color::RED << id << "-> Finished Job: " << out.title << " status: " << runner_status << color::RESET << std::endl;
        }
    }
}

void run_job_queue(vector<Jobs>& JobsList){
    std::cout << "hello world: starting 4 workers" << std::endl;

    JobQueue jq;
    for (const auto& job: JobsList){
        jq.push(job);
    }

    std::thread t1(&JobQueue::minirunner, &jq, 1);
    std::thread t2(&JobQueue::minirunner, &jq, 2);
    std::thread t3(&JobQueue::minirunner, &jq, 3);    
    std::thread t4(&JobQueue::minirunner, &jq, 4);

    t1.join();
    t2.join();
    t3.join();
    t4.join();
}
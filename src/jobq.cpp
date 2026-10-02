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
        active_process_ct++;
        return true;
    }
    return false;
}
int JobQueue::get_process_ct(){
    std::lock_guard<std::mutex> lock(lock_);
    return active_process_ct + jobs_.size();
}

int JobQueue::decrement_process_ct(){
    std::lock_guard<std::mutex> lock(lock_);
    active_process_ct --;
    return active_process_ct;
}

void JobQueue::run_with_print(Jobs& out, int id){
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

void JobQueue::minirunner(int id){
    Jobs out;
    // when we have an active process and an empty pop, continue
    // when we have no active process an an empty pop, end
    // when we have an active process and there is a pop, work
    // when we have no active process and there is a pop, work

    do{
        if (this->pop(out)){
            if (!out.gpu) this->run_with_print(out, id);
            else if (gpu_sem.try_acquire_for(50ms)){
                this->run_with_print(out, id);
                gpu_sem.release();
            }else{
                this->push(out);
            }
            this->decrement_process_ct();
        }std::this_thread::sleep_for(50ms)
    }while(get_process_ct() > 0);
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
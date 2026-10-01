#include <minisched/jobq.hpp>

void JobQueue::push(Jobs job){
    std::lock_guard<std::mutex> lock(lock_);
    jobs_.push(std::move(job));
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
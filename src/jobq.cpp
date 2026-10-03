#include <minisched/jobq.hpp>
#include <minisched/runner.hpp>
#include <minisched/colors.hpp>
#include <thread>
#include <unistd.h> 
#include <semaphore>


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

void JobQueue::run_with_print(Jobs& out, int id){
    {
        std::lock_guard<std::mutex> lock(write_lock);
        std::cout << color::YELLOW << id << " -> Started Job as parent with title: " << out.title << color::RESET << std::endl;
    }
    int runner_status = runner_no_map(out);
    {
        std::lock_guard<std::mutex> lock(write_lock);
        std::cout << color::RED << id << " -> Finished Job: " << out.title << " status: " << runner_status << color::RESET << std::endl;
    }
}

void JobQueue::minirunner(int id){
    Jobs out;
    while(this->pop(out)){ // while not line 26
        this->run_with_print(out, id);
        {
            std::lock_guard<std::mutex> lock(lock_);
            relief_resources(out);
        }
        thread_wake.notify_one();
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
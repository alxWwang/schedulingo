#include <cstdlib>          // atoi
#include <cstring>          // strcmp
#include <fstream>          // std::ifstream
#include <iostream>         // std::cout, std::cerr
#include <sstream>          // std::istringstream
#include <string>           // std::string, std::getline
#include <unordered_map>    // std::unordered_map (n_max_runner)
#include <vector>           // std::vector

#include <sys/wait.h>       // waitpid, WIFEXITED, WEXITSTATUS (n_max_runner)
#include <sys/mman.h>   // shm_open, mmap, munmap, shm_unlink
#include <fcntl.h>      // O_CREAT, O_RDWR, O_RDONLY
#include <unistd.h>     // ftruncate, close


#include "minisched/colors.hpp"
#include "minisched/jobq.hpp"
#include "minisched/jobs.hpp"
#include "minisched/runner.hpp"
#include "minisched/status.hpp"

using namespace std;
// constexpr int MAX_N = 5;

void n_max_runner(int max_n, vector<Jobs>& JobsList, unordered_map<pid_t, Jobs>& jobMap){
    size_t jobs_done = 0;
    size_t active_jobs = 0;
    size_t job_index = 0;

    while (jobs_done != JobsList.size()){
        // fill runners, non blocking, fractions of ms.
        while (static_cast<int>(active_jobs) < max_n && job_index < JobsList.size()){ // if there are less jobs then n we start a new job
            runner(JobsList[job_index], jobMap);
            active_jobs ++;
            job_index ++;
        }
        // while the whole job is not done, we capture one at a time.
        if (active_jobs > 0){
            int status = 0;
            pid_t done = waitpid(-1, &status, 0);
        
            if (WIFEXITED(status)){
                std::cout << color::RED << "Finished Job: " << jobMap[done].title << " with code: " << WIFEXITED(status) << " status: " << WEXITSTATUS(status) << color::RESET << std::endl;
            }
            jobs_done ++;
            active_jobs --;
        }

    }
}

int run_monitor(){
    int fd = shm_open(SHM_NAME, O_RDWR, 0600);
    if (fd < 0) {perror("File unavailable"); return 1;}
    void*p = mmap(nullptr, sizeof(SchedulingStatus), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (p == MAP_FAILED) {perror("mmap"); return 1;}
    close(fd);

    SchedulingStatus* pSsq = static_cast<SchedulingStatus*>(p);
    int i = 0;

    while (true){
        pSsq->lock.lock();
        std::cout << "\033[2J\033[H"; 
        SchedulingStatus ssq;
        memcpy(static_cast<void*>(&ssq), pSsq, sizeof(SchedulingStatus));
        pSsq->lock.unlock();
        for (int i = 0; i < ssq.row_count; i++){
            Row &pRow = ssq.rows[i];
            print_ssq(pRow);
        }
        cout << "--" << i << endl;
        i++;
        if (!ssq.running) break;
        this_thread::sleep_for(std::chrono::seconds(1));
    }
    return 0;
}

int main(int argc, char *argv[]){
    if (argc < 2) {
        cout << "usage: " << argv[0] << "<file>";
        return 1;
    }

    JobQueue jq;
    for (int i = 2; i < argc; i ++){
        if (strcmp(argv[i],"--gpu_ct") == 0 && i+1 < argc){
            jq.set_gpu(atoi(argv[i+1]));
        }if (strcmp(argv[i],"--worker_ct") == 0 && i+1 < argc){
            jq.set_worker(atoi(argv[i+1]));
        }if (strcmp(argv[i], "monitor") == 0){
            run_monitor();
            return 0;
        }
    }

    string path = argv[1];
    ifstream incoming_job(path);
    string line;
    // unordered_map<pid_t, Jobs> jobMap;
    vector<Jobs> JobsList;

    if (!incoming_job) {
        cerr << "File " << argv[1] << " is unavailable\n";
        return 1;
    }

    while (getline(incoming_job, line)){
        if (line.size() == 0 || line[0] == '#') continue;
        istringstream in(line);
        try{
            JobsList.push_back(Jobs(in));
        }catch(...){
            continue;
        }
    }

    shm_unlink(SHM_NAME);
    int fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0600);
    if (fd < 0) {perror("shm_open"); return 1;}
    if (ftruncate(fd, sizeof(SchedulingStatus)) < 0) {perror("ftruncate"); return 1;}
    void*p = mmap(nullptr, sizeof(SchedulingStatus), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (p == MAP_FAILED) {perror("mmap"); return 1;}
    close(fd);

    SchedulingStatus* ssq = static_cast<SchedulingStatus*>(p);
    
    ssq->lock.lock();
    ssq->running = true;
    for (const Jobs& jobs: JobsList){
        Row *pRow = &ssq->rows[ssq->row_count];
        std::strncpy(pRow->title, jobs.title.data(), sizeof(pRow->title) -1);
        pRow->title[jobs.title.size()] = '\0';
        pRow->timelimit = jobs.timeLimit;
        pRow->gpu = jobs.gpu;
        pRow->status_ = JobStatus::WAITING;
        ssq->row_count++;
    }
    for (int i = 0; i < ssq->row_count; i++){
        Row &pRow = ssq->rows[i];
        print_ssq(pRow);
    }
    ssq->lock.unlock();
    jq.run_job_queue(JobsList, ssq);

    munmap(p, sizeof(SchedulingStatus));
    shm_unlink(SHM_NAME);

    // n_max_runner(MAX_N, JobsList, jobMap);
    return 0;
}


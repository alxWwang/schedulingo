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
    if (fd < 0) {cerr << "minisched is not running (no status table found)\n"; return 1;}
    void*p = mmap(nullptr, sizeof(SchedulingStatus), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (p == MAP_FAILED) {perror("mmap"); return 1;}
    close(fd);

    SchedulingStatus* pSsq = static_cast<SchedulingStatus*>(p);
    int i = 0;

    while (true){
        pSsq->lock.lock();
        SchedulingStatus ssq;
        memcpy(static_cast<void*>(&ssq), pSsq, sizeof(SchedulingStatus));
        pSsq->lock.unlock();
        std::cout << "\033[2J\033[H";       // clear the screen (outside the lock: printing is slow)
        double time_now = std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count();
        for (int i = 0; i < ssq.row_count; i++){
            Row &pRow = ssq.rows[i];
            print_ssq(pRow, time_now);
        }
        cout << "--" << i << endl;
        i++;
        if (!ssq.running) break;
        this_thread::sleep_for(std::chrono::seconds(1));
    }

    munmap(p, sizeof(SchedulingStatus));
    cout << "monitor stopped"<<endl;
    return 0;
}

int main(int argc, char *argv[]){
    if (argc < 2) {
        cout << "usage: " << argv[0] << " <jobs file> [--gpu_ct N] [--worker_ct N]\n"
             << "       " << argv[0] << " monitor\n";
        return 1;
    }
    // monitor mode doesn't need a jobs file, so check argv[1] before treating it as one
    if (strcmp(argv[1], "monitor") == 0){
        return run_monitor();
    }

    JobQueue jq;
    for (int i = 2; i < argc; i ++){
        if (strcmp(argv[i],"--gpu_ct") == 0 && i+1 < argc){
            jq.set_gpu(atoi(argv[i+1]));
        }if (strcmp(argv[i],"--worker_ct") == 0 && i+1 < argc){
            jq.set_worker(atoi(argv[i+1]));
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

    // run_job_queue fills the table, sets running = true, and sets it false when done
    jq.run_job_queue(JobsList, ssq);

    munmap(p, sizeof(SchedulingStatus));
    shm_unlink(SHM_NAME);

    // n_max_runner(MAX_N, JobsList, jobMap);
    return 0;
}


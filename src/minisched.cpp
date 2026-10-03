#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>

#include "minisched/jobs.hpp"
#include "minisched/runner.hpp"
#include "minisched/colors.hpp"
#include "minisched/jobq.hpp"

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
    jq.run_job_queue(JobsList);
    // n_max_runner(MAX_N, JobsList, jobMap);
    return 0;
}


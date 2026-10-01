#pragma once
#include <minisched/jobs.hpp>
#include <queue>
#include <mutex>

class JobQueue{
    public:
        void push(Jobs job);
        bool pop(Jobs& out);

    private:
        std::queue<Jobs> jobs_;
        std::mutex lock_;
};
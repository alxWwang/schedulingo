#pragma once

#include <atomic>


class SpinLock{

    // THIS IS LITERALLY A MUTEX LOCK BUT IT DOES NOT SLEEP THE THREAD
    // if we were to implement this as a mutex, we can add another line before the while loop to yield to the OS
    public:
        void lock(){
            while(is_taken.test_and_set(std::memory_order_acquire)){}
            // test and set returns old value and sets it to true
            // if the original value was false (NOT is_taken) then it will break the loop
            // if the original value was true (is_taken) then it will keep looping 
        }
        void unlock(){
            is_taken.clear(std::memory_order_release);
            // sets is_taken to false
        }
        bool try_lock(){
            return !is_taken.test_and_set(std::memory_order_acquire);
            // return true if the original value was false (NOT is_taken)
            // locking was successful because it was free
            // locking was unsiccessful because it was not free
        }
    private:
        std::atomic_flag is_taken = ATOMIC_FLAG_INIT;
};

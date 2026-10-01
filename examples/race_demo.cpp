// Demo: two threads incrementing a shared counter, without and with a mutex.
// Build: clang++ -std=c++20 -O2 examples/race_demo.cpp -o race_demo && ./race_demo
#include <iostream>
#include <thread>
#include <mutex>

int main() {
    constexpr int N = 1'000'000;

    // ---- 1. No lock: a data race ----
    int counter = 0;
    auto add_unsafe = [&counter]() {          // lambda: a small function that
        for (int i = 0; i < N; i++)           // captures `counter` by reference
            counter++;
    };
    std::thread t1(add_unsafe);               // starts running immediately
    std::thread t2(add_unsafe);
    t1.join();                                // wait for t1 to finish
    t2.join();
    std::cout << "no lock:   " << counter << "  (expected " << 2 * N << ")\n";

    // ---- 2. With a mutex ----
    int safe = 0;
    std::mutex m;
    auto add_safe = [&safe, &m]() {
        for (int i = 0; i < N; i++) {
            std::lock_guard<std::mutex> lock(m);   // lock now, unlock at end of scope
            safe++;
        }
    };
    std::thread t3(add_safe);
    std::thread t4(add_safe);
    t3.join();
    t4.join();
    std::cout << "mutex:     " << safe << "  (expected " << 2 * N << ")\n";
}

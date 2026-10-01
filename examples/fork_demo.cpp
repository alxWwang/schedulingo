// Demo: fork() -> execvp() -> waitpid()
// Build: clang++ -std=c++20 -Wall -Wextra examples/fork_demo.cpp -o fork_demo && ./fork_demo
#include <iostream>
#include <unistd.h>     // fork, execvp, getpid
#include <sys/wait.h>   // waitpid, WIFEXITED, WEXITSTATUS

int main() {
    int x = 100;
    std::cout << "[parent] my pid is " << getpid() << "\n";

    std::cout.flush();                  // empty the output buffer BEFORE forking (see notes)
    pid_t pid = fork();                 // <-- ONE process goes in, TWO come out

    if (pid < 0) {                      // fork failed
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        // ===== CHILD: fork() returned 0 =====
        x = 999;                        // only changes the CHILD's copy of x
        std::cout << "[child ] pid " << getpid() << ", x = " << x << "\n";
        std::cout.flush();              // exec wipes this process, including unflushed output

        // replace this child with a different program: `echo hello from echo`
        char* args[] = { (char*)"echo", (char*)"hello from echo", nullptr };
        execvp(args[0], args);

        // execvp only returns if it FAILED (e.g. program not found)
        perror("execvp");
        _exit(127);
    }

    // ===== PARENT: fork() returned the child's pid =====
    std::cout << "[parent] started child " << pid << "\n";

    int status = 0;
    waitpid(pid, &status, 0);           // block until that child finishes

    if (WIFEXITED(status))
        std::cout << "[parent] child exited with code " << WEXITSTATUS(status) << "\n";

    std::cout << "[parent] x = " << x << "  (unchanged!)\n";
    return 0;
}

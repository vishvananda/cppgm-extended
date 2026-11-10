#include <cstdio>
#include <csignal>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <unistd.h>
extern "C" void asm_stop(int*);
extern "C" long raw_flagged_caller(long);
int main() {
    for (long value = -31; value < 32; ++value)
        if (raw_flagged_caller(value) != value + 12) return 6;
    void* memory = mmap(0, 4096, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANON, -1, 0);
    if (memory == MAP_FAILED) return 1;
    int* state = static_cast<int*>(memory);
    *state = 0;
    pid_t child = fork();
    if (child < 0) return 2;
    if (child == 0) {
        rlimit limit = {0, 0};
        if (setrlimit(RLIMIT_CORE, &limit) != 0) _exit(42);
        asm_stop(state);
        _exit(41);
    }
    int status = 0;
    if (waitpid(child, &status, 0) != child) return 3;
    bool observed = WIFSIGNALED(status) && WTERMSIG(status) == SIGILL && *state == 77;
    if (munmap(memory, 4096) != 0) return 4;
    if (!observed) return 5;
    std::puts("nonreturning assembly preserves its effects and explicit CFG termination");
}

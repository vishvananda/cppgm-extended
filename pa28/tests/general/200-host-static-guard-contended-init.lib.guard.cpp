#include <pthread.h>
#include <sched.h>
#include <stdlib.h>
int calls, attempts, release_first;
int load(const int* value) { return __atomic_load_n(value, __ATOMIC_SEQ_CST); }
extern "C" int __real___cxa_guard_acquire(long long*);
extern "C" int __wrap___cxa_guard_acquire(long long* guard) {
  __atomic_add_fetch(&attempts, 1, __ATOMIC_SEQ_CST);
  return __real___cxa_guard_acquire(guard);
}
int read();
int next() {
  __atomic_add_fetch(&calls, 1, __ATOMIC_SEQ_CST);
  while (!load(&release_first)) sched_yield();
  return 7;
}
void* enter(void*) {
  if (read() != 7) abort();
  return 0;
}
int main() {
  pthread_t first, second;
  if (pthread_create(&first, 0, enter, 0)) return 2;
  while (!load(&calls)) sched_yield();
  if (pthread_create(&second, 0, enter, 0)) {
    __atomic_store_n(&release_first, 1, __ATOMIC_SEQ_CST);
    pthread_join(first, 0);
    return 3;
  }
  // The ABI protocol reaches acquire twice; the old plain guard reaches next
  // twice. Either outcome releases the initializer without a scheduling delay.
  while (load(&attempts) < 2 && load(&calls) < 2) sched_yield();
  __atomic_store_n(&release_first, 1, __ATOMIC_SEQ_CST);
  if (pthread_join(first, 0) || pthread_join(second, 0)) return 4;
  return load(&calls) != 1 || load(&attempts) != 2;
}

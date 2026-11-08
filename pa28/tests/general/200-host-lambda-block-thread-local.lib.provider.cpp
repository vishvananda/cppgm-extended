#include <pthread.h>
extern "C" int exercise();
static void *run(void *output) {
    *static_cast<int*>(output) = exercise() == 45 && exercise() == 67;
    return 0;
}
extern "C" int host_threads() {
    int first = 0, second = 0;
    pthread_t a, b;
    if (pthread_create(&a, 0, run, &first)) return 2;
    if (pthread_create(&b, 0, run, &second)) { pthread_join(a, 0); return 3; }
    int joined_a = pthread_join(a, 0);
    int joined_b = pthread_join(b, 0);
    return joined_a || joined_b || !first || !second;
}

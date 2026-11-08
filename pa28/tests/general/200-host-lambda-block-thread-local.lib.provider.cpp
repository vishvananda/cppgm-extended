#include <thread>
extern "C" int exercise();
extern "C" int host_threads() {
    int first = 0, second = 0;
    std::thread a([&] { first = exercise() == 45 && exercise() == 67; });
    std::thread b([&] { second = exercise() == 45 && exercise() == 67; });
    a.join(); b.join();
    return !first || !second;
}

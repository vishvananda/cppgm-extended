// Thread storage is destroyed before static storage, independently of definition order.
extern "C" void abort() noexcept;
int destroyed;
struct Item { ~Item() { destroyed = 1; } };
thread_local Item item;
struct Check { ~Check() { if (destroyed != 1) abort(); } } check;
int main() { (void)&item; return destroyed; }

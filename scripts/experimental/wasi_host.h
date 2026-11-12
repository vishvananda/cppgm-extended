// Experimental single-threaded compiler host. The pinned WASI libc has
// ordinary data storage for errno; libc++ uses the same object layouts
// with non-atomic reference counts in a host with no threads.
#include <__config_site>
#undef _LIBCPP_HAS_THREADS
#define _LIBCPP_HAS_THREADS 0
#undef _LIBCPP_HAS_THREAD_API_PTHREAD
#define _LIBCPP_HAS_THREAD_API_PTHREAD 0
#define _Thread_local

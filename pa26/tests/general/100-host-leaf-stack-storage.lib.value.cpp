extern "C" long host_value(long seed) { volatile long storage[32]; for (int i = 0; i != 32; ++i) storage[i] = seed + i; return storage[0] * 2 + 1; }

extern "C" long long touch_globals(int);
extern "C" long long observe(int amount) { return touch_globals(amount); }

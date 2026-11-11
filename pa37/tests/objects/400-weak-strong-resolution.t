extern int chosen;
extern "C" int choice();
extern "C" int from_weak();
extern "C" int observe() { return chosen + choice() + from_weak(); }

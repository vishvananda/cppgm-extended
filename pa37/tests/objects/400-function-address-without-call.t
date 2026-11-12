using Function = int (*)();
extern "C" int answer() noexcept { return 17; }
extern "C" Function get_address() { return &answer; }
extern "C" int present() { return get_address() ? 17 : 1; }

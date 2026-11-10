int safe() noexcept;
int (*ordinary)() = &safe;
int call() { return ordinary(); }

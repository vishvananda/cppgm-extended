void* operator new(unsigned long, void*) noexcept;
void* allocate(void* storage) { return __builtin_operator_new(4, storage); }

void operator delete(void*, void*) noexcept;
void release(void* storage) { __builtin_operator_delete(storage, storage); }

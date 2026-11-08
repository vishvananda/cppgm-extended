typedef float Vector __attribute__((vector_size(16)));
extern "C" void stream_ps(void* pointer, Vector value) {
    __asm__ __volatile__("movntps %1, (%0)" : : "r"(pointer), "x"(value) : "memory");
}

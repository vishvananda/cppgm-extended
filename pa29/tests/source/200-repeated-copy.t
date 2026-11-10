extern "C" void repeated_copy(unsigned char* destination, const unsigned char* source, unsigned long count) {
    asm volatile("rep movsb" : "+D"(destination), "+S"(source), "+c"(count) : : "memory");
}

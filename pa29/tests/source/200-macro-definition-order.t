asm(".macro step result:req\nadd $2,\\result\n.endm\n");
extern "C" long macro_source_early(long value) {
    asm("step %0" : "+r"(value) : : "cc");
    return value;
}
template<class T> T macro_source_template(T value) {
    asm("step %0" : "+r"(value) : : "cc");
    return value;
}
struct MacroMember {
    static long early(long value) {
        asm("step %0" : "+r"(value) : : "cc");
        return value;
    }
};
asm(".purgem step\n.macro step result:req\nadd $7,\\result\n.endm\n");
extern "C" long macro_source_late(long value) {
    asm("step %0" : "+r"(value) : : "cc");
    return value;
}
extern "C" long macro_source_instantiated(long value) {
    return macro_source_template(value)+MacroMember::early(value);
}
extern "C" long macro_source_inlined(long value) {
    return macro_source_early(value)+macro_source_late(value);
}

extern "C" int computed_dispatch(int opcode, int n) {
    static const void *table[] = { &&start, &&add, &&done };
    int value = 0;
    goto *table[opcode];
start:
    value = 3;
add:
    value += n;
    if (--n > 0) goto *table[1];
done:
    return value;
}
template<int N> int templated_dispatch(int opcode) {
    static const void *table[] = { &&first, &&second };
    goto *table[opcode];
first: return N;
second: return N + 7;
}
extern "C" int computed_templates(int opcode) {
    return templated_dispatch<3>(opcode) * 100 + templated_dispatch<11>(opcode);
}
extern "C" const void *computed_identity(int choose) {
    static const void *table[] = { &&first, &&second };
    return table[choose];
first: goto first_tail;
second: goto second_tail;
first_tail: return table[0];
second_tail: return table[1];
}

extern "C" unsigned host_mix(unsigned);
extern "C" int computed_empty_entries(unsigned choice) {
    static const void *entries[] = {&&first, &&second, &&last};
    goto *entries[choice];
first:
    asm volatile("");
    goto second;
second:
    asm volatile("");
    goto last;
last:
    return 71;
}
extern "C" unsigned computed_live_state(unsigned state, unsigned rounds) {
    static const void* entries[] = {&&even, &&odd};
    if (!rounds) return state;
    goto *entries[state & 1];
even:
    {
        unsigned local = state * 17 + 3;
        if (__builtin_expect((local & 16) != 0, 0)) local ^= 0x7351;
        else local += 29;
        state = host_mix(local);
    }
    if (--rounds) goto *entries[state & 1];
    return state;
odd:
    {
        unsigned local = state + 0x12345;
        if (local & 8) local *= 3;
        else local ^= 0x9537;
        state = host_mix(local);
    }
    if (--rounds) goto *entries[state & 1];
    return state;
}

struct DispatchState { unsigned value, remaining; };
extern "C" unsigned computed_aggregate_state(unsigned seed, unsigned rounds) {
    static const void *entries[] = {&&even, &&odd};
    DispatchState state = {seed, rounds};
    if (!state.remaining) return state.value;
    goto *entries[state.value & 1];
even:
    {
        DispatchState saved = state;
        state.value = host_mix(saved.value * 17 + saved.remaining);
    }
    if (--state.remaining) goto *entries[state.value & 1];
    return state.value;
odd:
    {
        DispatchState saved = state;
        state.value = host_mix((saved.value ^ 0x7351u) + saved.remaining);
    }
    if (--state.remaining) goto *entries[state.value & 1];
    return state.value;
}

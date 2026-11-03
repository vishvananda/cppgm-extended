// A join only needs values read before another definition. A handler that
// overwrites loop state differs from one that observes the last update.
volatile int observed;
__attribute__((noinline)) void maybe_throw(int i, int at) {
    if (i == at) throw i;
}
__attribute__((noinline)) int overwritten(int n, int at) {
    int state = 3;
    try {
        for (int i = 0; i < n; ++i) {
            state = state * 3 + i;
            if (i & 1) state += 7;
            maybe_throw(i, at);
        }
        return state;
    } catch (int value) {
        state = 17;
        observed = state;
        return state + value;
    }
}
__attribute__((noinline)) int visible(int n, int at) {
    int state = 3;
    try {
        for (int i = 0; i < n; ++i) {
            state = state * 3 + i;
            if (i & 1) state += 7;
            maybe_throw(i, at);
        }
    } catch (int value) {
        observed = state;
        return state + value;
    }
    return state;
}
int main() {
    for (int n = 0; n <= 8; ++n)
        for (int at = -1; at <= n; ++at) {
            int state = 3;
            for (int i = 0; i < n; ++i) {
                state = state * 3 + i;
                if (i & 1) state += 7;
                if (i == at) break;
            }
            const bool thrown = at >= 0 && at < n;
            observed = 0;
            if (overwritten(n, at) != (thrown ? 17 + at : state)) return 1;
            if (observed != (thrown ? 17 : 0)) return 2;
            observed = 0;
            if (visible(n, at) != state + (thrown ? at : 0)) return 3;
            if (observed != (thrown ? state : 0)) return 4;
        }
    return 0;
}

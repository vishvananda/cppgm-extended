// Discarded sizeof queries have no operand effects or volatile accesses.
int effects;
volatile long observed;
int effect() { ++effects; return 17; }
void discard_queries() {
    sizeof(effect());
    sizeof(++effects);
    sizeof(observed);
    sizeof(long[3]);
}
int main() {
    discard_queries();
    return effects != 0;
}

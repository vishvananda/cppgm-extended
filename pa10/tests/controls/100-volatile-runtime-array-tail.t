// N3485 8.5.1p7, 7.1.6.1: omitted volatile elements retain typed
// volatile stores, even when the emitted initialization loops. expect exit 0
int calls;
int next() { return ++calls; }
int main()
{
    volatile int values[33] = {next(), next()};
    return calls != 2 || values[0] != 1 || values[1] != 2 || values[32] != 0;
}

// GNU computed goto leaves a scope without invoking its destructors.
static int destroyed;
struct Guard { ~Guard() { ++destroyed; } };
extern "C" int computed_cleanup() {
    destroyed = 0;
    {
        Guard guard;
        goto *&&done;
    }
done:
    return destroyed;
}
extern "C" int direct_cleanup() {
    destroyed = 0;
    {
        Guard guard;
        goto done;
    }
done:
    return destroyed;
}

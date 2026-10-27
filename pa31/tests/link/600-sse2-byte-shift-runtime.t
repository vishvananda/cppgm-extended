// SSE2 whole-register byte shifts, both directions, all byte positions and
// oversized counts. Header wrappers use macros at O0 and inline functions
// when optimized; vector operands must be evaluated once even for zero output.

using Floats = float __attribute__((vector_size(16)));
auto value = __builtin_ia32_pminub128(Floats{}, Floats{});

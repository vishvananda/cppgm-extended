using Floats = float __attribute__((vector_size(16)));
int main() { __builtin_ia32_cvttps2dq(Floats{}, Floats{}); }

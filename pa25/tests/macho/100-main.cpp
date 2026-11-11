int helper();
int main(int argc, char** argv) {
  return argc > 0 && argv != nullptr && argv[0] != nullptr && helper() == 11 ? 0 : 3;
}
#if !defined(__APPLE__) || !defined(__MACH__) || defined(__linux__) || defined(__ELF__)
#error Mach-O compilations must expose only Darwin target macros
#endif

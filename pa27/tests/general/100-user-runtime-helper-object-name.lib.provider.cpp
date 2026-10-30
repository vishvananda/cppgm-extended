extern "C" void* cppgm_builtin_memcpy(void*, const void*, unsigned long);
extern "C" unsigned long cppgm_builtin_strlen(const char*) { return 117; }
extern "C" void* cppgm_builtin_memset(void* to, int, unsigned long) { return to; }
extern "C" int host_check()
{
  char to = 9;
  char from = 6;
  return cppgm_builtin_memcpy(&to, &from, 1) != &to || to != 9;
}

#include <csignal>

int call_raise()
{
  return std::raise(SIGTERM);
}

int main()
{
  return 0;
}

// GNU cold: a call of a cold function larger than its call stays a call, and
// so does a call on the path to a cold call, also where a small cold function
// goes inline; a cold function takes no copy of a callee larger than a call,
// while an ordinary caller inlines the same callee.
volatile int sink;
__attribute__((cold)) void report(int code, int value)
{
  sink = code;
  sink = value;
  sink = code * value;
  sink = code + value;
  sink = code - value;
  sink = code ^ value;
}
__attribute__((cold)) void note(int code) { sink = code; }
int helper(int x) { return x * 3 + (x >> 2) - (x & 7) + (x | 9) * 5; }
[[gnu::cold]] int slow_path(int x) { return helper(x) + helper(x + 1); }
int fast_path(int x) { return helper(x) + helper(x + 1); }
int checked_sum(const int* values, int count)
{
  int sum = 0;
  for (int i = 0; i < count; ++i)
  {
    if (values[i] < 0)
    {
      sink = helper(values[i]);
      report(i, values[i]);
    }
    sum += values[i];
  }
  return sum;
}
int noted(int value)
{
  if (value < 0)
  {
    sink = helper(value);
    note(value);
  }
  return value + 1;
}

int main()
{
  int values[4] = { 1, -2, 3, 4 };
  int sum = checked_sum(values, 4);
  return sum == 6 && slow_path(sum) == fast_path(sum) && noted(-1) == 0 ? 0 : 1;
}

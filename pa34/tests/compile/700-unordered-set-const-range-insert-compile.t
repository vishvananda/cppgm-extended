#include <string>
#include <unordered_set>

void add_names(std::unordered_set<std::string> & target,
               const std::unordered_set<std::string> & source)
{
  target.insert(source.begin(), source.end());
}

int main()
{
  std::unordered_set<std::string> target;
  std::unordered_set<std::string> source;
  add_names(target, source);
  return 0;
}

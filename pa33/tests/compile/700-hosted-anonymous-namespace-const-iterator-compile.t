#include <string>
#include <vector>

namespace parser_trace {
namespace {
struct Event {
  const char * category;
  std::string location;
  std::string message;
};
}

std::vector<Event> events_;

void note() {
  std::vector<Event>::const_iterator last = events_.begin();
  (void)last;
}
}

int main() {
  parser_trace::note();
  return 0;
}

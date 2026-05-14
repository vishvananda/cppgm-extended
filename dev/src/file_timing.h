#pragma once

#include <string>

namespace file_timing {

bool enabled();

bool startup_enabled();
void startup_mark(const char * label);

class ScopedTimer
{
public:
  ScopedTimer(const char * category, const std::string & location);
  ~ScopedTimer();

private:
  const char * category_;
  std::string location_;
  bool active_;
  long long start_us_;
};

}  // namespace file_timing

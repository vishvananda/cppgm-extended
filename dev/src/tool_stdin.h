#pragma once

#include <cstdio>
#include <streambuf>
#include <string>

inline std::string read_all_stdin()
{
  std::string bytes;
  char buffer[1 << 16];
  std::size_t size;
  while((size = std::fread(buffer, 1, sizeof(buffer), stdin)) != 0) {
    bytes.append(buffer, size);
  }
  return bytes;
}

class MemoryInputBuffer : public std::streambuf
{
public:
  explicit MemoryInputBuffer(const std::string & bytes)
  {
    if(bytes.empty()) {
      setg(nullptr, nullptr, nullptr);
      return;
    }
    char * begin = const_cast<char *>(bytes.data());
    setg(begin, begin, begin + bytes.size());
  }
};

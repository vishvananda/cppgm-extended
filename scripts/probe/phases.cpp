// One host-built caller separates the unmodified parser's two phases.
#include "simdjson.h"
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iostream>
int main(int argc, char** argv) {
  if (argc != 4 && argc != 5) return 2;
  int phase = std::strcmp(argv[2], "stage1") == 0 ? 1 : std::strcmp(argv[2], "stage2") == 0 ? 2 : std::strcmp(argv[2], "full") == 0 ? 0 : -1;
  if (phase < 0) return 2;
  char* end = nullptr;
  unsigned long count = std::strtoul(argv[3], &end, 10);
  if (!end || *end) return 2;
  simdjson::padded_string input;
  simdjson::dom::parser parser;
  simdjson::dom::element element;
  if (simdjson::padded_string::load(argv[1]).get(input) || parser.parse(input).get(element)) return 1;
  const auto wall_start = std::chrono::steady_clock::now();
  const auto cpu_start = std::clock();
  for (unsigned long i = 0; i < count; ++i) {
    simdjson::error_code error;
    if (phase == 1) error = parser.implementation->stage1(reinterpret_cast<const uint8_t*>(input.data()), input.size(), simdjson::stage1_mode::regular);
    else if (phase == 2) error = parser.implementation->stage2(parser.doc);
    else error = parser.parse(input).get(element);
    if (error) return 1;
  }
  const double cpu = double(std::clock() - cpu_start) / CLOCKS_PER_SEC;
  const double wall = std::chrono::duration<double>(std::chrono::steady_clock::now() - wall_start).count();
  if (phase == 1 && parser.implementation->stage2(parser.doc)) return 1;
  if (argc == 5) std::cout << parser.doc.root() << '\n';
  else std::printf("{\"phase\":%d,\"cpu\":%.17g,\"wall\":%.17g,\"iterations\":%lu,\"bytes\":%zu}\n", phase,cpu,wall,count,input.size());
}

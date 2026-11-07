// Fixed host-built driver: time the separately compiled library's DOM parser.
#include "simdjson.h"
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>

int main(int argc, char** argv) {
  if (argc != 4) return 2;
  simdjson::padded_string input;
  simdjson::dom::parser parser;
  simdjson::dom::element document;
  if (simdjson::padded_string::load(argv[1]).get(input) || parser.parse(input).get(document)) return 1;
  if (std::string(argv[2]) == "check") {
    std::cout << document << '\n';
    return 0;
  }
  char* end = nullptr;
  const unsigned long iterations = std::strtoul(argv[3], &end, 10);
  if (std::string(argv[2]) != "time" || !end || *end || !iterations) return 2;
  const std::clock_t cpu_start = std::clock();
  const auto start = std::chrono::steady_clock::now();
  for (unsigned long i = 0; i < iterations; ++i) {
    if (parser.parse(input).get(document)) return 1;
  }
  const double wall = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
  const double cpu = double(std::clock() - cpu_start) / CLOCKS_PER_SEC;
  std::printf("{\"wall\":%.17g,\"cpu\":%.17g,\"iterations\":%lu,\"bytes\":%zu}\n",
              wall, cpu, iterations, input.size());
}

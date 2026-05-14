// (C) 2013 CPPGM Foundation www.cppgm.org.  All rights reserved.

#include <cstdlib>
#include <cstdio>
#include <chrono>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

#include "cy86_compiler.h"
#include "cy86_internal.h"

namespace {

double elapsed_seconds(
    const chrono::steady_clock::time_point & start,
    const chrono::steady_clock::time_point & end)
{
  return chrono::duration_cast<chrono::duration<double> >(end - start).count();
}

}

int main(int argc, char ** argv)
{
  try
  {
    vector<string> args;
    for(int i = 1; i < argc; ++i) {
      args.emplace_back(argv[i]);
    }

    bool report_phases = false;
    if(!args.empty() && args[0] == "--phases") {
      report_phases = true;
      args.erase(args.begin());
    }

    if(args.size() < 3) {
      throw logic_error("invalid usage");
    }

    size_t repeat = stoull(args[0]);
    const string & output_target = args[1];
    vector<string> srcfiles(args.begin() + 2, args.end());
    const string outfile = "/tmp/cy86bench.out";
    const cy86_internal::NativeTarget native_target =
        cy86_internal::native_target_for_output(
            cy86_internal::parse_output_target(output_target));

    double total_tokenize = 0.0;
    double total_build = 0.0;
    double total_emit = 0.0;
    size_t total_tokens = 0;
    size_t total_statements = 0;

    for(size_t pass = 0; pass < repeat; ++pass) {
      for(size_t i = 0; i < srcfiles.size(); ++i) {
        chrono::steady_clock::time_point tokenize_start = chrono::steady_clock::now();
        vector<PostToken> tokens = tokenize_cy86_translation_unit(srcfiles[i]);
        total_tokens += tokens.size();
        chrono::steady_clock::time_point tokenize_end = chrono::steady_clock::now();

        chrono::steady_clock::time_point build_start = chrono::steady_clock::now();
        cy86_internal::Program program = cy86_internal::build_program_from_tokens(tokens);
        chrono::steady_clock::time_point build_end = chrono::steady_clock::now();

        chrono::steady_clock::time_point emit_start = chrono::steady_clock::now();
        cy86_internal::write_native_program(program, outfile, native_target);
        chrono::steady_clock::time_point emit_end = chrono::steady_clock::now();

        total_tokenize += elapsed_seconds(tokenize_start, tokenize_end);
        total_build += elapsed_seconds(build_start, build_end);
        total_emit += elapsed_seconds(emit_start, emit_end);
        total_statements += program.statements.size();
        remove(outfile.c_str());
      }
    }

    if(report_phases) {
      double total = total_tokenize + total_build + total_emit;
      cout << "passes " << repeat << '\n';
      cout << "token_count " << total_tokens << '\n';
      cout << "statement_count " << total_statements << '\n';
      cout << "tokenize_sec " << total_tokenize << '\n';
      cout << "build_sec " << total_build << '\n';
      cout << "emit_sec " << total_emit << '\n';
      cout << "total_sec " << total << '\n';
      if(total > 0.0) {
        cout << "tokenize_pct " << (100.0 * total_tokenize / total) << '\n';
        cout << "build_pct " << (100.0 * total_build / total) << '\n';
        cout << "emit_pct " << (100.0 * total_emit / total) << '\n';
      }
    }
  }
  catch(exception & e)
  {
    cerr << "ERROR: " << e.what() << '\n';
    return EXIT_FAILURE;
  }
}

// (C) 2013 CPPGM Foundation www.cppgm.org.  All rights reserved.

#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

#include "posttokenizer.h"
#include "preprocessor.h"

struct CountingPostTokenSink : IPostTokenOutputStream
{
  size_t token_count = 0;
  size_t byte_count = 0;

  void count_source(const std::string & source)
  {
    ++token_count;
    byte_count += source.size();
  }

  void emit_invalid(const std::string& source)
  {
    throw logic_error(string("Invalid token in sequence: ") + source);
  }

  void emit_simple(const std::string& source, ETokenType token_type)
  {
    count_source(source);
    byte_count += static_cast<size_t>(token_type);
  }

  void emit_identifier(const std::string& source)
  {
    count_source(source);
  }

  void emit_literal(const std::string& source, EFundamentalType type,
      const void* /*data*/, size_t nbytes)
  {
    count_source(source);
    byte_count += nbytes + static_cast<size_t>(type);
  }

  void emit_literal_array(const std::string& source, size_t num_elements,
      EFundamentalType type, const void* /*data*/, size_t nbytes)
  {
    count_source(source);
    byte_count += num_elements + nbytes + static_cast<size_t>(type);
  }

  void emit_user_defined_literal_character(const std::string& source,
      const std::string& ud_suffix, EFundamentalType type,
      const void* /*data*/, size_t nbytes)
  {
    count_source(source);
    byte_count += ud_suffix.size() + nbytes + static_cast<size_t>(type);
  }

  void emit_user_defined_literal_string_array(const std::string& source,
      const std::string& ud_suffix, size_t num_elements, EFundamentalType type,
      const void* /*data*/, size_t nbytes)
  {
    count_source(source);
    byte_count += ud_suffix.size() + num_elements + nbytes +
                  static_cast<size_t>(type);
  }

  void emit_user_defined_literal_integer(const std::string& source,
      const std::string& ud_suffix, const std::string& prefix)
  {
    count_source(source);
    byte_count += ud_suffix.size() + prefix.size();
  }

  void emit_user_defined_literal_floating(const std::string& source,
      const std::string& ud_suffix, const std::string& prefix)
  {
    count_source(source);
    byte_count += ud_suffix.size() + prefix.size();
  }

  void emit_eof()
  {
    ++token_count;
  }
};

struct BenchInput
{
  string file;
  string contents;
};

string read_file(const string & path)
{
  ifstream in(path);
  if(!in) {
    throw logic_error(string("Unable to read input file: ") + path);
  }
  return string((istreambuf_iterator<char>(in)), istreambuf_iterator<char>());
}

int main(int argc, char** argv)
{
  try
  {
    vector<string> args;
    for(int i = 1; i < argc; ++i)
      args.emplace_back(argv[i]);

    if(args.size() < 2)
      throw logic_error("invalid usage");

    size_t repeat = stoull(args[0]);
    vector<BenchInput> srcfiles;
    srcfiles.reserve(args.size() - 1);
    for(auto it = args.begin() + 1; it != args.end(); ++it) {
      srcfiles.push_back({*it, read_file(*it)});
    }
    time_t timestamp = time(nullptr);
    CountingPostTokenSink output;

    for(size_t pass = 0; pass < repeat; ++pass) {
      for(const auto & srcfile : srcfiles) {
        istringstream input(srcfile.contents);
        Preprocessor preprocessor(string(), timestamp);
        preprocessor.load(srcfile.file, input.rdbuf());
        PostTokenizer posttokenizer(preprocessor);
        stream_post_tokens(posttokenizer, output);
      }
    }

    volatile size_t sink = output.token_count + output.byte_count;
    (void)sink;
  }
  catch (exception& e)
  {
    cerr << "ERROR: " << e.what() << '\n';
    return EXIT_FAILURE;
  }
}

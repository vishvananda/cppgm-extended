#include "cli_batch_frontend.h"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

namespace {

vector<string> split_tab_fields(const string & line)
{
  vector<string> fields;
  size_t start = 0;
  while(true) {
    const size_t end = line.find('\t', start);
    if(end == string::npos) {
      fields.push_back(line.substr(start));
      return fields;
    }
    fields.push_back(line.substr(start, end - start));
    start = end + 1;
  }
}

map<string, string> parse_env_overrides(const string & text)
{
  map<string, string> out;
  size_t start = 0;
  while(start <= text.size()) {
    const size_t end = text.find(';', start);
    const string field =
        text.substr(start,
                    end == string::npos ? string::npos : end - start);
    if(!field.empty()) {
      const size_t eq = field.find('=');
      if(eq != string::npos && eq != 0) {
        out[field.substr(0, eq)] = field.substr(eq + 1);
      }
    }
    if(end == string::npos) {
      return out;
    }
    start = end + 1;
  }
  return out;
}

class ScopedEnvOverrides {
public:
  explicit ScopedEnvOverrides(const map<string, string> & overrides)
  {
    for(map<string, string>::const_iterator it = overrides.begin();
        it != overrides.end();
        ++it) {
      const char * old = getenv(it->first.c_str());
      old_defined_.push_back(make_pair(it->first, old != NULL));
      old_values_.push_back(make_pair(it->first, old != NULL ? string(old) : string()));
      setenv(it->first.c_str(), it->second.c_str(), 1);
    }
  }

  ~ScopedEnvOverrides()
  {
    for(size_t i = 0; i < old_defined_.size(); ++i) {
      const string & name = old_defined_[i].first;
      if(old_defined_[i].second) {
        setenv(name.c_str(), old_values_[i].second.c_str(), 1);
      } else {
        unsetenv(name.c_str());
      }
    }
  }

private:
  vector<pair<string, bool> > old_defined_;
  vector<pair<string, string> > old_values_;
};

int run_cli_invocation(const vector<string> & args,
                       const CliFrontendRunner & runner,
                       ostream & err)
{
  try
  {
    return runner(args);
  }
  catch(const exception & e)
  {
    err << "ERROR: " << e.what() << endl;
    return EXIT_FAILURE;
  }
}

vector<string> batch_base_args(int argc, char ** argv)
{
  vector<string> base_args;
  for(int i = 1; i < argc; ++i) {
    if(string(argv[i]) == "--batch-stdin") {
      continue;
    }
    base_args.push_back(argv[i]);
  }
  return base_args;
}

void write_batch_status(int status)
{
  if(status == EXIT_SUCCESS) {
    cout << "EXIT_SUCCESS" << endl;
  } else if(status == EXIT_FAILURE) {
    cout << "EXIT_FAILURE" << endl;
  } else {
    cout << status << endl;
  }
}

int run_batch_request(const vector<string> & base_args,
                      const vector<string> & fields,
                      const CliFrontendRunner & runner)
{
  if(fields.size() < 5) {
    return EXIT_FAILURE;
  }

  ofstream out(fields[0].c_str());
  ofstream err(fields[1].c_str());
  if(!out || !err) {
    return EXIT_FAILURE;
  }

  ScopedEnvOverrides env_overrides(parse_env_overrides(fields[3]));

  streambuf * old_out = cout.rdbuf(out.rdbuf());
  streambuf * old_err = cerr.rdbuf(err.rdbuf());

  vector<string> args = base_args;
  args.insert(args.end(), fields.begin() + 4, fields.end());
  const int status = run_cli_invocation(args, runner, cerr);

  cout.rdbuf(old_out);
  cerr.rdbuf(old_err);
  return status;
}

int run_batch_stdin(int argc,
                    char ** argv,
                    const CliFrontendRunner & runner)
{
  const vector<string> base_args = batch_base_args(argc, argv);
  string line;
  while(getline(cin, line)) {
    if(line.empty()) {
      continue;
    }
    write_batch_status(
        run_batch_request(base_args, split_tab_fields(line), runner));
  }
  return EXIT_SUCCESS;
}

}  // namespace

int run_cli_frontend(int argc,
                     char ** argv,
                     const CliFrontendRunner & runner)
{
  if(getenv("WRAPPED_BATCH_STDIN") != NULL) {
    for(int i = 1; i < argc; ++i) {
      if(string(argv[i]) == "--batch-stdin") {
        return run_batch_stdin(argc, argv, runner);
      }
    }
  }

  vector<string> args;
  for(int i = 1; i < argc; ++i) {
    args.push_back(argv[i]);
  }
  return run_cli_invocation(args, runner, cerr);
}

#include "cpp_driver_frontend.h"

#include "cli_batch_frontend.h"

#include <cstdlib>
#include <exception>
#include <fstream>
#include <iostream>
#include <map>
#include <cstdio>
#include <stdexcept>
#include <string>
#include <unistd.h>
#include <vector>

using namespace std;

#include "cpp_tool_cli.h"
#include "cpp_toolchain.h"
#include "file_timing.h"
#include "machine_linker.h"
#include "machine_object.h"
#include "preproc_output.h"

namespace {

class ScopedEnvironmentOverride
{
public:
  ScopedEnvironmentOverride(const string & name,
                            const string & value,
                            bool preserve_existing_when_empty = false)
      : name_(name),
        had_old_(false),
        active_(true)
  {
    const char * old = std::getenv(name_.c_str());
    if(old != nullptr) {
      had_old_ = true;
      old_ = old;
    }
    if(value.empty() && preserve_existing_when_empty) {
      active_ = false;
      return;
    }
    if(value.empty()) {
      unsetenv(name_.c_str());
    } else {
      setenv(name_.c_str(), value.c_str(), 1);
    }
  }

  ~ScopedEnvironmentOverride()
  {
    if(!active_) {
      return;
    }
    if(had_old_) {
      setenv(name_.c_str(), old_.c_str(), 1);
    } else {
      unsetenv(name_.c_str());
    }
  }

private:
  string name_;
  bool had_old_;
  bool active_;
  string old_;
};

CppPreprocessOptions preprocess_options_from_invocation(
    const CppToolInvocation & invocation)
{
  return invocation.preprocess_options;
}

struct TempObjectSet
{
  vector<string> files;
  string dir;
  bool cleanup_enabled = true;

  ~TempObjectSet()
  {
    if(!cleanup_enabled) {
      return;
    }
    for(size_t i = 0; i < files.size(); ++i) {
      std::remove(files[i].c_str());
    }
    if(!dir.empty()) {
      std::remove(dir.c_str());
    }
  }
};

bool file_exists(const string & path)
{
  ifstream in(path.c_str(), ios::binary);
  return static_cast<bool>(in);
}

string basename_only(const string & path)
{
  const string::size_type slash = path.find_last_of("/\\");
  if(slash == string::npos) {
    return path;
  }
  return path.substr(slash + 1);
}

string default_compile_output_path(const string & input)
{
  string base = basename_only(input);
  const string::size_type dot = base.find_last_of('.');
  if(dot != string::npos) {
    base.erase(dot);
  }
  return base + ".o";
}

string makefile_quote(const string & value)
{
  string out;
  for(size_t i = 0; i < value.size(); ++i) {
    const char ch = value[i];
    if(ch == ' ' || ch == '#' || ch == '$' || ch == ':' || ch == '\\') {
      out.push_back('\\');
    }
    out.push_back(ch);
  }
  return out;
}

void write_depfile_if_requested(const CppToolInvocation & invocation,
                                const string & default_target,
                                const vector<string> & dependencies)
{
  if(!invocation.generate_depfile && invocation.depfile.empty()) {
    return;
  }

  const string depfile =
      invocation.depfile.empty() ? default_target + ".d" : invocation.depfile;
  vector<string> targets = invocation.dep_targets;
  if(targets.empty()) {
    targets.push_back(default_target);
  }

  ofstream out(depfile.c_str());
  if(!out) {
    throw logic_error("unable to open depfile");
  }
  for(size_t i = 0; i < targets.size(); ++i) {
    if(i) {
      out << ' ';
    }
    out << makefile_quote(targets[i]);
  }
  out << ":";
  for(size_t i = 0; i < dependencies.size(); ++i) {
    out << " " << makefile_quote(dependencies[i]);
  }
  out << "\n";
  if(invocation.depfile_phony_targets) {
    for(size_t i = 0; i < dependencies.size(); ++i) {
      out << makefile_quote(dependencies[i]) << ":\n";
    }
  }
}

string resolve_library_input(const vector<string> & library_paths,
                             const string & library_name)
{
  for(size_t i = 0; i < library_paths.size(); ++i) {
    const string prefix = library_paths[i] + "/lib" + library_name;
    const string object_path = prefix + ".o";
    if(file_exists(object_path)) {
      return object_path;
    }
    const string text_object_path = prefix + ".obj";
    if(file_exists(text_object_path)) {
      return text_object_path;
    }
  }
  throw logic_error("unable to resolve library " + library_name);
}

string make_temp_object_dir()
{
  string pattern = "/tmp/cppgm-cpptoolchain-XXXXXX";
  vector<char> buffer(pattern.begin(), pattern.end());
  buffer.push_back('\0');
  char * path = mkdtemp(buffer.data());
  if(path == NULL) {
    throw logic_error("unable to create temporary object directory");
  }
  return path;
}

bool handle_query_only_invocation(const CppToolInvocation & invocation)
{
  if(!invocation.query_only()) {
    return false;
  }
  run_host_cpp_query(invocation.query_args);
  return true;
}

void write_compile_input_object_file(const CppToolInvocation & invocation,
                                     const string & input,
                                     const string & outfile,
                                     vector<string> * dependencies)
{
  if(path_looks_like_lowir_file(input)) {
    write_cpp_lowir_object_file(vector<string>(1, input),
                                outfile,
                                invocation.output_target,
                                invocation.optimization_level,
                                invocation.debug_info_level,
                                dependencies);
    return;
  }
  write_cpp_object_file(vector<string>(1, input),
                        preprocess_options_from_invocation(invocation),
                        outfile,
                        invocation.output_target,
                        invocation.optimization_level,
                        invocation.debug_info_level,
                        dependencies);
}

void compile_cpp_inputs(const CppToolInvocation & invocation)
{
  ScopedEnvironmentOverride stdlib_flags("CPPGM_STDLIB_FLAGS",
                                         invocation.stdlib_flag,
                                         true);
  for(size_t i = 0; i < invocation.inputs.size(); ++i) {
    const string outfile =
        invocation.inputs.size() == 1 ? invocation.outfile :
        default_compile_output_path(invocation.inputs[i]);
    vector<string> dependencies;
    write_compile_input_object_file(invocation,
                                    invocation.inputs[i],
                                    outfile,
                                    &dependencies);
    write_depfile_if_requested(invocation, outfile, dependencies);
  }
}

int run_cpptoolchain_frontend_impl(const vector<string> & args)
{
  const CppToolInvocation invocation = parse_cpp_tool_invocation(args);
  if(handle_query_only_invocation(invocation)) {
    return EXIT_SUCCESS;
  }
  ScopedEnvironmentOverride stdlib_flags("CPPGM_STDLIB_FLAGS",
                                         invocation.stdlib_flag,
                                         true);
  if(invocation.preprocess_only) {
    throw logic_error("cpptoolchain does not support -E");
  }
  if(invocation.compile_only) {
    compile_cpp_inputs(invocation);
    return EXIT_SUCCESS;
  }

  TempObjectSet temp_objects;
  temp_objects.dir = make_temp_object_dir();
  vector<string> object_inputs;
  for(size_t i = 0; i < invocation.inputs.size(); ++i) {
    const string & input = invocation.inputs[i];
    if(path_looks_like_object_file(input)) {
      object_inputs.push_back(input);
    } else {
      const string objfile = temp_objects.dir + "/input" + to_string(i) + ".o";
      write_compile_input_object_file(invocation, input, objfile, nullptr);
      temp_objects.files.push_back(objfile);
      object_inputs.push_back(objfile);
    }
  }
  for(size_t i = 0; i < invocation.libraries.size(); ++i) {
    const string path = resolve_library_input(invocation.library_paths,
                                              invocation.libraries[i]);
    object_inputs.push_back(path);
  }

  if(can_use_host_toolchain_for_output_target(invocation.output_target)) {
    const bool preserve_temp_objects =
        link_host_objects_to_native(object_inputs,
                                    invocation.outfile,
                                    invocation.output_target,
                                    invocation.debug_info_level,
                                    invocation.stdlib_flag);
    if(preserve_temp_objects) {
      temp_objects.cleanup_enabled = false;
    }
    return EXIT_SUCCESS;
  }

  link_exception_objects_to_native(object_inputs, invocation.outfile, string());
  return EXIT_SUCCESS;
}

int run_cpphostinterop_frontend_impl(const vector<string> & args)
{
  const CppToolInvocation invocation = parse_cpp_tool_invocation(args);
  if(handle_query_only_invocation(invocation)) {
    return EXIT_SUCCESS;
  }
  ScopedEnvironmentOverride stdlib_flags("CPPGM_STDLIB_FLAGS",
                                         invocation.stdlib_flag,
                                         true);
  if(invocation.preprocess_only) {
    throw logic_error("cpphostinterop does not support -E");
  }
  if(!invocation.libraries.empty() || !invocation.library_paths.empty()) {
    throw logic_error("cpphostinterop does not support -L/-l");
  }
  if(!invocation.compile_only) {
    throw logic_error("cpphostinterop requires -c");
  }
  compile_cpp_inputs(invocation);
  return EXIT_SUCCESS;
}

int run_cpphostcompat_frontend_impl(const vector<string> & args)
{
  file_timing::startup_mark("hostcompat.enter");
  const CppToolInvocation invocation = parse_cpp_tool_invocation(args);
  file_timing::startup_mark("hostcompat.invocation_parsed");
  if(handle_query_only_invocation(invocation)) {
    return EXIT_SUCCESS;
  }
  ScopedEnvironmentOverride stdlib_flags("CPPGM_STDLIB_FLAGS",
                                         invocation.stdlib_flag,
                                         true);
  file_timing::startup_mark("hostcompat.env_ready");
  if(!invocation.libraries.empty() || !invocation.library_paths.empty()) {
    throw logic_error("cpphostcompat does not support -L/-l");
  }
  if(invocation.preprocess_only) {
    if(invocation.explicit_outfile) {
      file_timing::startup_mark("hostcompat.preprocess_file_output_begin");
      write_preprocessed_posttoken_output_file(invocation.outfile,
                                               invocation.inputs,
                                               preprocess_options_from_invocation(invocation));
    } else {
      file_timing::startup_mark("hostcompat.preprocess_stdout_begin");
      write_preprocessed_posttoken_output(cout,
                                          invocation.inputs,
                                          preprocess_options_from_invocation(invocation));
    }
    file_timing::startup_mark("hostcompat.preprocess_done");
    return EXIT_SUCCESS;
  }
  if(!invocation.compile_only) {
    throw logic_error("cpphostcompat requires -E or -c");
  }
  compile_cpp_inputs(invocation);
  return EXIT_SUCCESS;
}

}  // namespace

int run_cpptoolchain_frontend(int argc, char ** argv)
{
  return run_cli_frontend(argc, argv, run_cpptoolchain_frontend_impl);
}

int run_cpphostinterop_frontend(int argc, char ** argv)
{
  return run_cli_frontend(argc, argv, run_cpphostinterop_frontend_impl);
}

int run_cpphostcompat_frontend(int argc, char ** argv)
{
  return run_cli_frontend(argc, argv, run_cpphostcompat_frontend_impl);
}

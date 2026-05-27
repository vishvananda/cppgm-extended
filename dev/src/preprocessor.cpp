#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <set>
#include <sstream>
#include <stdexcept>

using namespace std;

#include "cppgm_builtin_host_config.h"
#include "encoding.h"
#include "file_timing.h"
#include "types.h"
#include "preprocessor.h"

#define CPPGM_STRINGIZE_IMPL(x) #x
#define CPPGM_STRINGIZE(x) CPPGM_STRINGIZE_IMPL(x)

#ifndef CPPGM_DEFAULT_HOST_CXX
#define CPPGM_DEFAULT_HOST_CXX ""
#endif

// bootstrap system call interface, used by get_file_id
extern "C" long int syscall(long int n, ...) throw ();

// get_file_id returns true if file found at path `path`.
// out parameter `out_fileid` is set to file id
bool get_file_id(const string& path, FileId & out_file_id)
{
  int res;
#ifdef __APPLE__
  // macOS x86-64: SYS_stat64=338, struct stat layout: int dev, short mode,
  // short nlink, long long ino, ...
  struct { int dev; short mode; short nlink; long long ino; long long unused[20]; } data;
  res = syscall(338, path.c_str(), &data);
  out_file_id = make_pair((unsigned long)data.dev, (unsigned long)data.ino);
#else
  // Linux x86-64: SYS_stat=4, struct stat layout: ulong dev, ulong ino, ...
  struct { unsigned long int dev; unsigned long int ino; long int unused[16]; } data;
  res = syscall(4, path.c_str(), &data);
  out_file_id = make_pair(data.dev, data.ino);
#endif

  return res == 0;
}

vector<string> split_path_list(const string & text, char sep)
{
  vector<string> parts;
  string current;
  for(size_t i = 0; i < text.size(); ++i) {
    if(text[i] == sep) {
      if(!current.empty()) {
        parts.push_back(current);
      }
      current.clear();
    } else {
      current.push_back(text[i]);
    }
  }
  if(!current.empty()) {
    parts.push_back(current);
  }
  return parts;
}

string trim_whitespace(const string & text)
{
  size_t start = 0;
  while(start < text.size() &&
        (text[start] == ' ' || text[start] == '\t' ||
         text[start] == '\r' || text[start] == '\n')) {
    ++start;
  }
  size_t end = text.size();
  while(end > start &&
        (text[end - 1] == ' ' || text[end - 1] == '\t' ||
         text[end - 1] == '\r' || text[end - 1] == '\n')) {
    --end;
  }
  return text.substr(start, end - start);
}

bool is_identifier_like_preprocessing_operator(const string & text)
{
  switch(text.size()) {
  case 2:
    return text == "or";
  case 3:
    return text == "and" || text == "new" || text == "not" || text == "xor";
  case 5:
    return text == "bitor" || text == "compl" || text == "or_eq";
  case 6:
    return text == "and_eq" || text == "bitand" || text == "delete" ||
           text == "not_eq" || text == "xor_eq";
  default:
    return false;
  }
}

bool is_defined_operand_token(EPPTokenType type, const string & data)
{
  return type == PP_IDENTIFIER ||
         (type == PP_PREPROCESSING_OP &&
          is_identifier_like_preprocessing_operator(data));
}

vector<string> parse_compiler_search_paths(const string & output)
{
  vector<string> results;
  bool collecting = false;
  istringstream in(output);
  string line;
  while(getline(in, line)) {
    if(line.find("#include <...> search starts here:") != string::npos) {
      collecting = true;
      continue;
    }
    if(!collecting) {
      continue;
    }
    if(line.find("End of search list.") != string::npos) {
      break;
    }
    const string trimmed = trim_whitespace(line);
    if(trimmed.empty()) {
      continue;
    }
    if(trimmed.find("(framework directory)") != string::npos) {
      continue;
    }
    results.push_back(trimmed);
  }
  return results;
}

string run_command_capture_stdout(const string & command)
{
  string output;
  FILE * pipe = popen(command.c_str(), "r");
  if(pipe == nullptr) {
    return output;
  }
  char buffer[4096];
  while(fgets(buffer, sizeof(buffer), pipe) != nullptr) {
    output += buffer;
  }
  pclose(pipe);
  return output;
}

string env_string(const char * name)
{
  const char * value = getenv(name);
  return value != nullptr ? string(value) : string();
}

bool runtime_host_probe_requested()
{
  const string host_cxx = env_string("CPPGM_HOST_CXX");
  if(!host_cxx.empty() &&
     host_cxx != cppgm_builtin_host_config::kHostCxx) {
    return true;
  }

  const string stdlib_flags = env_string("CPPGM_STDLIB_FLAGS");
  if(!stdlib_flags.empty() &&
     stdlib_flags != cppgm_builtin_host_config::kStdlibFlags) {
    return true;
  }

  return false;
}

string runtime_probe_host_cxx()
{
  const string host_cxx = env_string("CPPGM_HOST_CXX");
  return host_cxx.empty() ? string(cppgm_builtin_host_config::kHostCxx) :
                            host_cxx;
}

string runtime_probe_stdlib_flags()
{
  const string stdlib_flags = env_string("CPPGM_STDLIB_FLAGS");
  return stdlib_flags.empty() ?
      string(cppgm_builtin_host_config::kStdlibFlags) : stdlib_flags;
}

vector<string> build_compiler_probe_commands(const string & probe_args)
{
  vector<string> commands;
  set<string> seen;
  if(!runtime_host_probe_requested()) {
    return commands;
  }

  string effective_probe_args = probe_args;
  const string stdlib_flags = runtime_probe_stdlib_flags();
  if(!stdlib_flags.empty()) {
    effective_probe_args = stdlib_flags + " " + effective_probe_args;
  }
  auto add_unique = [&](const string & command)
  {
    if(!command.empty() && seen.insert(command).second) {
      commands.push_back(command);
    }
  };
  add_unique(runtime_probe_host_cxx() + " " + effective_probe_args);
  return commands;
}

struct HostPredefinedMacro
{
  string name;
  vector<EPPToken> tokens;
};

bool should_import_host_predefined_macro(const string & name)
{
  if(name.compare(0, 21, "__GLIBCXX_TYPE_INT_N_") == 0 ||
     name.compare(0, 24, "__GLIBCXX_BITSIZE_INT_N_") == 0) {
    return true;
  }
  if(name.size() >= 7 &&
     name.compare(0, 2, "__") == 0 &&
     name.compare(name.size() - 7, 7, "_TYPE__") == 0) {
    return true;
  }
  if(name.size() >= 6 &&
     name.compare(0, 2, "__") == 0 &&
     name.compare(name.size() - 6, 6, "_MAX__") == 0) {
    return true;
  }
  if(name.size() >= 8 &&
     name.compare(0, 2, "__") == 0 &&
     name.compare(name.size() - 8, 8, "_WIDTH__") == 0) {
    return true;
  }
  if(name.compare(0, 6, "__FLT_") == 0 ||
     name.compare(0, 6, "__DBL_") == 0 ||
     name.compare(0, 7, "__LDBL_") == 0 ||
     name.compare(0, 9, "__SIZEOF_") == 0 ||
     name.compare(0, 9, "__ATOMIC_") == 0 ||
     name.compare(0, 15, "__CLANG_ATOMIC_") == 0 ||
     name.compare(0, 13, "__GCC_ATOMIC_") == 0) {
    return true;
  }
  return name == "__APPLE__" ||
         name == "__APPLE_CC__" ||
         name == "__APPLE_CPP__" ||
         name == "__MACH__" ||
         name == "__linux__" ||
         name == "__unix__" ||
         name == "__unix" ||
         name == "__ELF__" ||
         name == "_GNU_SOURCE" ||
         name == "_WIN32" ||
         name == "__llvm__" ||
         name == "__clang__" ||
         name == "__clang_major__" ||
         name == "__clang_minor__" ||
         name == "__clang_patchlevel__" ||
         name == "__clang_version__" ||
         name == "__GNUC__" ||
         name == "__GNUC_MINOR__" ||
         name == "__GNUC_PATCHLEVEL__" ||
         name == "__GNUG__" ||
         name == "__VERSION__" ||
         name == "__GXX_EXPERIMENTAL_CXX0X__" ||
         name == "__GXX_WEAK__" ||
         name == "__USER_LABEL_PREFIX__" ||
         name == "__EXCEPTIONS" ||
         name == "__cpp_exceptions" ||
         name == "__cpp_ref_qualifiers" ||
         name == "__cpp_rvalue_references" ||
         name == "__cpp_static_assert" ||
         name == "__GXX_RTTI" ||
         name == "__cpp_rtti" ||
         name == "__apple_build_version__" ||
         name == "__CHAR_BIT__";
}

bool parse_host_define_line(const string & line, string & name, string & value)
{
  if(line.compare(0, 8, "#define ") != 0) {
    return false;
  }
  size_t pos = 8;
  while(pos < line.size() && (line[pos] == ' ' || line[pos] == '\t')) {
    ++pos;
  }
  const size_t name_start = pos;
  while(pos < line.size() && line[pos] != ' ' && line[pos] != '\t') {
    ++pos;
  }
  if(pos == name_start) {
    return false;
  }
  name = line.substr(name_start, pos - name_start);
  if(name.find('(') != string::npos) {
    return false;
  }
  while(pos < line.size() && (line[pos] == ' ' || line[pos] == '\t')) {
    ++pos;
  }
  value = line.substr(pos);
  return true;
}

bool parse_host_macro_tokens(const string & value,
                             vector<EPPToken> & out)
{
  vector<EPPToken> tokens = tokenize(value);
  out.clear();
  for(size_t i = 0; i < tokens.size(); ++i) {
    if(tokens[i].type == PP_WHITESPACE ||
       tokens[i].type == PP_EOF ||
       tokens[i].type == PP_NEW_LINE) {
      continue;
    }
    if(tokens[i].type == PP_HEADER_NAME || tokens[i].type == PP_NON_WHITESPACE) {
      return false;
    }
    out.push_back(tokens[i]);
  }
  return true;
}

bool is_valid_macro_identifier(const string & name)
{
  if(name.empty()) {
    return false;
  }
  const unsigned char first = static_cast<unsigned char>(name[0]);
  if(!(first == '_' || std::isalpha(first))) {
    return false;
  }
  for(size_t i = 1; i < name.size(); ++i) {
    const unsigned char ch = static_cast<unsigned char>(name[i]);
    if(!(ch == '_' || std::isalnum(ch))) {
      return false;
    }
  }
  return true;
}

bool parse_command_line_macro_definition(const string & spec,
                                         string & name,
                                         vector<EPPToken> & out)
{
  const size_t eq = spec.find('=');
  name = eq == string::npos ? spec : spec.substr(0, eq);
  if(!is_valid_macro_identifier(name)) {
    return false;
  }

  const string value = eq == string::npos ? "1" : spec.substr(eq + 1);
  out.clear();
  if(value.empty()) {
    return true;
  }

  vector<EPPToken> tokens = tokenize(value);
  for(size_t i = 0; i < tokens.size(); ++i) {
    if(tokens[i].type == PP_WHITESPACE ||
       tokens[i].type == PP_EOF ||
       tokens[i].type == PP_NEW_LINE) {
      continue;
    }
    if(tokens[i].type == PP_HEADER_NAME || tokens[i].type == PP_NON_WHITESPACE) {
      return false;
    }
    out.push_back(tokens[i]);
  }
  return true;
}

void add_host_predefined_macro(vector<HostPredefinedMacro> & results,
                               set<string> & seen,
                               const string & name,
                               const vector<EPPToken> & tokens)
{
  if(seen.insert(name).second) {
    results.push_back(HostPredefinedMacro{name, tokens});
  }
}

void parse_host_predefined_macro_lines(const string & text,
                                       vector<HostPredefinedMacro> & results,
                                       set<string> & seen)
{
  istringstream in(text);
  string line;
  while(getline(in, line)) {
    string name;
    string value;
    if(!parse_host_define_line(line, name, value) ||
       !should_import_host_predefined_macro(name)) {
      continue;
    }
    vector<EPPToken> tokens;
    if(parse_host_macro_tokens(value, tokens)) {
      add_host_predefined_macro(results, seen, name, tokens);
    }
  }
}

vector<HostPredefinedMacro> build_host_predefined_macros()
{
  file_timing::startup_mark("host_predefined_macros.build_begin");
  vector<HostPredefinedMacro> results;
  set<string> seen;

  if(runtime_host_probe_requested()) {
    const vector<string> probes =
        build_compiler_probe_commands("-std=gnu++11 -dM -E -x c++ - < /dev/null 2>/dev/null");
    for(size_t i = 0; i < probes.size(); ++i) {
      file_timing::startup_mark("host_predefined_macros.probe_begin");
      parse_host_predefined_macro_lines(run_command_capture_stdout(probes[i]),
                                        results,
                                        seen);
      file_timing::startup_mark("host_predefined_macros.probe_done");
      if(!results.empty()) {
        break;
      }
    }
  } else {
    file_timing::startup_mark("host_predefined_macros.compiled_defaults_begin");
    parse_host_predefined_macro_lines(
        cppgm_builtin_host_config::kHostPredefinedMacros,
        results,
        seen);
    file_timing::startup_mark("host_predefined_macros.compiled_defaults_done");
  }

  const bool saw_clang_family =
      seen.count("__clang__") != 0 || seen.count("__llvm__") != 0;
  const bool saw_gnu_family =
      seen.count("__GNUC__") != 0 || seen.count("__GNUG__") != 0;

#ifdef __APPLE__
  add_host_predefined_macro(results, seen, "__APPLE__", vector<EPPToken>(1, {PP_INT_LITERAL, "1"}));
#endif
#ifdef __MACH__
  add_host_predefined_macro(results, seen, "__MACH__", vector<EPPToken>(1, {PP_INT_LITERAL, "1"}));
#endif
#ifdef __linux__
  add_host_predefined_macro(results, seen, "__linux__", vector<EPPToken>(1, {PP_INT_LITERAL, "1"}));
#endif
#ifdef __unix__
  add_host_predefined_macro(results, seen, "__unix__", vector<EPPToken>(1, {PP_INT_LITERAL, "1"}));
#endif
#ifdef __ELF__
  add_host_predefined_macro(results, seen, "__ELF__", vector<EPPToken>(1, {PP_INT_LITERAL, "1"}));
#endif
#ifdef _WIN32
  add_host_predefined_macro(results, seen, "_WIN32", vector<EPPToken>(1, {PP_INT_LITERAL, "1"}));
#endif
#ifdef __clang__
  if(saw_clang_family) {
    add_host_predefined_macro(results, seen, "__llvm__", vector<EPPToken>(1, {PP_INT_LITERAL, "1"}));
    add_host_predefined_macro(results, seen, "__clang__", vector<EPPToken>(1, {PP_INT_LITERAL, "1"}));
    add_host_predefined_macro(results, seen, "__clang_major__",
                              vector<EPPToken>(1, {PP_INT_LITERAL, CPPGM_STRINGIZE(__clang_major__)}));
    add_host_predefined_macro(results, seen, "__clang_minor__",
                              vector<EPPToken>(1, {PP_INT_LITERAL, CPPGM_STRINGIZE(__clang_minor__)}));
    add_host_predefined_macro(results, seen, "__clang_patchlevel__",
                              vector<EPPToken>(1, {PP_INT_LITERAL, CPPGM_STRINGIZE(__clang_patchlevel__)}));
    add_host_predefined_macro(results, seen, "__clang_version__",
                              vector<EPPToken>(1, {PP_QUOTE_LITERAL, "\"cppgm clang-compatible\""}));
  }
#endif
#ifdef __GNUC__
  if(saw_gnu_family) {
    add_host_predefined_macro(results, seen, "__GNUC__",
                              vector<EPPToken>(1, {PP_INT_LITERAL, CPPGM_STRINGIZE(__GNUC__)}));
    add_host_predefined_macro(results, seen, "__GNUC_MINOR__",
                              vector<EPPToken>(1, {PP_INT_LITERAL, CPPGM_STRINGIZE(__GNUC_MINOR__)}));
    add_host_predefined_macro(results, seen, "__GNUC_PATCHLEVEL__",
                              vector<EPPToken>(1, {PP_INT_LITERAL, CPPGM_STRINGIZE(__GNUC_PATCHLEVEL__)}));
  }
#endif
#ifdef __GNUG__
  if(saw_gnu_family) {
    add_host_predefined_macro(results, seen, "__GNUG__",
                              vector<EPPToken>(1, {PP_INT_LITERAL, CPPGM_STRINGIZE(__GNUG__)}));
  }
#endif
  if(saw_clang_family || saw_gnu_family) {
    add_host_predefined_macro(results, seen, "__VERSION__",
                              vector<EPPToken>(1, {PP_QUOTE_LITERAL, "\"cppgm hosted-compatible\""}));
  }
#ifdef __apple_build_version__
  add_host_predefined_macro(results, seen, "__apple_build_version__",
                            vector<EPPToken>(1, {PP_INT_LITERAL,
                                                 CPPGM_STRINGIZE(__apple_build_version__)}));
#endif

  file_timing::startup_mark("host_predefined_macros.build_done");
  return results;
}

vector<string> build_standard_include_paths()
{
  set<string> seen;
  vector<string> results;
  FileId file_id;
  const bool debug_stdinc = getenv("CPPGM_DEBUG_STDINC") != nullptr;

  const char * env = getenv("CPPGM_STDINC_PATHS");
  if(env != nullptr && *env != '\0') {
    const vector<string> env_paths = split_path_list(env, ':');
    for(size_t i = 0; i < env_paths.size(); ++i) {
      if(seen.insert(env_paths[i]).second) {
        results.push_back(env_paths[i]);
      }
    }
    return results;
  }

  if(!runtime_host_probe_requested()) {
    for(const char * const * path =
            cppgm_builtin_host_config::kStandardIncludePaths;
        *path != nullptr;
        ++path) {
      if(seen.insert(*path).second) {
        results.push_back(*path);
      }
    }
    if(!results.empty()) {
      return results;
    }
  }

  const vector<string> probes =
      build_compiler_probe_commands("-E -x c++ - -v < /dev/null 2>&1");
  for(size_t i = 0; i < probes.size(); ++i) {
    file_timing::startup_mark("stdinc.probe_begin");
    const string probe_output = run_command_capture_stdout(probes[i]);
    file_timing::startup_mark("stdinc.probe_done");
    const vector<string> probe_paths = parse_compiler_search_paths(probe_output);
    if(debug_stdinc) {
      cerr << "CPPGM stdinc probe: " << probes[i] << '\n';
      cerr << "CPPGM stdinc parsed paths:";
      for(size_t j = 0; j < probe_paths.size(); ++j) {
        cerr << '\n' << "  " << probe_paths[j];
      }
      cerr << '\n';
    }
    bool found_cpp_header_dir = false;
    for(size_t j = 0; j < probe_paths.size(); ++j) {
      if(!get_file_id(probe_paths[j], file_id)) {
        if(debug_stdinc) {
          cerr << "CPPGM stdinc skip missing path: " << probe_paths[j] << '\n';
        }
        continue;
      }
      if(probe_paths[j].find("include/c++") != string::npos ||
         probe_paths[j].find("include/c++/v1") != string::npos) {
        found_cpp_header_dir = true;
      }
      if(seen.insert(probe_paths[j]).second) {
        results.push_back(probe_paths[j]);
      }
    }
    if(found_cpp_header_dir) {
      if(debug_stdinc) {
        cerr << "CPPGM stdinc selected probe " << i << '\n';
      }
      return results;
    }
  }

  const char * fallback_paths[] = {
      "/usr/include/c++/4.7/",
      "/usr/include/c++/4.7/x86_64-linux-gnu/",
      "/usr/include/c++/4.7/backward/",
      "/usr/lib/gcc/x86_64-linux-gnu/4.7/include/",
      "/usr/local/include/",
      "/usr/lib/gcc/x86_64-linux-gnu/4.7/include-fixed/",
      "/usr/include/x86_64-linux-gnu/",
      "/usr/include/"};
  for(size_t i = 0; i < sizeof(fallback_paths) / sizeof(fallback_paths[0]); ++i) {
    const string path = fallback_paths[i];
    if(seen.insert(path).second) {
      results.push_back(path);
    }
  }
  return results;
}

const vector<string> & standard_include_paths()
{
  static const vector<string> * cached =
      new vector<string>(build_standard_include_paths());
  return *cached;
}

const vector<HostPredefinedMacro> & host_predefined_macros()
{
  file_timing::startup_mark("host_predefined_macros.enter");
  static const vector<HostPredefinedMacro> * cached =
      new vector<HostPredefinedMacro>(build_host_predefined_macros());
  file_timing::startup_mark("host_predefined_macros.ready");
  return *cached;
}

struct BuiltinTimeStrings
{
  string date;
  string time;
};

const BuiltinTimeStrings & get_builtin_time_strings(time_t now)
{
  static time_t cached_now = 0;
  static BuiltinTimeStrings cached;
  if(cached.date.empty() || cached_now != now) {
    tm now_tm = *localtime(&now);
    char date_buffer[16];
    char time_buffer[16];
    strftime(date_buffer, sizeof(date_buffer), "\"%b %e %Y\"", &now_tm);
    strftime(time_buffer, sizeof(time_buffer), "\"%H:%M:%S\"", &now_tm);
    cached_now = now;
    cached.date = date_buffer;
    cached.time = time_buffer;
  }
  return cached;
}

EPPToken file_macro(const ExpansionContext & context)
{
  string qt = "\"";
  return EPPToken{PP_QUOTE_LITERAL, qt + context.file + qt};
}

EPPToken line_macro(const ExpansionContext & context)
{
  return EPPToken{PP_INT_LITERAL, to_string(context.line)};
}

namespace {

string join_include_path(const string & dir, const string & name)
{
  if(dir.empty()) {
    return name;
  }
  if(dir[dir.size() - 1] == '/') {
    return dir + name;
  }
  return dir + "/" + name;
}

bool is_whitespace_token(const EPPToken & token)
{
  return token.type == PP_WHITESPACE || token.type == PP_NEW_LINE;
}

size_t skip_whitespace_tokens(const vector<EPPToken> & tokens, size_t i)
{
  while(i < tokens.size() && is_whitespace_token(tokens[i])) {
    ++i;
  }
  return i;
}

bool is_zero_builtin_query(const string & id)
{
  return id == "__has_cpp_attribute" ||
         id == "__building_module";
}

bool is_supported_attribute_query(const string & name)
{
  return name == "__using_if_exists__" ||
         name == "exclude_from_explicit_instantiation" ||
         name == "__exclude_from_explicit_instantiation__";
}

bool is_predefined_builtin_probe_name(const string & name)
{
  return name == "__has_feature" ||
         name == "__has_extension" ||
         name == "__has_attribute" ||
         name == "__has_cpp_attribute" ||
         name == "__has_builtin" ||
         name == "__building_module" ||
         name == "__has_include" ||
         name == "__has_include_next";
}

bool parse_builtin_name_tokens(const vector<EPPToken> & tokens,
                               string & name)
{
  const size_t begin = skip_whitespace_tokens(tokens, 0);
  if(begin >= tokens.size() || tokens[begin].type != PP_IDENTIFIER) {
    return false;
  }
  const size_t end = skip_whitespace_tokens(tokens, begin + 1);
  if(end != tokens.size()) {
    return false;
  }
  name = tokens[begin].data;
  return true;
}

bool is_supported_builtin_name(const string & name)
{
  return name == "__remove_cv" ||
         name == "__remove_const" ||
         name == "__remove_cvref" ||
         name == "__decay" ||
         name == "__remove_reference" ||
         name == "__remove_reference_t" ||
         name == "__integer_pack" ||
         name == "__is_pod" ||
         name == "__is_constructible" ||
         name == "__is_trivially_constructible" ||
         name == "__is_trivially_destructible" ||
         name == "__is_trivially_assignable" ||
         name == "__is_trivially_copyable" ||
         name == "__has_trivial_destructor" ||
         name == "__is_trivial" ||
         name == "__is_standard_layout" ||
         name == "__is_destructible" ||
         name == "__is_nothrow_destructible" ||
         name == "__is_integral" ||
         name == "__is_floating_point" ||
         name == "__is_arithmetic" ||
         name == "__is_signed" ||
         name == "__is_unsigned" ||
         name == "__is_reference" ||
         name == "__is_lvalue_reference" ||
         name == "__is_rvalue_reference" ||
         name == "__is_void" ||
         name == "__is_array" ||
         name == "__is_pointer" ||
         name == "__is_enum" ||
         name == "__is_union" ||
         name == "__is_class" ||
         name == "__is_fundamental" ||
         name == "__is_scalar" ||
         name == "__is_compound" ||
         name == "__is_object" ||
         name == "__reference_constructs_from_temporary" ||
         name == "__reference_binds_to_temporary" ||
         name == "__builtin_bswap16" ||
         name == "__builtin_bswap32" ||
         name == "__builtin_bswap64" ||
         name == "__builtin_clz" ||
         name == "__builtin_clzl" ||
         name == "__builtin_clzll" ||
         name == "__builtin_ctz" ||
         name == "__builtin_ctzl" ||
         name == "__builtin_ctzll" ||
         name == "__builtin_clzg" ||
         name == "__builtin_ctzg" ||
         name == "__builtin_popcount" ||
         name == "__builtin_popcountl" ||
         name == "__builtin_popcountll" ||
         name == "__builtin_popcountg" ||
         name == "__builtin_invoke" ||
         name == "__builtin_offsetof" ||
         name == "__builtin_expect" ||
         name == "__builtin_prefetch" ||
         name == "__builtin_assume_aligned" ||
         name == "__builtin_fabsf" ||
         name == "__builtin_fabs" ||
         name == "__builtin_fabsl" ||
         name == "__builtin_abs" ||
         name == "__builtin_labs" ||
         name == "__builtin_llabs" ||
         name == "__type_pack_element";
}

bool is_supported_type_trait_feature_query(const string & name)
{
  return name == "is_union" ||
         name == "is_enum" ||
         name == "is_pod" ||
         name == "is_empty" ||
         name == "is_constructible" ||
         name == "is_trivially_constructible" ||
         name == "is_trivially_assignable" ||
         name == "is_trivially_copyable" ||
         name == "is_standard_layout" ||
         name == "is_destructible" ||
         name == "is_trivial" ||
         name == "is_class" ||
         name == "is_final" ||
         name == "is_abstract" ||
         name == "is_base_of" ||
         name == "is_convertible" ||
         name == "is_polymorphic" ||
         name == "has_virtual_destructor" ||
         name == "has_trivial_destructor";
}

bool is_supported_feature_query(const string & id, const string & name)
{
  if(id == "__has_feature") {
    return is_supported_type_trait_feature_query(name) ||
           name == "cxx_atomic" ||
           name == "cxx_alias_templates" ||
           name == "cxx_alignas" ||
           name == "cxx_alignof" ||
           name == "cxx_auto_type" ||
           name == "__cxx_binary_literals__" ||
           name == "cxx_default_function_template_args" ||
           name == "cxx_defaulted_functions" ||
           name == "cxx_deleted_functions" ||
           name == "cxx_decltype" ||
           name == "cxx_decltype_incomplete_return_types" ||
           name == "cxx_exceptions" ||
           name == "cxx_explicit_conversions" ||
           name == "cxx_generalized_initializers" ||
           name == "cxx_inline_namespaces" ||
           name == "cxx_lambdas" ||
           name == "cxx_local_type_template_args" ||
           name == "cxx_noexcept" ||
           name == "cxx_nullptr" ||
           name == "cxx_override_control" ||
           name == "cxx_range_for" ||
           name == "cxx_raw_string_literals" ||
           name == "cxx_reference_qualified_functions" ||
           name == "cxx_rtti" ||
           name == "cxx_rvalue_references" ||
           name == "cxx_static_assert" ||
           name == "cxx_strong_enums" ||
           name == "cxx_trailing_return" ||
           name == "cxx_unicode_literals" ||
           name == "cxx_unrestricted_unions" ||
           name == "cxx_variadic_templates" ||
           name == "__cxx_variable_templates__";
  }
  if(id == "__has_extension") {
    return is_supported_type_trait_feature_query(name) ||
           name == "c_atomic" ||
           name == "cxx_alias_templates" ||
           name == "cxx_alignas" ||
           name == "cxx_alignof" ||
           name == "cxx_auto_type" ||
           name == "__cxx_binary_literals__" ||
           name == "cxx_default_function_template_args" ||
           name == "cxx_defaulted_functions" ||
           name == "cxx_deleted_functions" ||
           name == "cxx_decltype" ||
           name == "cxx_decltype_incomplete_return_types" ||
           name == "cxx_exceptions" ||
           name == "cxx_explicit_conversions" ||
           name == "cxx_generalized_initializers" ||
           name == "cxx_inline_namespaces" ||
           name == "cxx_lambdas" ||
           name == "cxx_local_type_template_args" ||
           name == "cxx_noexcept" ||
           name == "cxx_nullptr" ||
           name == "cxx_override_control" ||
           name == "cxx_range_for" ||
           name == "cxx_raw_string_literals" ||
           name == "cxx_reference_qualified_functions" ||
           name == "cxx_rtti" ||
           name == "cxx_rvalue_references" ||
           name == "cxx_static_assert" ||
           name == "cxx_strong_enums" ||
           name == "cxx_trailing_return" ||
           name == "cxx_unicode_literals" ||
           name == "cxx_unrestricted_unions" ||
           name == "cxx_variadic_templates" ||
           name == "__cxx_variable_templates__";
  }
  return false;
}

struct IncludeQuery
{
  string name;
  bool system = false;
};

bool decode_header_token(const EPPToken & token, IncludeQuery & out)
{
  if(token.type == PP_HEADER_NAME) {
    out.system = !token.data.empty() && token.data[0] == '<';
    out.name = token.data.substr(1, token.data.size() - 2);
    return true;
  }
  if(token.type == PP_QUOTE_LITERAL) {
    if(token.data.empty() || token.data[0] == 'R') {
      return false;
    }
    auto qdata = parse_quote_literal(token.data);
    if(qdata.quote != '"' || qdata.enc != '"' || !qdata.ud_suffix.empty()) {
      return false;
    }
    out.system = false;
    out.name = encode_utf8(qdata.contents);
    return true;
  }
  return false;
}

bool parse_header_query_tokens(const vector<EPPToken> & tokens,
                               IncludeQuery & out)
{
  const size_t begin = skip_whitespace_tokens(tokens, 0);
  if(begin >= tokens.size()) {
    return false;
  }
  const size_t end = skip_whitespace_tokens(tokens, begin + 1);
  if(end == tokens.size() && decode_header_token(tokens[begin], out)) {
    return true;
  }

  if(tokens[begin].type != PP_PREPROCESSING_OP || tokens[begin].data != "<") {
    return false;
  }

  string name;
  size_t i = begin + 1;
  for(; i < tokens.size(); ++i) {
    if(tokens[i].type == PP_PREPROCESSING_OP && tokens[i].data == ">") {
      ++i;
      break;
    }
    if(is_whitespace_token(tokens[i])) {
      continue;
    }
    name += tokens[i].data;
  }
  if(i == begin + 1 || name.empty()) {
    return false;
  }
  if(skip_whitespace_tokens(tokens, i) != tokens.size()) {
    return false;
  }
  out.system = true;
  out.name = name;
  return true;
}

bool parse_builtin_argument_tokens(const vector<EPPToken> & tokens,
                                   size_t start,
                                   vector<EPPToken> & argument,
                                   size_t & next)
{
  size_t i = skip_whitespace_tokens(tokens, start);
  if(i >= tokens.size() ||
     tokens[i].type != PP_PREPROCESSING_OP ||
     tokens[i].data != "(") {
    return false;
  }
  ++i;
  int depth = 0;
  argument.clear();
  for(; i < tokens.size(); ++i) {
    const EPPToken & token = tokens[i];
    if(token.type == PP_PREPROCESSING_OP) {
      if(token.data == "(") {
        ++depth;
      } else if(token.data == ")") {
        if(depth == 0) {
          next = i + 1;
          return true;
        }
        --depth;
      }
    }
    argument.push_back(token);
  }
  return false;
}

string dump_tokens(const vector<EPPToken> & tokens)
{
  string out;
  for(size_t i = 0; i < tokens.size(); ++i) {
    if(i && tokens[i].type != PP_WHITESPACE && !out.empty() &&
       out[out.size() - 1] != ' ') {
      out += ' ';
    }
    if(tokens[i].type == PP_WHITESPACE) {
      out += ' ';
    } else {
      out += tokens[i].data;
    }
  }
  return out;
}

}

Preprocessor::Preprocessor(const string & file,
                           time_t now,
                           const CppPreprocessOptions & options) :
  directive_state(DirectiveState::Start),
  defined_state(DefinedState::None),
  last_type(PP_NEW_LINE),
  include_paths(options.include_paths),
  system_include_paths(options.system_include_paths),
  dependency_files_(),
  dependency_file_set_(),
  emit_insignificant_whitespace(options.emit_insignificant_whitespace),
  cursor(this),
  raw_input(&cursor),
  pragma_op_nesting(0)
{
  macroizer.set_context_provider([this]() {
    return ExpansionContext{this->file(),
                            this->line(),
                            this->current_file_is_system_header()};
  });
  macroizer.macro_add("__CPPGM__", PP_INT_LITERAL, "201303L");
  macroizer.macro_add("__cplusplus", PP_INT_LITERAL, "201103L");
  macroizer.macro_add("__STDC_HOSTED__", PP_INT_LITERAL, "1");
  macroizer.macro_add("__ORDER_LITTLE_ENDIAN__", PP_INT_LITERAL, "1234");
  macroizer.macro_add("__ORDER_BIG_ENDIAN__", PP_INT_LITERAL, "4321");
  macroizer.macro_add("__BYTE_ORDER__", PP_INT_LITERAL, "1234");
  macroizer.macro_add("__LITTLE_ENDIAN__", PP_INT_LITERAL, "1");
  macroizer.macro_add("__x86_64__", PP_INT_LITERAL, "1");
  macroizer.macro_add("__LP64__", PP_INT_LITERAL, "1");
  file_timing::startup_mark("preprocessor.before_host_predefined_macros");
  const vector<HostPredefinedMacro> & predefined = host_predefined_macros();
  file_timing::startup_mark("preprocessor.after_host_predefined_macros");
  for(size_t i = 0; i < predefined.size(); ++i) {
    macroizer.macro_start(predefined[i].name);
    for(size_t j = 0; j < predefined[i].tokens.size(); ++j) {
      macroizer.macro_add_repl(predefined[i].tokens[j].type,
                               predefined[i].tokens[j].data);
    }
    macroizer.macro_finish();
  }
  file_timing::startup_mark("preprocessor.host_predefined_macros_installed");
  for(size_t i = 0; i < options.macro_definitions.size(); ++i) {
    string name;
    vector<EPPToken> tokens;
    if(!parse_command_line_macro_definition(options.macro_definitions[i], name, tokens)) {
      throw logic_error("invalid -D macro definition: " + options.macro_definitions[i]);
    }
    macroizer.macro_start(name);
    for(size_t j = 0; j < tokens.size(); ++j) {
      macroizer.macro_add_repl(tokens[j].type, tokens[j].data);
    }
    macroizer.macro_finish();
  }
  for(size_t i = 0; i < options.macro_undefinitions.size(); ++i) {
    macroizer.macro_remove(options.macro_undefinitions[i]);
  }
  macroizer.macro_add("__CPPGM_AUTHOR__",
                      PP_QUOTE_LITERAL,
                      "Vishvananda Ishaya");
  if(!now) {
    now = time(nullptr);
  }
  const auto & builtin_time_strings = get_builtin_time_strings(now);
  macroizer.macro_add("__DATE__",
                      PP_QUOTE_LITERAL,
                      builtin_time_strings.date);
  macroizer.macro_add("__TIME__",
                      PP_QUOTE_LITERAL,
                      builtin_time_strings.time);
  const auto add_function_like_builtin_macro =
      [this](const string & name, const vector<EPPToken> & replacement)
      {
        macroizer.macro_start(name);
        macroizer.macro_set_functional();
        for(size_t i = 0; i < replacement.size(); ++i) {
          macroizer.macro_add_repl(replacement[i].type, replacement[i].data);
        }
        macroizer.macro_finish();
      };
  add_function_like_builtin_macro(
      "__builtin_FILE",
      vector<EPPToken>(1, {PP_IDENTIFIER, "__FILE__"}));
  add_function_like_builtin_macro(
      "__builtin_LINE",
      vector<EPPToken>(1, {PP_IDENTIFIER, "__LINE__"}));
  add_function_like_builtin_macro(
      "__builtin_FUNCTION",
      vector<EPPToken>(1, {PP_QUOTE_LITERAL, "\"\""}));
  add_function_like_builtin_macro(
      "__builtin_COLUMN",
      vector<EPPToken>(1, {PP_INT_LITERAL, "0"}));
  // macroizer replaces these with the right data
  macroizer.macro_add("__FILE__", &file_macro);
  macroizer.macro_add("__LINE__", &line_macro);
  if(!file.empty()) {
    load(file);
    for(size_t i = options.forced_include_files.size(); i > 0; --i) {
      string resolved_path;
      FileId file_id;
      vector<string> search_dirs;
      size_t search_dir_index = 0;
      if(!resolve_include(options.forced_include_files[i - 1],
                          false,
                          false,
                          resolved_path,
                          file_id,
                          search_dirs,
                          search_dir_index)) {
        throw logic_error("Could not find forced include " +
                          options.forced_include_files[i - 1]);
      }
      const bool system_header =
          search_dir_index >= first_system_search_dir_index(search_dirs);
      note_dependency(resolved_path, system_header);
      files.emplace_back(new FileState(resolved_path, system_header));
      files.back()->include_search_dirs = search_dirs;
      files.back()->include_search_index = search_dir_index;
      files.back()->include_search_index_valid = true;
      start_new_file();
    }
  }
}

Preprocessor::TokenCursor::TokenCursor(Preprocessor * owner) :
  owner(owner),
  has_lookahead(false),
  lookahead(),
  line_start(true),
  token_line_start(true),
  token_file(),
  token_file_ptr(nullptr),
  token_line(0),
  token_column(0)
{}

void Preprocessor::TokenCursor::update_line_state(const EPPToken & token)
{
  token_line_start = line_start;
  if(token.type == PP_NEW_LINE) {
    line_start = true;
  } else if(token.type != PP_WHITESPACE) {
    line_start = false;
  }
}

EPPToken Preprocessor::TokenCursor::resume_injected()
{
  while(!owner->injections.empty()) {
    auto & injected = owner->injections.back();
    if(injected.tokens.empty()) {
      if(injected.has_resume_state) {
        owner->directive_state = injected.resume_state;
      }
      owner->injections.pop_back();
      continue;
    }
    auto token = injected.tokens.front();
    injected.tokens.pop_front();
    update_line_state(token);
    return token;
  }
  return EPPToken{PP_EOF};
}

EPPToken Preprocessor::TokenCursor::read_file()
{
  if(owner->files.empty()) {
    token_line_start = line_start;
    token_file.clear();
    token_file_ptr = nullptr;
    token_line = 0;
    token_column = 0;
    return EPPToken{PP_EOF};
  }
  FileState & file = *owner->files.back();
  auto token = file.tokenizer.get();
  token_file_ptr = &file.file;
  token_line = static_cast<uint32_t>(file.tokenizer.get_token_ln());
  token_column = static_cast<uint32_t>(file.tokenizer.get_token_ch());
  update_line_state(token);
  return token;
}

EPPToken Preprocessor::TokenCursor::get()
{
  if(has_lookahead) {
    auto item = lookahead;
    has_lookahead = false;
    token_line_start = item.line_start;
    token_file_ptr = item.file;
    if(token_file_ptr == nullptr) {
      token_file.clear();
    }
    token_line = item.line;
    token_column = item.column;
    return item.token;
  }
  auto injected = resume_injected();
  if(injected.type != PP_EOF || !owner->injections.empty()) {
    return injected;
  }
  return read_file();
}

EPPToken Preprocessor::TokenCursor::get_collapsed()
{
  auto token = get();
  if(token.type != PP_WHITESPACE && token.type != PP_NEW_LINE) {
    return token;
  }
  do {
    token = get();
  } while(token.type == PP_WHITESPACE || token.type == PP_NEW_LINE);
  if(token.type != PP_EOF) {
    has_lookahead = true;
    lookahead.token = token;
    lookahead.line_start = token_line_start;
    lookahead.file = token_file_ptr;
    lookahead.line = token_line;
    lookahead.column = token_column;
  }
  return EPPToken{PP_WHITESPACE};
}

bool Preprocessor::TokenCursor::at_line_start() const
{
  return token_line_start;
}

void Preprocessor::TokenCursor::clear_token_line_start()
{
  token_line_start = false;
}

void Preprocessor::TokenCursor::reset_line_state()
{
  line_start = true;
  token_line_start = true;
  token_file.clear();
  token_file_ptr = nullptr;
  token_line = 0;
  token_column = 0;
}

bool Preprocessor::TokenCursor::complete() const
{
  return owner->files.empty() && owner->injections.empty() && !has_lookahead;
}

EPPToken Preprocessor::RawInputSource::get()
{
  return cursor->get_collapsed();
}

EPPToken Preprocessor::get_input()
{
  return cursor.get();
}

EPPToken Preprocessor::get()
{
  if (complete()) {
    return EPPToken{PP_EOF};
  }
  EPPToken token;

  auto output = false;
  do {
    if(!injections.empty()) {
      token = cursor.get();
      output = process(token.type, token.data, false);
    } else if(macroizer.active()) {
      token = macroizer.get();
      if(token.type == PP_EOF) {
        continue;
      }
      output = process(token.type, token.data, false);
    } else {
      token = cursor.get();
      output = process(token.type, token.data, true);
    }
  } while (output == false);
  if (token.type == PP_NEW_LINE)
      token.type = PP_WHITESPACE;
  return token;
}

void Preprocessor::handle_hash_identifier(const string & data)
{
  auto & ifstate = ifstates.back();
  if(data == "define") {
    directive_state = DirectiveState::Define;
  } else if(data == "undef") {
    directive_state = DirectiveState::Undef;
  } else if(data == "pragma") {
    directive_state = DirectiveState::Pragma;
  } else if(data == "line") {
    begin_collected_directive(DirectiveState::Line);
  } else if(data == "include") {
    begin_collected_directive(DirectiveState::Include);
  } else if(data == "include_next") {
    begin_collected_directive(DirectiveState::IncludeNext);
  } else if(data == "if") {
    ifstates.push_back({ifstate.active, ifstate.active, IfState::If});
    defined_state = DefinedState::None;
    begin_collected_directive(DirectiveState::If);
  } else if(data == "ifdef") {
    ifstates.push_back({ifstate.active, ifstate.active, IfState::If});
    begin_collected_directive(DirectiveState::If);
    defined_state = DefinedState::NoParen;
  } else if(data == "ifndef") {
    ifstates.push_back({ifstate.active, ifstate.active, IfState::If});
    begin_collected_directive(DirectiveState::NotIf);
    defined_state = DefinedState::NoParen;
  } else if (data == "elif") {
    if(ifstate.state == IfState::Start)
      throw logic_error("Found #elif without matching #if");
    if(ifstate.state == IfState::Else)
      throw logic_error("Found #elif after #else");
    if(ifstate.active) {
      ifstate.state = IfState::NoElif;
      ifstate.active = false;
      directive_state = DirectiveState::Inactive;
    }
    if(ifstate.state != IfState::NoElif) {
      ifstate.active = true;
      defined_state = DefinedState::None;
      begin_collected_directive(DirectiveState::Elif);
    }
  } else if (data == "else") {
    if(ifstate.state == IfState::Start)
      throw logic_error("Found #else without matching #if");
    if(ifstate.state == IfState::Else)
      throw logic_error("Multiple #else statements in #if");
    const bool prior_branch_taken = (ifstate.state == IfState::NoElif);
    ifstate.state = IfState::Else;
    ifstate.active = prior_branch_taken ? false : !ifstate.active;
    directive_state = DirectiveState::Else;
  } else if (data == "endif") {
    if(ifstate.state == IfState::Start)
      throw logic_error("Found #endif without matching #if");
    ifstates.pop_back();
    if(ifstates.back().active) {
      directive_state = DirectiveState::EndIf;
    } else {
      directive_state = DirectiveState::Inactive;
    }
  } else if(data == "error") {
    error_msg.clear();
    directive_state = DirectiveState::Error;
  } else if(data == "warning") {
    warning_msg.clear();
    directive_state = DirectiveState::Warning;
  } else {
    throw logic_error(string("Unknown preprocessing directive at ") + file() +
                      ":" + to_string(line()) + ": " + data);
  }

  auto & finalstate = ifstates.back();
  if(!finalstate.active || !finalstate.parent_active) {
    finalstate.active = false;
    directive_state = DirectiveState::Inactive;
    defined_state = DefinedState::None;
  }
}

void Preprocessor::begin_collected_directive(DirectiveState state)
{
  directive_tokens.clear();
  directive_state = state;
}

void Preprocessor::append_directive_token(EPPTokenType type, const string & data)
{
  directive_tokens.emplace_back(EPPToken{type, data});
}

vector<EPPToken> Preprocessor::expand_directive_tokens()
{
  auto results = macroizer.expand(std::move(directive_tokens));
  directive_tokens.clear();
  return results;
}

vector<string> Preprocessor::build_include_search_dirs(bool quoted) const
{
  vector<string> search_dirs;
  if(quoted) {
    const string current_file = file();
    const size_t slash = current_file.rfind('/');
    if(slash < current_file.size()) {
      search_dirs.push_back(current_file.substr(0, slash + 1));
    }
  }
  search_dirs.insert(search_dirs.end(), include_paths.begin(), include_paths.end());
  search_dirs.push_back(string());
  search_dirs.insert(search_dirs.end(),
                     system_include_paths.begin(),
                     system_include_paths.end());
  const vector<string> & stdinc = standard_include_paths();
  search_dirs.insert(search_dirs.end(), stdinc.begin(), stdinc.end());
  return search_dirs;
}

size_t Preprocessor::first_system_search_dir_index(
    const vector<string> & search_dirs) const
{
  const size_t system_path_count =
      system_include_paths.size() + standard_include_paths().size();
  if(search_dirs.size() < system_path_count) {
    return search_dirs.size();
  }
  return search_dirs.size() - system_path_count;
}

bool Preprocessor::resolve_include(const string & name,
                                   bool system,
                                   bool include_next,
                                   string & resolved_path,
                                   FileId & resolved_id,
                                   vector<string> & search_dirs,
                                   size_t & search_dir_index) const
{
  if(include_next) {
    if(files.empty() || !files.back()->include_search_index_valid) {
      return false;
    }
    search_dirs = files.back()->include_search_dirs;
    for(size_t i = files.back()->include_search_index + 1; i < search_dirs.size(); ++i) {
      const string candidate = join_include_path(search_dirs[i], name);
      if(get_file_id(candidate, resolved_id)) {
        resolved_path = candidate;
        search_dir_index = i;
        return true;
      }
    }
    return false;
  }

  search_dirs = build_include_search_dirs(!system);
  for(size_t i = 0; i < search_dirs.size(); ++i) {
    const string candidate = join_include_path(search_dirs[i], name);
    if(get_file_id(candidate, resolved_id)) {
      resolved_path = candidate;
      search_dir_index = i;
      return true;
    }
  }
  return false;
}

void Preprocessor::finish_include_directive(bool include_next)
{
  auto results = expand_directive_tokens();
  IncludeQuery include_query;
  if(!parse_header_query_tokens(results, include_query)) {
    throw logic_error("Invalid token found in #include");
  }
  directive_state = DirectiveState::Start;
  FileId file_id;
  string resolved_path;
  vector<string> search_dirs;
  size_t search_dir_index = 0;
  if(!resolve_include(include_query.name,
                      include_query.system,
                      include_next,
                      resolved_path,
                      file_id,
                      search_dirs,
                      search_dir_index)) {
    ostringstream out;
    out << "Could not find " << include_query.name << " in #include"
        << " from " << file();
    if(include_next) {
      out << " (#include_next)";
    }
    out << " search_dirs=[";
    for(size_t i = 0; i < search_dirs.size(); ++i) {
      if(i) {
        out << ", ";
      }
      out << '"' << search_dirs[i] << '"';
    }
    out << "]";
    throw logic_error(out.str());
  }
  if(file_ids.count(file_id)) {
    return;
  }
  const bool system_header =
      search_dir_index >= first_system_search_dir_index(search_dirs);
  note_dependency(resolved_path, system_header);
  files.emplace_back(new FileState(resolved_path, system_header));
  files.back()->include_search_dirs = search_dirs;
  files.back()->include_search_index = search_dir_index;
  files.back()->include_search_index_valid = true;
  start_new_file();
}

void Preprocessor::finish_if_directive()
{
  vector<EPPToken> rewritten;
  try {
    auto results = expand_directive_tokens();
    for(size_t i = 0; i < results.size(); ++i) {
      const EPPToken & item = results[i];
      if(item.type != PP_IDENTIFIER) {
        rewritten.push_back(item);
        continue;
      }

      vector<EPPToken> argument;
      size_t next = 0;
      if((item.data == "__has_include" || item.data == "__has_include_next") &&
         parse_builtin_argument_tokens(results, i + 1, argument, next)) {
        IncludeQuery include_query;
        if(!parse_header_query_tokens(argument, include_query)) {
          throw logic_error("Invalid header operand in " + item.data);
        }
        FileId file_id;
        string resolved_path;
        vector<string> search_dirs;
        size_t search_dir_index = 0;
        const bool found = resolve_include(include_query.name,
                                           include_query.system,
                                           item.data == "__has_include_next",
                                           resolved_path,
                                           file_id,
                                           search_dirs,
                                           search_dir_index);
        rewritten.push_back(EPPToken{PP_INT_LITERAL, found ? "1" : "0"});
        i = next - 1;
        continue;
      }

      if(item.data == "__has_builtin" &&
         parse_builtin_argument_tokens(results, i + 1, argument, next)) {
        string builtin_name;
        const bool supported =
            parse_builtin_name_tokens(argument, builtin_name) &&
            is_supported_builtin_name(builtin_name);
        rewritten.push_back(EPPToken{PP_INT_LITERAL, supported ? "1" : "0"});
        i = next - 1;
        continue;
      }

      if((item.data == "__has_feature" || item.data == "__has_extension") &&
         parse_builtin_argument_tokens(results, i + 1, argument, next)) {
        string feature_name;
        const bool supported =
            parse_builtin_name_tokens(argument, feature_name) &&
            is_supported_feature_query(item.data, feature_name);
        rewritten.push_back(EPPToken{PP_INT_LITERAL, supported ? "1" : "0"});
        i = next - 1;
        continue;
      }

      if(item.data == "__has_attribute" &&
         parse_builtin_argument_tokens(results, i + 1, argument, next)) {
        string attribute_name;
        const bool supported =
            parse_builtin_name_tokens(argument, attribute_name) &&
            is_supported_attribute_query(attribute_name);
        rewritten.push_back(EPPToken{PP_INT_LITERAL, supported ? "1" : "0"});
        i = next - 1;
        continue;
      }

      if(is_zero_builtin_query(item.data) &&
         parse_builtin_argument_tokens(results, i + 1, argument, next)) {
        rewritten.push_back(EPPToken{PP_INT_LITERAL, "0"});
        i = next - 1;
        continue;
      }

      rewritten.push_back(item);
    }
    calculator = Calculator();
    for (auto & item : rewritten) {
      if(item.type != PP_WHITESPACE)
        calculator.accumulate(item.type, item.data);
    }
    auto active = calculator.calculate();
    auto & ifstate = ifstates.back();
    defined_state = DefinedState::None;
    if(directive_state == DirectiveState::NotIf) {
      active = !active;
    } else if(active && directive_state == DirectiveState::Elif) {
      ifstate.state = IfState::NoElif;
    }
    ifstate.active = active;
    if(active)
      directive_state = DirectiveState::Start;
    else
      directive_state = DirectiveState::Inactive;
  } catch (expr_error & e) {
    throw logic_error(string("Error in controlling expr at ") + file() + ":" +
                      to_string(line()) + " [" + dump_tokens(rewritten) +
                      "]: " + e.what());
  }
}

void Preprocessor::finish_line_directive()
{
  auto results = expand_directive_tokens();
  enum struct LineState {NeedInt, NeedStringOrEnd, NeedEnd};
  auto state = LineState::NeedInt;
  for(auto & item : results) {
    if(item.type == PP_WHITESPACE) {
      continue;
    }
    if(state == LineState::NeedInt) {
      if(item.type != PP_INT_LITERAL) {
        throw logic_error("Expected integer after #line");
      }
      unsigned long long result;
      string ud_suffix;
      try {
        classify_int(item.data, result, ud_suffix);
      } catch (logic_error & e) {
        throw logic_error("Int literal out of range in #line");
      }
      if(!ud_suffix.empty()) {
        throw logic_error("Invalid user defined int found in #line");
      }
      if(!files.empty()) {
        files.back()->tokenizer.set_ln(result);
      }
      state = LineState::NeedStringOrEnd;
      continue;
    }
    if(state == LineState::NeedStringOrEnd) {
      if(item.type != PP_QUOTE_LITERAL) {
        throw logic_error("Expected string after integer in #line");
      }
      if(item.data[0] == 'R') {
        throw logic_error("Invalid raw string in #line");
      }
      auto qdata = parse_quote_literal(item.data);
      if(qdata.quote != '"' || qdata.enc != '"' ||
         !qdata.ud_suffix.empty()) {
        throw logic_error("Invalid literal found in #line");
      }
      files.back()->file = encode_utf8(qdata.contents);
      state = LineState::NeedEnd;
      continue;
    }
    if(state == LineState::NeedEnd) {
      throw logic_error("Unexpected tokens after string in #line");
    }
  }
  if(state == LineState::NeedInt) {
    throw logic_error("Expected integer after #line");
  }
  directive_state = DirectiveState::Start;
}

void Preprocessor::handle_pragma_op_literal(const string & data,
                                           DirectiveState resume_state)
{
  auto qdata = parse_quote_literal(data);
  if(qdata.quote != '"' || qdata.enc != '"' ||
     !qdata.ud_suffix.empty()) {
    directive_state = resume_state;
    return;
  }
  try {
    auto results = tokenize(encode_utf8(qdata.contents));
    results.pop_back();

    DirectiveState pragma_state = DirectiveState::Pragma;
    results.emplace_back(EPPToken{PP_NEW_LINE, "\n"});
    for(size_t i = 0; i < results.size(); ++i) {
      const EPPTokenType type = results[i].type;
      const string & text = results[i].data;
      switch(pragma_state) {
      case DirectiveState::Start:
        break;
      case DirectiveState::Pragma:
        if(type == PP_IDENTIFIER) {
          if(text == "once") {
            FileId file_id;
            if(get_file_id(file(), file_id)) {
              file_ids.insert(file_id);
            } else {
              throw logic_error(string("Could not read file: ") + file());
            }
            pragma_state = DirectiveState::Once;
          } else {
            pragma_state = DirectiveState::Ignore;
          }
        } else if(type == PP_NEW_LINE) {
          pragma_state = DirectiveState::Start;
        } else if(type != PP_WHITESPACE) {
          pragma_state = DirectiveState::Ignore;
        }
        break;
      case DirectiveState::Once:
        if(type == PP_NEW_LINE) {
          pragma_state = DirectiveState::Start;
        } else if(type != PP_WHITESPACE) {
          throw logic_error("Illegal token after #pragma once");
        }
        break;
      case DirectiveState::Ignore:
        if(type == PP_NEW_LINE) {
          pragma_state = DirectiveState::Start;
        }
        break;
      default:
        throw logic_error("Illegal pragma operator state");
      }
    }
  } catch (logic_error& e) {
    throw logic_error(string("Invalid string in _Pragma: ") + e.what());
  }
  directive_state = resume_state;
}

void Preprocessor::finish_pragma_operator()
{
  vector<EPPToken> results = expand_directive_tokens();
  string literal;
  bool have_literal = false;
  for(size_t i = 0; i < results.size(); ++i) {
    if(results[i].type == PP_WHITESPACE) {
      continue;
    }
    if(!have_literal && results[i].type == PP_QUOTE_LITERAL) {
      literal = results[i].data;
      have_literal = true;
      continue;
    }
    throw logic_error(string("Expected single string literal in _Pragma after expansion [") +
                      dump_tokens(results) + "]");
  }
  if(!have_literal) {
    throw logic_error("Expected string after ( in _Pragma");
  }
  handle_pragma_op_literal(literal, DirectiveState::Start);
}

bool Preprocessor::process(const EPPTokenType type, const string & data,
                           bool allow_macro_start)
{
  bool output = false;
  if(directive_state != DirectiveState::DefineReplacement &&
     directive_state != DirectiveState::Inactive &&
     data == "__VA_ARGS__") {
    throw logic_error("Illegal use of __VA_ARGS__");
  }
  switch (directive_state) {
  case DirectiveState::Inactive:
    if(type == PP_PREPROCESSING_OP && cursor.at_line_start() &&
       (data == "#" || data == "%:")) {
      directive_state = DirectiveState::Hash;
    }
    break;
  case DirectiveState::Hash:
    if(!ifstates.empty() && !ifstates.back().active &&
       type == PP_IDENTIFIER &&
       data != "if" && data != "elif" && data != "ifdef" &&
       data != "ifndef" && data != "else" && data != "endif") {
      directive_state = DirectiveState::Inactive;
      break;
    }
    if(!ifstates.empty() && !ifstates.back().active &&
        type != PP_WHITESPACE && type != PP_IDENTIFIER) {
      directive_state = DirectiveState::Inactive;
      break;
    }
    if(type == PP_IDENTIFIER) {
      handle_hash_identifier(data);
    } else if(type == PP_NEW_LINE) {
      directive_state = DirectiveState::Start;
    } else if(type != PP_WHITESPACE) {
      throw logic_error("Invalid preprocessing directive");
    }
    break;
  case DirectiveState::Define:
    if(type == PP_IDENTIFIER) {
      macroizer.macro_start(data);
      directive_state = DirectiveState::DefineIdent;
    } else if(type != PP_WHITESPACE) {
      throw logic_error("Missing identifier for define");
    }
    break;
  case DirectiveState::DefineIdent:
    if(type == PP_NEW_LINE) {
      macroizer.macro_finish();
      directive_state = DirectiveState::Start;
    } else if (type == PP_PREPROCESSING_OP && data == "(") {
      macroizer.macro_set_functional();
      directive_state = DirectiveState::DefineParam;
    } else if (type == PP_WHITESPACE) {
      directive_state = DirectiveState::DefineReplacement;
    } else {
      throw logic_error("Invalid define directive");
    }
    break;
  case DirectiveState::DefineReplacement:
    if(type == PP_NEW_LINE) {
      macroizer.macro_finish();
      directive_state = DirectiveState::Start;
    } else {
      macroizer.macro_add_repl(type, data);
    }
    break;
  case DirectiveState::DefineParam:
    if(type == PP_IDENTIFIER) {
      macroizer.macro_add_param(data);
      directive_state = DirectiveState::DefineSep;
    } else if(type == PP_PREPROCESSING_OP && data == "...") {
      macroizer.macro_add_param("__VA_ARGS__");
      directive_state = DirectiveState::DefineVa;
    } else if(type == PP_PREPROCESSING_OP && data == ")") {
      directive_state = DirectiveState::DefineReplacement;
    } else if(type != PP_WHITESPACE) {
      throw logic_error("Invalid define directive");
    }
    break;
  case DirectiveState::DefineSep:
    if(type == PP_PREPROCESSING_OP) {
      if(data == ",") {
        directive_state = DirectiveState::DefineParam;
      } else if(data == ")") {
        directive_state = DirectiveState::DefineReplacement;
      }
    } else if(type != PP_WHITESPACE) {
      throw logic_error("Invalid define directive");
    }
    break;
  case DirectiveState::DefineVa:
    if(type == PP_PREPROCESSING_OP && data == ")") {
        directive_state = DirectiveState::DefineReplacement;
    } else if(type != PP_WHITESPACE) {
      throw logic_error("Unexpected data after ...");
    }
    break;
  case DirectiveState::Undef:
    if(type == PP_IDENTIFIER) {
      macroizer.macro_remove(data);
      directive_state = DirectiveState::UndefIdent;
    } else if(type != PP_WHITESPACE) {
      throw logic_error("Invalid undef directive");
    }
    break;
  case DirectiveState::UndefIdent:
    if(type == PP_NEW_LINE) {
      directive_state = DirectiveState::Start;
    } else if(type != PP_WHITESPACE) {
      throw logic_error("Invalid undef directive");
    }
    break;
  case DirectiveState::Include:
  case DirectiveState::IncludeNext:
    if(type == PP_NEW_LINE) {
      finish_include_directive(directive_state == DirectiveState::IncludeNext);
    } else {
      append_directive_token(type, data);
    }
    break;
  case DirectiveState::If:
  case DirectiveState::NotIf:
  case DirectiveState::Elif:
    if(type == PP_NEW_LINE) {
      finish_if_directive();
    } else if(type == PP_IDENTIFIER && data == "defined" &&
              defined_state != DefinedState::Start) {
      defined_state = DefinedState::Start;
    } else {
      switch(defined_state) {
      case DefinedState::None:
        append_directive_token(type, data);
        break;
      case DefinedState::Start:
        if(type == PP_PREPROCESSING_OP && data == "(") {
          defined_state = DefinedState::Paren;
          break;
        }
        // fallthrough
      case DefinedState::NoParen:
      case DefinedState::Paren:
        if(is_defined_operand_token(type, data)) {
          if(macroizer.macro_exists(data) ||
             is_predefined_builtin_probe_name(data))
            append_directive_token(PP_INT_LITERAL, "1");
          else
            append_directive_token(PP_INT_LITERAL, "0");
          if(defined_state == DefinedState::Paren)
            defined_state = DefinedState::End;
          else
            defined_state = DefinedState::None;
        } else if(type != PP_WHITESPACE) {
          throw logic_error("Expected identifier in defined expression.");
        }
        break;
      case DefinedState::End:
        if(type == PP_PREPROCESSING_OP && data == ")") {
          defined_state = DefinedState::None;
        } else if(type != PP_WHITESPACE) {
          throw logic_error("Expected end paren in defined expression.");
        }
        break;
      }
    }
    break;
  case DirectiveState::Pragma:
    if(type == PP_IDENTIFIER) {
      if(data == "once") {
        FileId file_id;
        if(get_file_id(file(), file_id)) {
          file_ids.insert(file_id);
        } else {
          throw logic_error(string("Could not read file: ") + file());
        }

        directive_state = DirectiveState::Once;
      } else if(data == "cppgm_mock_unknown") {
        directive_state = DirectiveState::Ignore;
      } else {
        directive_state = DirectiveState::Ignore;
      }
    } else if(type == PP_NEW_LINE) {
      directive_state = DirectiveState::Start;
    } else if(type != PP_WHITESPACE) {
      directive_state = DirectiveState::Ignore;
    }
    break;
  case DirectiveState::Once:
    if(type == PP_NEW_LINE) {
      directive_state = DirectiveState::Start;
    } else if(type != PP_WHITESPACE) {
      throw logic_error("Illegal token after #pragma once");
    }
    break;
  case DirectiveState::Line:
    if(type == PP_NEW_LINE) {
      finish_line_directive();
    } else {
      append_directive_token(type, data);
    }
    break;
  case DirectiveState::Ignore:
    if(type == PP_NEW_LINE) {
      directive_state = DirectiveState::Start;
    }
    break;
  case DirectiveState::Else:
    if(type == PP_NEW_LINE) {
      directive_state = ifstates.back().active ?
                        DirectiveState::Start :
                        DirectiveState::Inactive;
    } else if(type != PP_WHITESPACE) {
      throw logic_error("Illegal token after #else");
    }
    break;
  case DirectiveState::EndIf:
    if(type == PP_NEW_LINE) {
      directive_state = DirectiveState::Start;
    } else if(type != PP_WHITESPACE) {
      throw logic_error("Illegal token after #endif");
    }
    break;
  case DirectiveState::Error:
    if(type == PP_NEW_LINE) {
      throw logic_error("#error " + error_msg);
    } else if(type == PP_WHITESPACE) {
      error_msg += " ";
    } else {
      error_msg += data;
    }
    break;
  case DirectiveState::Warning:
    if(type == PP_NEW_LINE) {
      cerr << file() << ":" << line()
           << ": warning: #warning " << trim_whitespace(warning_msg) << '\n';
      directive_state = DirectiveState::Start;
    } else if(type == PP_WHITESPACE) {
      warning_msg += " ";
    } else {
      warning_msg += data;
    }
    break;
  case DirectiveState::PragmaOp:
    if (type == PP_PREPROCESSING_OP && data == "(") {
      directive_tokens.clear();
      pragma_op_nesting = 0;
      directive_state = DirectiveState::PragmaOpLit;
    } else if (type != PP_WHITESPACE && type != PP_NEW_LINE) {
      throw logic_error("Expected ( after _Pragma");
    }
    break;
  case DirectiveState::PragmaOpLit:
    if(type == PP_PREPROCESSING_OP && data == "(") {
      ++pragma_op_nesting;
      append_directive_token(type, data);
    } else if(type == PP_PREPROCESSING_OP && data == ")") {
      if(pragma_op_nesting == 0) {
        finish_pragma_operator();
      } else {
        --pragma_op_nesting;
        append_directive_token(type, data);
      }
    } else if(type == PP_EOF) {
      throw logic_error("Unterminated _Pragma");
    } else {
      append_directive_token(type, data);
    }
    break;
  case DirectiveState::PragmaOpEnd:
    if (type == PP_PREPROCESSING_OP && data == ")") {
      directive_state = DirectiveState::Start;
    } else if (type != PP_WHITESPACE && type != PP_NEW_LINE) {
      throw logic_error("Expected ) after string in _Pragma");
    }
    break;
  case DirectiveState::Start:
    if(type == PP_EOF) {
      break;
    }
    if(type == PP_PREPROCESSING_OP && cursor.at_line_start() &&
       (data == "#" || data == "%:")) {
      directive_state = DirectiveState::Hash;
      break;
    }
    if(type == PP_WHITESPACE || type == PP_NEW_LINE) {
      if(emit_insignificant_whitespace &&
         last_type != PP_WHITESPACE && last_type != PP_NEW_LINE) {
        output = true;
      }
    } else if(allow_macro_start &&
              type == PP_IDENTIFIER &&
              macroizer.macro_exists(data)) {
      macroizer.begin(raw_input, EPPToken{type, data});
      cursor.clear_token_line_start();
    } else if(type == PP_IDENTIFIER && data == "_Pragma") {
      directive_state = DirectiveState::PragmaOp;
    } else {
      output = true;
    }
  }
    if (type == PP_EOF) {
      if(ifstates.back().state != IfState::Start) {
        throw logic_error("File completed with unmatched #if");
      }
      ifstates.pop_back();
      files.pop_back();
      last_type = PP_NEW_LINE;
      cursor.reset_line_state();
      if (complete())
        output = true;
    } else if(last_type != PP_NEW_LINE || type != PP_WHITESPACE) {
      last_type = type;
    }
  return output;
}

void Preprocessor::load(streambuf * buf) {
  files.emplace_back(new FileState(buf));
  start_new_file();
}

void Preprocessor::load(const string & file, streambuf * buf) {
  files.emplace_back(new FileState(file, buf));
  note_dependency(file, false);
  start_new_file();
}

void Preprocessor::load(const string & file) {
  files.emplace_back(new FileState(file));
  note_dependency(file, false);
  start_new_file();
}

void Preprocessor::start_new_file() {
  if(files.size() == 200) {
    throw logic_error("Maximum include nexting reached");
  }
  last_type = PP_NEW_LINE;
  cursor.reset_line_state();
  ifstates.push_back({true, true, IfState::Start});
}

void Preprocessor::stream(IPPTokenStream & output) {
  try {
    stream_pp_tokens(*this, output);
  } catch (exception& e) {
    if(files.empty()) {
      throw;
    }
    auto tokenizer = &files.back()->tokenizer;
    throw logic_error(to_string(tokenizer->get_ln()) + ":" +
                      to_string(tokenizer->get_ch()) + ":" + e.what());
  }
}

void Preprocessor::note_dependency(const string & path, bool system)
{
  if(system || path.empty()) {
    return;
  }
  if(dependency_file_set_.insert(path).second) {
    dependency_files_.push_back(path);
  }
}

Preprocessor::FileState::FileState(const string & file, bool system_header) :
  file(file),
  include_search_dirs(),
  include_search_index(0),
  include_search_index_valid(false),
  system_header(system_header),
  stream(file),
  tokenizer(stream.rdbuf())
{}

Preprocessor::FileState::FileState(streambuf * buf) :
  file("fake"),
  include_search_dirs(),
  include_search_index(0),
  include_search_index_valid(false),
  system_header(false),
  stream(),
  tokenizer(buf)
{}

Preprocessor::FileState::FileState(const string & file, streambuf * buf) :
  file(file),
  include_search_dirs(),
  include_search_index(0),
  include_search_index_valid(false),
  system_header(false),
  stream(),
  tokenizer(buf)
{}

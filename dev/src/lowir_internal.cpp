#include "lowir_internal.h"

#include <algorithm>
#include <cerrno>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <limits>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

#include "symbol_linkage.h"

namespace lowir_internal {

namespace {

struct LineInfo
{
  string file;
  size_t line = 0;
  string text;
};

string trim(const string & input)
{
  size_t first = 0;
  while(first < input.size() &&
        isspace(static_cast<unsigned char>(input[first]))) {
    ++first;
  }
  size_t last = input.size();
  while(last > first &&
        isspace(static_cast<unsigned char>(input[last - 1]))) {
    --last;
  }
  return input.substr(first, last - first);
}

bool is_plain_identifier_text_impl(const string & text)
{
  if(text.empty()) {
    return false;
  }
  const unsigned char first = static_cast<unsigned char>(text[0]);
  if(!(isalpha(first) || first == '_')) {
    return false;
  }
  for(size_t i = 1; i < text.size(); ++i) {
    const unsigned char ch = static_cast<unsigned char>(text[i]);
    if(!(isalnum(ch) || ch == '_')) {
      return false;
    }
  }
  return true;
}

void append_loaded_line(vector<LineInfo> & lines,
                        const string & label,
                        size_t line_no,
                        const string & raw_line)
{
  string line = raw_line;
  size_t comment = line.find('#');
  if(comment != string::npos) {
    line.erase(comment);
  }
  const string cleaned = trim(line);
  if(cleaned.empty()) {
    return;
  }
  LineInfo info;
  info.file = label;
  info.line = line_no;
  info.text = cleaned;
  lines.push_back(info);
}

vector<LineInfo> load_lines_from_stream(istream & in, const string & label)
{
  vector<LineInfo> lines;
  string line;
  size_t line_no = 0;
  while(getline(in, line)) {
    ++line_no;
    append_loaded_line(lines, label, line_no, line);
  }
  return lines;
}

vector<LineInfo> load_lines(const vector<string> & srcfiles)
{
  vector<LineInfo> lines;
  for(size_t i = 0; i < srcfiles.size(); ++i) {
    ifstream in(srcfiles[i].c_str());
    if(!in) {
      throw ParseError("unable to open input file: " + srcfiles[i]);
    }
    vector<LineInfo> file_lines = load_lines_from_stream(in, srcfiles[i]);
    lines.insert(lines.end(), file_lines.begin(), file_lines.end());
  }
  return lines;
}

string describe_location(const LineInfo & line)
{
  ostringstream out;
  out << line.file << ":" << line.line;
  return out.str();
}

void fail(const LineInfo & line, const string & message)
{
  throw ParseError(describe_location(line) + ": " + message);
}

vector<string> lex_line(const string & line)
{
  vector<string> tokens;
  string current;
  for(size_t i = 0; i < line.size(); ++i) {
    const char ch = line[i];
    if(isspace(static_cast<unsigned char>(ch))) {
      if(!current.empty()) {
        tokens.push_back(current);
        current.clear();
      }
      continue;
    }
    if(ch == '-' && i + 1 < line.size() && line[i + 1] == '>') {
      if(!current.empty()) {
        tokens.push_back(current);
        current.clear();
      }
      tokens.push_back("->");
      ++i;
      continue;
    }
    if(ch == '(' || ch == ')' || ch == '{' || ch == '}' ||
       ch == ':' || ch == ',' || ch == '=' ||
       ch == '[' || ch == ']') {
      if(!current.empty()) {
        tokens.push_back(current);
        current.clear();
      }
      tokens.push_back(string(1, ch));
      continue;
    }
    current += ch;
  }
  if(!current.empty()) {
    tokens.push_back(current);
  }
  return tokens;
}

struct TokenStream
{
  const vector<string> & tokens;
  size_t pos = 0;

  explicit TokenStream(const vector<string> & in_tokens)
    : tokens(in_tokens)
  {}

  bool eof() const
  {
    return pos >= tokens.size();
  }

  const string & peek() const
  {
    if(eof()) {
      throw logic_error("unexpected token eof");
    }
    return tokens[pos];
  }

  bool consume(const string & token)
  {
    if(!eof() && tokens[pos] == token) {
      ++pos;
      return true;
    }
    return false;
  }

  string take()
  {
    const string & token = peek();
    ++pos;
    return token;
  }

  void expect(const LineInfo & line, const string & token)
  {
    if(!consume(token)) {
      fail(line, "expected '" + token + "'");
    }
  }
};

bool is_integer_token(const string & token);

bool is_valid_type(const string & text)
{
  if(text.size() > 6 &&
     text.compare(0, 4, "obj<") == 0 &&
     text[text.size() - 1] == '>') {
    const size_t x_pos = text.find('x', 4);
    if(x_pos != string::npos && x_pos + 1 < text.size() - 1) {
      const string size_text = text.substr(4, x_pos - 4);
      const string align_text = text.substr(x_pos + 1, text.size() - x_pos - 2);
      if(is_integer_token(size_text) && is_integer_token(align_text)) {
        const long long size = atoll(size_text.c_str());
        const long long align = atoll(align_text.c_str());
        if(size > 0 && align > 0 && (align & (align - 1)) == 0 && align <= 16) {
          return true;
        }
      }
    }
  }
  return text == "void" || text == "i1" || text == "i8" ||
         text == "u8" || text == "i16" || text == "u16" ||
         text == "i32" || text == "u32" || text == "i64" ||
         text == "i128" || text == "u128" ||
         text == "f32" || text == "f64" || text == "f80" ||
         text == "ptr";
}

bool parse_object_type_text(const string & text, size_t & size, size_t & alignment)
{
  if(text.size() <= 6 ||
     text.compare(0, 4, "obj<") != 0 ||
     text[text.size() - 1] != '>') {
    return false;
  }
  const size_t x_pos = text.find('x', 4);
  if(x_pos == string::npos || x_pos + 1 >= text.size() - 1) {
    return false;
  }
  const string size_text = text.substr(4, x_pos - 4);
  const string align_text = text.substr(x_pos + 1, text.size() - x_pos - 2);
  if(!is_integer_token(size_text) || !is_integer_token(align_text)) {
    return false;
  }
  const long long parsed_size = atoll(size_text.c_str());
  const long long parsed_alignment = atoll(align_text.c_str());
  if(parsed_size <= 0 || parsed_alignment <= 0 ||
     (parsed_alignment & (parsed_alignment - 1)) != 0 ||
     parsed_alignment > 16) {
    return false;
  }
  size = static_cast<size_t>(parsed_size);
  alignment = static_cast<size_t>(parsed_alignment);
  return true;
}

SymbolRole parse_symbol_role_text(const string & text)
{
  if(text == "entry") return SR_ENTRY;
  if(text == "init") return SR_INIT;
  if(text == "fini") return SR_FINI;
  if(text == "eh_top") return SR_EH_TOP;
  if(text == "eh_value") return SR_EH_VALUE;
  if(text == "eh_type") return SR_EH_TYPE;
  if(text == "eh_unhandled") return SR_EH_UNHANDLED;
  if(text == "eh_allocate_exception") return SR_EH_ALLOCATE_EXCEPTION;
  if(text == "eh_begin_catch") return SR_EH_BEGIN_CATCH;
  if(text == "eh_call_unexpected") return SR_EH_CALL_UNEXPECTED;
  if(text == "eh_current_exception_type") return SR_EH_CURRENT_EXCEPTION_TYPE;
  if(text == "eh_end_catch") return SR_EH_END_CATCH;
  if(text == "eh_rethrow") return SR_EH_RETHROW;
  if(text == "eh_throw") return SR_EH_THROW;
  if(text == "eh_personality") return SR_EH_PERSONALITY;
  if(text == "eh_resume") return SR_EH_RESUME;
  return SR_NONE;
}

LanguageLinkageMode parse_language_linkage_text(const string & text)
{
  if(text == "c") return LLM_C;
  if(text == "cpp") return LLM_CPP;
  return LLM_DEFAULT;
}

SymbolBindingMode parse_symbol_binding_text(const string & text)
{
  if(text == "internal") return SBM_INTERNAL;
  if(text == "strong") return SBM_STRONG;
  if(text == "weak") return SBM_WEAK;
  return SBM_DEFAULT;
}

bool parse_yes_no_text(const string & text, bool & value)
{
  if(text == "yes") {
    value = true;
    return true;
  }
  if(text == "no") {
    value = false;
    return true;
  }
  return false;
}

ir_model::SymbolLinkage ir_symbol_linkage_from_binding(SymbolBindingMode binding)
{
  switch(binding) {
    case SBM_INTERNAL:
      return ir_model::SL_INTERNAL;
    case SBM_WEAK:
      return ir_model::SL_WEAK;
    case SBM_STRONG:
    case SBM_DEFAULT:
      return ir_model::SL_EXTERNAL;
  }
  return ir_model::SL_EXTERNAL;
}

SymbolBindingMode binding_from_ir_symbol_linkage(ir_model::SymbolLinkage linkage)
{
  switch(linkage) {
    case ir_model::SL_INTERNAL:
      return SBM_INTERNAL;
    case ir_model::SL_WEAK:
      return SBM_WEAK;
    case ir_model::SL_EXTERNAL:
      return SBM_STRONG;
  }
  return SBM_DEFAULT;
}

ParamPassingMode parse_param_passing_mode_text(const string & text)
{
  if(text == "direct") return PPM_DIRECT;
  if(text == "indirect_result") return PPM_INDIRECT_RESULT;
  if(text == "by_address") return PPM_BY_ADDRESS;
  if(text == "reference") return PPM_REFERENCE;
  if(text == "decay") return PPM_DECAY;
  return PPM_DIRECT;
}

ParamCaptureMode parse_param_capture_mode_text(const string & text)
{
  if(text == "nocapture") return PCM_NOCAPTURE;
  if(text == "maycapture") return PCM_MAYCAPTURE;
  return PCM_DEFAULT;
}

ParamAccessMode parse_param_access_mode_text(const string & text)
{
  if(text == "none") return PAM_NONE;
  if(text == "read") return PAM_READ;
  if(text == "write") return PAM_WRITE;
  if(text == "readwrite") return PAM_READWRITE;
  return PAM_DEFAULT;
}

ParamAliasMode parse_param_alias_mode_text(const string & text)
{
  if(text == "noalias") return PALM_NOALIAS;
  return PALM_DEFAULT;
}

CallArityMode parse_call_arity_mode_text(const string & text)
{
  if(text == "fixed") return CAM_FIXED;
  if(text == "variadic") return CAM_VARIADIC;
  if(text == "prototype_relaxed") return CAM_PROTOTYPE_RELAXED;
  return CAM_FIXED;
}

CallEffectsMode parse_call_effects_mode_text(const string & text)
{
  if(text == "readnone") return CFXM_READNONE;
  if(text == "readonly") return CFXM_READONLY;
  if(text == "readwrite") return CFXM_READWRITE;
  return CFXM_DEFAULT;
}

CallUnwindMode parse_call_unwind_mode_text(const string & text)
{
  if(text == "may") return CUM_MAY;
  if(text == "no") return CUM_NO;
  return CUM_DEFAULT;
}

CallReturnMode parse_call_return_mode_text(const string & text)
{
  if(text == "returns") return CRM_RETURNS;
  if(text == "noreturn") return CRM_NORETURN;
  return CRM_DEFAULT;
}

GlobalStorageMode parse_global_storage_text(const string & text)
{
  if(text == "writable") return GSM_WRITABLE;
  if(text == "readonly") return GSM_READONLY;
  if(text == "thread_local") return GSM_THREAD_LOCAL;
  return GSM_DEFAULT;
}

IndexProjectionKind parse_index_projection_text(const string & text)
{
  if(text == "array_element") return IPK_ARRAY_ELEMENT;
  if(text == "field") return IPK_FIELD;
  if(text == "base_subobject") return IPK_BASE_SUBOBJECT;
  if(text == "reference_field") return IPK_REFERENCE_FIELD;
  return IPK_NONE;
}

void parse_index_metadata(IndexProjectionKind & projection,
                          TokenStream & stream,
                          const LineInfo & line)
{
  if(!stream.consume("[")) {
    return;
  }
  bool saw_projection = false;
  for(;;) {
    if(stream.eof()) {
      fail(line, "unterminated index metadata");
    }
    const string key = stream.take();
    stream.expect(line, "=");
    if(stream.eof()) {
      fail(line, "expected metadata value");
    }
    const string value = stream.take();
    if(key == "projection") {
      if(saw_projection) {
        fail(line, "duplicate projection metadata");
      }
      projection = parse_index_projection_text(value);
      if(projection == IPK_NONE) {
        fail(line, "unknown index projection '" + value + "'");
      }
      saw_projection = true;
    } else {
      fail(line, "unknown index metadata key '" + key + "'");
    }
    if(stream.consume("]")) {
      break;
    }
    stream.expect(line, ",");
  }
}

void parse_global_metadata(GlobalStorageMode & storage,
                           SymbolMetadata & metadata,
                           TokenStream & stream,
                           const LineInfo & line)
{
  if(!stream.consume("[")) {
    return;
  }
  bool saw_storage = storage != GSM_DEFAULT;
  bool saw_role = false;
  bool saw_linkage = false;
  bool saw_binding = false;
  bool saw_object = false;
  bool saw_keep_alias = false;
  bool saw_prefer_local = false;
  bool saw_object_root = false;
  bool saw_section_segment = false;
  bool saw_section_name = false;
  for(;;) {
    if(stream.eof()) {
      fail(line, "unterminated global metadata");
    }
    const string key = stream.take();
    stream.expect(line, "=");
    if(stream.eof()) {
      fail(line, "expected metadata value");
    }
    const string value = stream.take();
    if(key == "storage") {
      if(saw_storage) {
        fail(line, "duplicate storage metadata");
      }
      storage = parse_global_storage_text(value);
      if(storage == GSM_DEFAULT) {
        fail(line, "unknown global storage '" + value + "'");
      }
      saw_storage = true;
    } else if(key == "role") {
      if(saw_role) {
        fail(line, "duplicate role metadata");
      }
      metadata.role = parse_symbol_role_text(value);
      if(metadata.role == SR_NONE) {
        fail(line, "unknown symbol role '" + value + "'");
      }
      saw_role = true;
    } else if(key == "linkage") {
      if(saw_linkage) {
        fail(line, "duplicate linkage metadata");
      }
      metadata.linkage = parse_language_linkage_text(value);
      if(metadata.linkage == LLM_DEFAULT) {
        fail(line, "unknown language linkage '" + value + "'");
      }
      saw_linkage = true;
    } else if(key == "binding") {
      if(saw_binding) {
        fail(line, "duplicate binding metadata");
      }
      metadata.binding = parse_symbol_binding_text(value);
      if(metadata.binding == SBM_DEFAULT) {
        fail(line, "unknown symbol binding '" + value + "'");
      }
      saw_binding = true;
    } else if(key == "object") {
      if(saw_object) {
        fail(line, "duplicate object metadata");
      }
      metadata.object_symbol = value;
      if(metadata.object_symbol.empty()) {
        fail(line, "object metadata requires non-empty symbol name");
      }
      saw_object = true;
    } else if(key == "keep_alias") {
      if(saw_keep_alias) {
        fail(line, "duplicate keep_alias metadata");
      }
      if(!parse_yes_no_text(value, metadata.keep_internal_alias)) {
        fail(line, "unknown keep_alias mode '" + value + "'");
      }
      saw_keep_alias = true;
    } else if(key == "prefer_local") {
      if(saw_prefer_local) {
        fail(line, "duplicate prefer_local metadata");
      }
      if(!parse_yes_no_text(value, metadata.prefer_local_object_binding)) {
        fail(line, "unknown prefer_local mode '" + value + "'");
      }
      saw_prefer_local = true;
    } else if(key == "object_root") {
      if(saw_object_root) {
        fail(line, "duplicate object_root metadata");
      }
      if(!parse_yes_no_text(value, metadata.object_output_root)) {
        fail(line, "unknown object_root mode '" + value + "'");
      }
      saw_object_root = true;
    } else if(key == "section_segment") {
      if(saw_section_segment) {
        fail(line, "duplicate section_segment metadata");
      }
      metadata.section_segment = value;
      saw_section_segment = true;
    } else if(key == "section_name") {
      if(saw_section_name) {
        fail(line, "duplicate section_name metadata");
      }
      metadata.section_name = value;
      if(metadata.section_name.empty()) {
        fail(line, "section_name metadata requires non-empty name");
      }
      saw_section_name = true;
    } else {
      fail(line, "unknown global metadata key '" + key + "'");
    }
    if(stream.consume("]")) {
      break;
    }
    stream.expect(line, ",");
  }
}

void parse_function_metadata(FunctionBoundaryMetadata & boundary,
                             SymbolMetadata & symbol,
                             TokenStream & stream,
                             const LineInfo & line)
{
  bool saw_arity = false;
  bool saw_effects = false;
  bool saw_unwind = false;
  bool saw_return = false;
  bool saw_role = false;
  bool saw_linkage = false;
  bool saw_binding = false;
  bool saw_object = false;
  bool saw_tls_for = false;
  bool saw_keep_alias = false;
  bool saw_prefer_local = false;
  bool saw_object_root = false;
  bool saw_trivial_lifecycle = false;
  bool saw_force_inline = false;
  while(stream.consume("[")) {
    for(;;) {
      if(stream.eof()) {
        fail(line, "unterminated function metadata");
      }
      const string key = stream.take();
      stream.expect(line, "=");
      if(stream.eof()) {
        fail(line, "expected metadata value");
      }
      const string value = stream.take();
      if(key == "arity") {
        if(saw_arity) {
          fail(line, "duplicate arity metadata");
        }
        boundary.arity = parse_call_arity_mode_text(value);
        if(value != "fixed" && boundary.arity == CAM_FIXED) {
          fail(line, "unknown function arity mode '" + value + "'");
        }
        saw_arity = true;
      } else if(key == "effects") {
        if(saw_effects) {
          fail(line, "duplicate effects metadata");
        }
        boundary.effects = parse_call_effects_mode_text(value);
        if(boundary.effects == CFXM_DEFAULT) {
          fail(line, "unknown function effects mode '" + value + "'");
        }
        saw_effects = true;
      } else if(key == "unwind") {
        if(saw_unwind) {
          fail(line, "duplicate unwind metadata");
        }
        boundary.unwind = parse_call_unwind_mode_text(value);
        if(boundary.unwind == CUM_DEFAULT) {
          fail(line, "unknown function unwind mode '" + value + "'");
        }
        saw_unwind = true;
      } else if(key == "return") {
        if(saw_return) {
          fail(line, "duplicate return metadata");
        }
        boundary.returns = parse_call_return_mode_text(value);
        if(boundary.returns == CRM_DEFAULT) {
          fail(line, "unknown function return mode '" + value + "'");
        }
        saw_return = true;
      } else if(key == "role") {
        if(saw_role) {
          fail(line, "duplicate role metadata");
        }
        symbol.role = parse_symbol_role_text(value);
        if(symbol.role == SR_NONE) {
          fail(line, "unknown symbol role '" + value + "'");
        }
        saw_role = true;
      } else if(key == "linkage") {
        if(saw_linkage) {
          fail(line, "duplicate linkage metadata");
        }
        symbol.linkage = parse_language_linkage_text(value);
        if(symbol.linkage == LLM_DEFAULT) {
          fail(line, "unknown language linkage '" + value + "'");
        }
        saw_linkage = true;
      } else if(key == "binding") {
        if(saw_binding) {
          fail(line, "duplicate binding metadata");
        }
        symbol.binding = parse_symbol_binding_text(value);
        if(symbol.binding == SBM_DEFAULT) {
          fail(line, "unknown symbol binding '" + value + "'");
        }
        saw_binding = true;
      } else if(key == "object") {
        if(saw_object) {
          fail(line, "duplicate object metadata");
        }
        symbol.object_symbol = value;
        if(symbol.object_symbol.empty()) {
          fail(line, "object metadata requires non-empty symbol name");
        }
        saw_object = true;
      } else if(key == "tls_for") {
        if(saw_tls_for) {
          fail(line, "duplicate tls_for metadata");
        }
        symbol.tls_for_symbol = value;
        if(symbol.tls_for_symbol.empty() || symbol.tls_for_symbol[0] != '@') {
          fail(line, "tls_for metadata requires a LowIR global symbol name");
        }
        saw_tls_for = true;
      } else if(key == "keep_alias") {
        if(saw_keep_alias) {
          fail(line, "duplicate keep_alias metadata");
        }
        if(!parse_yes_no_text(value, symbol.keep_internal_alias)) {
          fail(line, "unknown keep_alias mode '" + value + "'");
        }
        saw_keep_alias = true;
      } else if(key == "prefer_local") {
        if(saw_prefer_local) {
          fail(line, "duplicate prefer_local metadata");
        }
        if(!parse_yes_no_text(value, symbol.prefer_local_object_binding)) {
          fail(line, "unknown prefer_local mode '" + value + "'");
        }
        saw_prefer_local = true;
      } else if(key == "object_root") {
        if(saw_object_root) {
          fail(line, "duplicate object_root metadata");
        }
        if(!parse_yes_no_text(value, symbol.object_output_root)) {
          fail(line, "unknown object_root mode '" + value + "'");
        }
        saw_object_root = true;
      } else if(key == "trivial_lifecycle") {
        if(saw_trivial_lifecycle) {
          fail(line, "duplicate trivial_lifecycle metadata");
        }
        if(!parse_yes_no_text(value, symbol.object_trivial_lifecycle)) {
          fail(line, "unknown trivial_lifecycle mode '" + value + "'");
        }
        saw_trivial_lifecycle = true;
      } else if(key == "force_inline") {
        if(saw_force_inline) {
          fail(line, "duplicate force_inline metadata");
        }
        if(!parse_yes_no_text(value, symbol.force_inline)) {
          fail(line, "unknown force_inline mode '" + value + "'");
        }
        saw_force_inline = true;
      } else {
        fail(line, "unknown function metadata key '" + key + "'");
      }
      if(stream.consume("]")) {
        break;
      }
      stream.expect(line, ",");
    }
  }
}

void parse_parameter_metadata(ParameterMetadata & metadata,
                              TokenStream & stream,
                              const LineInfo & line)
{
  if(!stream.consume("[")) {
    return;
  }
  bool saw_pass = false;
  bool saw_capture = false;
  bool saw_access = false;
  bool saw_alias = false;
  for(;;) {
    if(stream.eof()) {
      fail(line, "unterminated parameter metadata");
    }
    const string key = stream.take();
    stream.expect(line, "=");
    if(stream.eof()) {
      fail(line, "expected metadata value");
    }
    const string value = stream.take();
    if(key == "pass") {
      if(saw_pass) {
        fail(line, "duplicate pass metadata");
      }
      metadata.passing = parse_param_passing_mode_text(value);
      if(value != "direct" && metadata.passing == PPM_DIRECT) {
        fail(line, "unknown parameter pass mode '" + value + "'");
      }
      saw_pass = true;
    } else if(key == "capture") {
      if(saw_capture) {
        fail(line, "duplicate capture metadata");
      }
      metadata.capture = parse_param_capture_mode_text(value);
      if(metadata.capture == PCM_DEFAULT) {
        fail(line, "unknown parameter capture mode '" + value + "'");
      }
      saw_capture = true;
    } else if(key == "access") {
      if(saw_access) {
        fail(line, "duplicate access metadata");
      }
      metadata.access = parse_param_access_mode_text(value);
      if(metadata.access == PAM_DEFAULT) {
        fail(line, "unknown parameter access mode '" + value + "'");
      }
      saw_access = true;
    } else if(key == "alias") {
      if(saw_alias) {
        fail(line, "duplicate alias metadata");
      }
      metadata.alias = parse_param_alias_mode_text(value);
      if(metadata.alias == PALM_DEFAULT) {
        fail(line, "unknown parameter alias mode '" + value + "'");
      }
      saw_alias = true;
    } else {
      fail(line, "unknown parameter metadata key '" + key + "'");
    }
    if(stream.consume("]")) {
      break;
    }
    stream.expect(line, ",");
  }
}

bool parse_float_literal_token(const string & token,
                               long double & value,
                               LowType & literal_type)
{
  auto lower_ascii = [](string text) -> string {
    for(char & ch : text) {
      ch = static_cast<char>(tolower(static_cast<unsigned char>(ch)));
    }
    return text;
  };
  auto strip_suffix = [](const string & text,
                         const string & suffix,
                         string & stripped) -> bool {
    if(text.size() < suffix.size() ||
       text.compare(text.size() - suffix.size(), suffix.size(), suffix) != 0) {
      return false;
    }
    stripped = text.substr(0, text.size() - suffix.size());
    return true;
  };
  auto try_parse_signaling_nan = [&]() -> bool {
    bool negative = false;
    string parsed = token;
    if(!parsed.empty() && (parsed[0] == '-' || parsed[0] == '+')) {
      negative = parsed[0] == '-';
      parsed.erase(parsed.begin());
    }
    if(parsed == "snanf" || parsed == "snanF") {
      literal_type.text = "f32";
    } else if(parsed == "snanl" || parsed == "snanL") {
      literal_type.text = "f80";
    } else if(parsed == "snan") {
      literal_type.text = "f64";
    } else {
      return false;
    }
    value = numeric_limits<long double>::quiet_NaN();
    if(negative) {
      value = -value;
    }
    return true;
  };
  auto token_consumed = [](const char * end) -> bool {
    return end && *end == '\0';
  };
  auto try_parse_f32 = [&](const string & parsed) -> bool {
    if(parsed.empty()) {
      return false;
    }
    char * end = nullptr;
    const float parsed_value = strtof(parsed.c_str(), &end);
    if(!token_consumed(end)) {
      return false;
    }
    value = parsed_value;
    return true;
  };
  auto try_parse_f64 = [&](const string & parsed) -> bool {
    if(parsed.empty()) {
      return false;
    }
    char * end = nullptr;
    const double parsed_value = strtod(parsed.c_str(), &end);
    if(!token_consumed(end)) {
      return false;
    }
    value = parsed_value;
    return true;
  };
  auto try_parse_f80 = [&](const string & parsed) -> bool {
    if(parsed.empty()) {
      return false;
    }
    char * end = nullptr;
    const long double parsed_value = strtold(parsed.c_str(), &end);
    if(!token_consumed(end)) {
      return false;
    }
    value = parsed_value;
    return true;
  };

  if(try_parse_signaling_nan()) {
    return true;
  }

  const string lowered = lower_ascii(token);
  string parsed;
  if(strip_suffix(lowered, "f16", parsed) ||
     strip_suffix(lowered, "bf16", parsed) ||
     strip_suffix(lowered, "f32", parsed)) {
    literal_type.text = "f32";
    return try_parse_f32(parsed);
  }
  if(strip_suffix(lowered, "f64", parsed) ||
     strip_suffix(lowered, "f32x", parsed)) {
    literal_type.text = "f64";
    return try_parse_f64(parsed);
  }
  if(strip_suffix(lowered, "f128", parsed) ||
     strip_suffix(lowered, "f64x", parsed)) {
    literal_type.text = "f80";
    return try_parse_f80(parsed);
  }

  literal_type.text = "f64";
  if(try_parse_f64(token)) {
    return true;
  }

  string standard_suffix_parsed = token;
  if(!standard_suffix_parsed.empty() &&
     (standard_suffix_parsed.back() == 'f' ||
      standard_suffix_parsed.back() == 'F')) {
    literal_type.text = "f32";
    standard_suffix_parsed.erase(standard_suffix_parsed.end() - 1);
    return try_parse_f32(standard_suffix_parsed);
  }
  if(!standard_suffix_parsed.empty() &&
     (standard_suffix_parsed.back() == 'l' ||
      standard_suffix_parsed.back() == 'L')) {
    literal_type.text = "f80";
    standard_suffix_parsed.erase(standard_suffix_parsed.end() - 1);
    return try_parse_f80(standard_suffix_parsed);
  }
  return false;
}

bool is_terminator_instruction(const Instruction & instruction)
{
  return instruction.kind == Instruction::IK_JUMP ||
         instruction.kind == Instruction::IK_BRANCH ||
         instruction.kind == Instruction::IK_SWITCH ||
         instruction.kind == Instruction::IK_RETURN ||
         instruction.kind == Instruction::IK_THROW ||
         instruction.kind == Instruction::IK_RESUME;
}

void collect_block_targets(const Instruction & instruction,
                           vector<string> & block_targets)
{
  const auto maybe_push = [&](const Operand & operand)
    {
      if(operand.kind == Operand::OP_LABEL) {
        block_targets.push_back(operand.text);
      }
    };
  maybe_push(instruction.first);
  maybe_push(instruction.second);
  maybe_push(instruction.third);
  for(size_t i = 0; i < instruction.args.size(); ++i) {
    maybe_push(instruction.args[i]);
  }
}

void ensure_unique_parameters(const vector<Parameter> & params,
                              const LineInfo & line,
                              const string & what)
{
  set<string> names;
  for(size_t i = 0; i < params.size(); ++i) {
    if(!names.insert(params[i].name).second) {
      fail(line, what + " has duplicate parameter " + params[i].name);
    }
  }
}

void validate_parameter_metadata(const vector<Parameter> & params,
                                 const LowType & return_type,
                                 const LineInfo & line,
                                 const string & what)
{
  bool saw_indirect_result = false;
  for(size_t i = 0; i < params.size(); ++i) {
    const Parameter & param = params[i];
    if(param.metadata.capture != PCM_DEFAULT &&
       param.type.text != "ptr") {
      fail(line,
           what + " parameter " + param.name + " with capture mode '" +
               string(param_capture_mode_text(param.metadata.capture)) +
               "' must have type ptr");
    }
    if(param.metadata.access != PAM_DEFAULT &&
       param.type.text != "ptr") {
      fail(line,
           what + " parameter " + param.name + " with access mode '" +
               string(param_access_mode_text(param.metadata.access)) +
               "' must have type ptr");
    }
    if(param.metadata.alias != PALM_DEFAULT &&
       param.type.text != "ptr") {
      fail(line,
           what + " parameter " + param.name + " with alias mode '" +
               string(param_alias_mode_text(param.metadata.alias)) +
               "' must have type ptr");
    }
    if(param.metadata.passing == PPM_DIRECT) {
      continue;
    }
    const bool requires_ptr =
        param.metadata.passing == PPM_INDIRECT_RESULT ||
        param.metadata.passing == PPM_BY_ADDRESS ||
        param.metadata.passing == PPM_REFERENCE ||
        param.metadata.passing == PPM_DECAY;
    if(requires_ptr && param.type.text != "ptr") {
      fail(line,
           what + " parameter " + param.name + " with pass mode '" +
               string(param_passing_mode_text(param.metadata.passing)) +
               "' must have type ptr");
    }
    if(param.metadata.passing == PPM_INDIRECT_RESULT) {
      if(i != 0) {
        fail(line, what + " indirect result parameter must appear first");
      }
      if(return_type.text != "void") {
        fail(line, what + " indirect result parameter requires return type void");
      }
      if(saw_indirect_result) {
        fail(line, what + " has duplicate indirect result parameters");
      }
      saw_indirect_result = true;
    }
  }
}

bool operand_is_scalar_literal_for_type(const Operand & operand,
                                        const LowType & type)
{
  if(is_object_type(type)) {
    return false;
  }
  if(type.text == "f32" || type.text == "f64" || type.text == "f80") {
    return operand.kind == Operand::OP_FLOAT || operand.kind == Operand::OP_INTEGER;
  }
  if(type.text == "void") {
    return false;
  }
  if(type.text == "ptr") {
    return operand.kind == Operand::OP_INTEGER;
  }
  return operand.kind == Operand::OP_INTEGER;
}

LowType parse_type(TokenStream & stream, const LineInfo & line)
{
  if(stream.eof()) {
    fail(line, "expected type");
  }
  const string token = stream.take();
  if(!is_valid_type(token)) {
    fail(line, "unknown type '" + token + "'");
  }
  LowType type;
  type.text = token;
  return type;
}

bool is_integer_token(const string & token)
{
  if(token.empty()) {
    return false;
  }
  size_t start = token[0] == '-' ? 1 : 0;
  if(start == token.size()) {
    return false;
  }
  for(size_t i = start; i < token.size(); ++i) {
    if(!isdigit(static_cast<unsigned char>(token[i]))) {
      return false;
    }
  }
  return true;
}

long long parse_integer_token_bits(const string & token)
{
  errno = 0;
  char * end = NULL;
  if(!token.empty() && token[0] == '-') {
    const long long value = strtoll(token.c_str(), &end, 10);
    if(end == NULL || *end != '\0' || errno == ERANGE) {
      throw logic_error("invalid signed integer literal");
    }
    return value;
  }
  const unsigned long long value = strtoull(token.c_str(), &end, 10);
  if(end == NULL || *end != '\0' || errno == ERANGE) {
    throw logic_error("invalid unsigned integer literal");
  }
  return static_cast<long long>(value);
}

Operand parse_operand(TokenStream & stream, const LineInfo & line)
{
  if(stream.eof()) {
    fail(line, "expected operand");
  }
  const string token = stream.take();
  Operand operand;
  operand.text = token;
  if(!token.empty() && token[0] == '%') {
    operand.kind = Operand::OP_TEMP;
    return operand;
  }
  if(!token.empty() && token[0] == '$') {
    operand.kind = Operand::OP_SLOT;
    return operand;
  }
  if(!token.empty() && token[0] == '@') {
    operand.kind = Operand::OP_GLOBAL;
    return operand;
  }
  if(!token.empty() && token[0] == '^') {
    operand.kind = Operand::OP_LABEL;
    return operand;
  }
  if(token == "nullptr") {
    operand.kind = Operand::OP_INTEGER;
    operand.int_value = 0;
    return operand;
  }
  if(is_integer_token(token)) {
    operand.kind = Operand::OP_INTEGER;
    try
    {
      operand.int_value = parse_integer_token_bits(token);
    }
    catch(const logic_error &)
    {
      fail(line, "invalid integer literal '" + token + "'");
    }
    return operand;
  }
  if(parse_float_literal_token(token, operand.float_value, operand.literal_type)) {
    operand.kind = Operand::OP_FLOAT;
    return operand;
  }
  fail(line, "invalid operand '" + token + "'");
  return operand;
}

string parse_name_with_prefix(TokenStream & stream,
                              const LineInfo & line,
                              char prefix,
                              const string & what)
{
  if(stream.eof()) {
    fail(line, "expected " + what);
  }
  const string token = stream.take();
  if(token.empty() || token[0] != prefix) {
    fail(line, "expected " + what);
  }
  return token;
}

long long parse_integer_literal(TokenStream & stream,
                                const LineInfo & line,
                                const string & what)
{
  if(stream.eof()) {
    fail(line, "expected " + what);
  }
  const string token = stream.take();
  if(!is_integer_token(token)) {
    fail(line, "expected " + what);
  }
  try
  {
    return parse_integer_token_bits(token);
  }
  catch(const logic_error &)
  {
    fail(line, "expected " + what);
  }
  return 0;
}

long long parse_optional_address_addend(TokenStream & stream,
                                        const LineInfo & line)
{
  if(stream.consume("+")) {
    return parse_integer_literal(stream, line, "address addend");
  }
  if(stream.consume("-")) {
    return -parse_integer_literal(stream, line, "address addend");
  }
  return 0;
}

size_t parse_positive_byte_count(TokenStream & stream, const LineInfo & line)
{
  const long long value =
      parse_integer_literal(stream, line, "positive byte count");
  if(value <= 0) {
    fail(line, "byte count must be positive");
  }
  return static_cast<size_t>(value);
}

void parse_storage_span(TokenStream & stream,
                        const LineInfo & line,
                        size_t & byte_count,
                        size_t & byte_alignment)
{
  if(stream.eof()) {
    fail(line, "expected storage span");
  }
  const string token = stream.take();
  const size_t x_pos = token.find('x');
  const string size_text = x_pos == string::npos ? token : token.substr(0, x_pos);
  const string align_text =
      x_pos == string::npos ? string() : token.substr(x_pos + 1);
  if(!is_integer_token(size_text)) {
    fail(line, "expected storage span");
  }
  const long long parsed_size = atoll(size_text.c_str());
  if(parsed_size <= 0) {
    fail(line, "byte count must be positive");
  }
  byte_count = static_cast<size_t>(parsed_size);
  if(align_text.empty()) {
    byte_alignment = 1;
    return;
  }
  if(!is_integer_token(align_text)) {
    fail(line, "expected storage alignment");
  }
  const long long parsed_alignment = atoll(align_text.c_str());
  if(parsed_alignment <= 0 ||
     (parsed_alignment & (parsed_alignment - 1)) != 0) {
    fail(line, "storage alignment must be a positive power of two");
  }
  byte_alignment = static_cast<size_t>(parsed_alignment);
}

GlobalDefinition parse_scalar_global(const LineInfo & line)
{
  const vector<string> tokens = lex_line(line.text);
  TokenStream stream(tokens);
  stream.expect(line, "global");
  GlobalDefinition global;
  global.name = parse_name_with_prefix(stream, line, '@', "global name");
  if(stream.consume("readonly")) {
    global.storage = GSM_READONLY;
  } else if(stream.consume("thread_local")) {
    global.storage = GSM_THREAD_LOCAL;
  }
  stream.expect(line, ":");
  global.type = parse_type(stream, line);
  if(is_object_type(global.type)) {
    fail(line, "direct object types are not supported in globals");
  }
  parse_global_metadata(global.storage, global.metadata, stream, line);
  if(global.metadata.role != SR_NONE &&
     !is_global_symbol_role(global.metadata.role)) {
    fail(line, "invalid global role '" + string(symbol_role_text(global.metadata.role)) + "'");
  }
  stream.expect(line, "=");
  if(stream.consume("zero")) {
    global.init_kind = GlobalDefinition::INIT_ZERO;
  } else if(stream.consume("addr")) {
    global.init_kind = GlobalDefinition::INIT_ADDR;
    global.init_operand = parse_operand(stream, line);
    if(global.init_operand.kind != Operand::OP_GLOBAL) {
      fail(line, "addr initializer requires @name");
    }
    global.addr_addend = parse_optional_address_addend(stream, line);
  } else {
    global.init_kind = GlobalDefinition::INIT_INTEGER;
    global.init_operand = parse_operand(stream, line);
    if(!operand_is_scalar_literal_for_type(global.init_operand, global.type)) {
      fail(line, "scalar literal initializer required");
    }
  }
  if(!stream.eof()) {
    fail(line, "unexpected trailing tokens in global definition");
  }
  return global;
}

GlobalDefinition parse_structured_global_header(const LineInfo & line)
{
  const vector<string> tokens = lex_line(line.text);
  TokenStream stream(tokens);
  stream.expect(line, "global");
  GlobalDefinition global;
  global.structured = true;
  global.name = parse_name_with_prefix(stream, line, '@', "global name");
  if(stream.consume("readonly")) {
    global.storage = GSM_READONLY;
  } else if(stream.consume("thread_local")) {
    global.storage = GSM_THREAD_LOCAL;
  }
  parse_global_metadata(global.storage, global.metadata, stream, line);
  if(global.metadata.role != SR_NONE &&
     !is_global_symbol_role(global.metadata.role)) {
    fail(line, "invalid global role '" + string(symbol_role_text(global.metadata.role)) + "'");
  }
  stream.expect(line, "=");
  stream.expect(line, "{");
  if(!stream.eof()) {
    fail(line, "unexpected trailing tokens in structured global header");
  }
  return global;
}

GlobalDefinition::DataItem parse_global_data_item(const LineInfo & line)
{
  const vector<string> tokens = lex_line(line.text);
  TokenStream stream(tokens);
  GlobalDefinition::DataItem item;
  if(stream.consume("zero")) {
    item.kind = GlobalDefinition::DataItem::ITEM_ZERO;
    item.zero_bytes = parse_positive_byte_count(stream, line);
  } else {
    item.type = parse_type(stream, line);
    if(is_object_type(item.type)) {
      fail(line, "direct object types are not supported in global data items");
    }
    if(item.type.text == "ptr") {
      stream.expect(line, "addr");
      const Operand symbol = parse_operand(stream, line);
      if(symbol.kind != Operand::OP_GLOBAL) {
        fail(line, "ptr data item requires addr @symbol");
      }
      item.kind = GlobalDefinition::DataItem::ITEM_ADDR;
      item.symbol = symbol.text;
      item.addr_addend = parse_optional_address_addend(stream, line);
    } else {
      if(item.type.text == "void") {
        fail(line, "void is not a valid data item type");
      }
      item.kind = GlobalDefinition::DataItem::ITEM_INTEGER;
      item.literal_operand = parse_operand(stream, line);
      if(!operand_is_scalar_literal_for_type(item.literal_operand, item.type)) {
        fail(line, "scalar literal required");
      }
    }
  }
  if(!stream.eof()) {
    fail(line, "unexpected trailing tokens in structured global item");
  }
  return item;
}

void parse_instruction_debug_location(InstructionDebugLocation & debug_location,
                                      TokenStream & stream,
                                      const LineInfo & line);

Function parse_function_header(const LineInfo & line)
{
  const vector<string> tokens = lex_line(line.text);
  TokenStream stream(tokens);
  stream.expect(line, "function");
  Function function;
  function.name = parse_name_with_prefix(stream, line, '@', "function name");
  stream.expect(line, "(");
  if(!stream.consume(")")) {
    for(;;) {
      Parameter param;
      param.name = parse_name_with_prefix(stream, line, '%', "parameter");
      stream.expect(line, ":");
      param.type = parse_type(stream, line);
      parse_parameter_metadata(param.metadata, stream, line);
      function.params.push_back(param);
      if(stream.consume(")")) {
        break;
      }
      stream.expect(line, ",");
    }
  }
  stream.expect(line, "->");
  function.return_type = parse_type(stream, line);
  parse_function_metadata(function.boundary, function.metadata, stream, line);
  if(function.metadata.role != SR_NONE &&
     !is_function_symbol_role(function.metadata.role)) {
    fail(line, "invalid function role '" + string(symbol_role_text(function.metadata.role)) + "'");
  }
  parse_instruction_debug_location(function.debug_location, stream, line);
  stream.expect(line, "{");
  if(!stream.eof()) {
    fail(line, "unexpected trailing tokens in function header");
  }
  ensure_unique_parameters(function.params, line, "function " + function.name);
  validate_parameter_metadata(function.params, function.return_type, line, "function " + function.name);
  return function;
}

vector<Parameter> parse_function_parameters(TokenStream & stream,
                                            const LineInfo & line)
{
  vector<Parameter> params;
  stream.expect(line, "(");
  if(!stream.consume(")")) {
    for(;;) {
      Parameter param;
      param.name = parse_name_with_prefix(stream, line, '%', "parameter");
      stream.expect(line, ":");
      param.type = parse_type(stream, line);
      parse_parameter_metadata(param.metadata, stream, line);
      params.push_back(param);
      if(stream.consume(")")) {
        break;
      }
      stream.expect(line, ",");
    }
  }
  return params;
}

FunctionDeclaration parse_function_declaration(const LineInfo & line)
{
  const vector<string> tokens = lex_line(line.text);
  TokenStream stream(tokens);
  stream.expect(line, "declare");
  stream.expect(line, "function");
  FunctionDeclaration function;
  function.name = parse_name_with_prefix(stream, line, '@', "function name");
  function.params = parse_function_parameters(stream, line);
  stream.expect(line, "->");
  function.return_type = parse_type(stream, line);
  parse_function_metadata(function.boundary, function.metadata, stream, line);
  if(function.metadata.role != SR_NONE &&
     !is_function_symbol_role(function.metadata.role)) {
    fail(line, "invalid function role '" + string(symbol_role_text(function.metadata.role)) + "'");
  }
  if(!stream.eof()) {
    fail(line, "unexpected trailing tokens in function declaration");
  }
  ensure_unique_parameters(function.params, line, "function declaration " + function.name);
  validate_parameter_metadata(function.params,
                              function.return_type,
                              line,
                              "function declaration " + function.name);
  return function;
}

GlobalDeclaration parse_global_declaration(const LineInfo & line)
{
  const vector<string> tokens = lex_line(line.text);
  TokenStream stream(tokens);
  stream.expect(line, "declare");
  stream.expect(line, "global");
  GlobalDeclaration global;
  global.name = parse_name_with_prefix(stream, line, '@', "global name");
  if(stream.consume(":")) {
    global.has_type = true;
    global.type = parse_type(stream, line);
    if(is_object_type(global.type)) {
      fail(line, "direct object types are not supported in globals");
    }
  }
  parse_global_metadata(global.storage, global.metadata, stream, line);
  if(global.metadata.role != SR_NONE &&
     !is_global_symbol_role(global.metadata.role)) {
    fail(line, "invalid global role '" + string(symbol_role_text(global.metadata.role)) + "'");
  }
  if(!stream.eof()) {
    fail(line, "unexpected trailing tokens in global declaration");
  }
  return global;
}

ObjectAlias parse_object_alias(const LineInfo & line)
{
  const vector<string> tokens = lex_line(line.text);
  TokenStream stream(tokens);
  stream.expect(line, "alias");
  stream.expect(line, "object");
  if(stream.eof()) {
    fail(line, "expected alias object symbol");
  }
  ObjectAlias alias;
  alias.object_symbol = stream.take();
  if(alias.object_symbol.empty() ||
     alias.object_symbol == "=" ||
     alias.object_symbol == "(" ||
     alias.object_symbol == ")" ||
     alias.object_symbol == "{" ||
     alias.object_symbol == "}" ||
     alias.object_symbol == "[" ||
     alias.object_symbol == "]" ||
     alias.object_symbol == "," ||
     alias.object_symbol == ":" ||
     alias.object_symbol == "->") {
    fail(line, "expected alias object symbol");
  }
  stream.expect(line, "=");
  alias.target = parse_name_with_prefix(stream, line, '@', "alias target");
  if(!stream.eof()) {
    fail(line, "unexpected trailing tokens in object alias");
  }
  return alias;
}

pair<string, LowType> parse_slot(const LineInfo & line)
{
  const vector<string> tokens = lex_line(line.text);
  TokenStream stream(tokens);
  stream.expect(line, "slot");
  const string name = parse_name_with_prefix(stream, line, '$', "slot");
  stream.expect(line, ":");
  const LowType type = parse_type(stream, line);
  if(!stream.eof()) {
    fail(line, "unexpected trailing tokens in slot definition");
  }
  return make_pair(name, type);
}

string parse_block_label(const LineInfo & line)
{
  const vector<string> tokens = lex_line(line.text);
  TokenStream stream(tokens);
  stream.expect(line, "block");
  const string label = parse_name_with_prefix(stream, line, '^', "block label");
  stream.expect(line, ":");
  if(!stream.eof()) {
    fail(line, "unexpected trailing tokens in block definition");
  }
  return label;
}

vector<Parameter> parse_function_parameters(TokenStream & stream,
                                            const LineInfo & line);

vector<Operand> parse_call_arguments(TokenStream & stream, const LineInfo & line)
{
  vector<Operand> args;
  stream.expect(line, "(");
  if(stream.consume(")")) {
    return args;
  }
  for(;;) {
    args.push_back(parse_operand(stream, line));
    if(stream.consume(")")) {
      break;
    }
    stream.expect(line, ",");
  }
  return args;
}

void parse_call_signature(Instruction & instruction,
                          TokenStream & stream,
                          const LineInfo & line)
{
  if(!stream.consume("as")) {
    return;
  }
  instruction.has_call_signature = true;
  instruction.call_params = parse_function_parameters(stream, line);
  stream.expect(line, "->");
  instruction.call_return_type = parse_type(stream, line);
  SymbolMetadata metadata;
  parse_function_metadata(instruction.call_boundary, metadata, stream, line);
  if(metadata.role != SR_NONE ||
     metadata.linkage != LLM_DEFAULT ||
     metadata.binding != SBM_DEFAULT ||
     !metadata.object_symbol.empty() ||
     !metadata.tls_for_symbol.empty() ||
     metadata.keep_internal_alias ||
     metadata.prefer_local_object_binding ||
     metadata.object_output_root ||
     metadata.object_trivial_lifecycle ||
     metadata.force_inline) {
    fail(line, "call signature metadata does not allow symbol metadata");
  }
}

void parse_instruction_debug_location(InstructionDebugLocation & debug_location,
                                      TokenStream & stream,
                                      const LineInfo & line)
{
  if(!stream.consume("!dbg")) {
    return;
  }
  stream.expect(line, "(");
  if(stream.eof()) {
    fail(line, "expected debug source file");
  }
  const string file = stream.take();
  if(file == "(" || file == ")" || file == "," ||
     file == "[" || file == "]" || file == "{" ||
     file == "}" || file == ":" || file == "=" ||
     file == "->") {
    fail(line, "expected debug source file");
  }
  stream.expect(line, ",");
  const long long debug_line = parse_integer_literal(stream, line, "debug line");
  if(debug_line <= 0) {
    fail(line, "debug line must be positive");
  }
  stream.expect(line, ",");
  const long long debug_column = parse_integer_literal(stream, line, "debug column");
  if(debug_column <= 0) {
    fail(line, "debug column must be positive");
  }
  stream.expect(line, ")");
  debug_location.file = file;
  debug_location.line = static_cast<size_t>(debug_line);
  debug_location.column = static_cast<size_t>(debug_column);
}

ir_model::ExportedSymbol synthesized_export_identity(const string & name,
                                                     SymbolBindingMode binding)
{
  string object_name = name;
  if(binding != SBM_INTERNAL && !object_name.empty() && object_name[0] == '@') {
    object_name.erase(0, 1);
  }
  ir_model::ExportedSymbol out;
  out.internal_symbol = name;
  out.object_symbol = object_name;
  out.linkage = ir_symbol_linkage_from_binding(binding);
  return out;
}

void maybe_add_exported_symbol(Program & program,
                               const string & name,
                               const SymbolMetadata & metadata)
{
  if(metadata.binding == SBM_DEFAULT &&
     metadata.object_symbol.empty() &&
     !metadata.keep_internal_alias &&
     !metadata.prefer_local_object_binding) {
    return;
  }
  ir_model::ExportedSymbol identity =
      synthesized_export_identity(name, metadata.binding);
  if(!metadata.object_symbol.empty()) {
    identity.object_symbol = metadata.object_symbol;
  }
  identity.keep_internal_alias = metadata.keep_internal_alias;
  identity.prefer_local_object_binding = metadata.prefer_local_object_binding;
  identity.linkage = ir_symbol_linkage_from_binding(metadata.binding);
  program.exported_symbols.push_back(identity);
}

Instruction parse_instruction(const LineInfo & line)
{
  const vector<string> tokens = lex_line(line.text);
  TokenStream stream(tokens);
  Instruction instruction;

  const auto parse_order_literal =
      [&](Operand & out_operand, const string & what)
      {
        out_operand.kind = Operand::OP_INTEGER;
        out_operand.int_value = parse_integer_literal(stream, line, what);
        out_operand.text = to_string(out_operand.int_value);
      };

  const auto finish_instruction =
      [&]() -> Instruction
      {
        parse_instruction_debug_location(instruction.debug_location, stream, line);
        if(!stream.eof()) {
          fail(line, "unexpected trailing tokens in instruction");
        }
        return instruction;
      };

  if(stream.consume("store")) {
    instruction.kind = Instruction::IK_STORE;
    instruction.type = parse_type(stream, line);
    instruction.first = parse_operand(stream, line);
    stream.expect(line, ",");
    instruction.second = parse_operand(stream, line);
    return finish_instruction();
  }

  if(stream.consume("atomic_store")) {
    instruction.kind = Instruction::IK_ATOMIC_STORE;
    instruction.type = parse_type(stream, line);
    instruction.first = parse_operand(stream, line);
    stream.expect(line, ",");
    instruction.second = parse_operand(stream, line);
    stream.expect(line, ",");
    parse_order_literal(instruction.third, "atomic order");
    return finish_instruction();
  }

  if(stream.consume("call")) {
    instruction.kind = Instruction::IK_CALL;
    instruction.call_returns_void = true;
    instruction.type = parse_type(stream, line);
    if(instruction.type.text != "void") {
      fail(line, "void call must use return type void");
    }
    instruction.first = parse_operand(stream, line);
    instruction.args = parse_call_arguments(stream, line);
    parse_call_signature(instruction, stream, line);
    if(instruction.has_call_signature &&
       instruction.call_return_type.text != "void") {
      fail(line, "void call signature must use return type void");
    }
    if(instruction.first.kind != Operand::OP_GLOBAL && !instruction.has_call_signature) {
      fail(line, "indirect call requires explicit call signature");
    }
    return finish_instruction();
  }

  if(stream.consume("atomic_thread_fence")) {
    instruction.kind = Instruction::IK_ATOMIC_THREAD_FENCE;
    parse_order_literal(instruction.first, "atomic order");
    return finish_instruction();
  }

  if(stream.consume("atomic_signal_fence")) {
    instruction.kind = Instruction::IK_ATOMIC_SIGNAL_FENCE;
    parse_order_literal(instruction.first, "atomic order");
    return finish_instruction();
  }

  if(stream.consume("va_start")) {
    instruction.kind = Instruction::IK_VA_START;
    instruction.first = parse_operand(stream, line);
    if(instruction.first.kind == Operand::OP_LABEL) {
      fail(line, "va_start requires value operand");
    }
    return finish_instruction();
  }

  if(stream.consume("copyobj")) {
    instruction.kind = Instruction::IK_COPYOBJ;
    parse_storage_span(stream, line, instruction.byte_count, instruction.byte_alignment);
    instruction.first = parse_operand(stream, line);
    stream.expect(line, ",");
    instruction.second = parse_operand(stream, line);
    return finish_instruction();
  }

  if(stream.consume("zeroinit")) {
    instruction.kind = Instruction::IK_ZEROINIT;
    parse_storage_span(stream, line, instruction.byte_count, instruction.byte_alignment);
    instruction.first = parse_operand(stream, line);
    return finish_instruction();
  }

  if(stream.consume("eh_try")) {
    instruction.kind = Instruction::IK_EH_TRY;
    instruction.first = parse_operand(stream, line);
    if(instruction.first.kind != Operand::OP_LABEL) {
      fail(line, "eh_try requires block label");
    }
    return finish_instruction();
  }

  if(stream.consume("eh_cleanup")) {
    if(stream.eof() || stream.peek() == "!dbg") {
      instruction.kind = Instruction::IK_EH_CLEANUP_CLAUSE;
      return finish_instruction();
    }
    instruction.kind = Instruction::IK_EH_CLEANUP;
    instruction.first = parse_operand(stream, line);
    if(instruction.first.kind != Operand::OP_LABEL) {
      fail(line, "eh_cleanup requires block label");
    }
    return finish_instruction();
  }

  if(stream.consume("eh_catch")) {
    instruction.kind = Instruction::IK_EH_CATCH;
    instruction.first = parse_operand(stream, line);
    if(instruction.first.kind != Operand::OP_GLOBAL) {
      fail(line, "eh_catch requires @symbol");
    }
    if(stream.consume(",")) {
      instruction.eh_selector = parse_integer_literal(stream, line, "EH selector");
      instruction.has_eh_selector = true;
      if(instruction.eh_selector <= 0) {
        fail(line, "EH selector must be positive");
      }
    }
    return finish_instruction();
  }

  if(stream.consume("eh_filter")) {
    instruction.kind = Instruction::IK_EH_FILTER;
    if(!stream.eof() && stream.peek() != "!dbg") {
      instruction.args.push_back(parse_operand(stream, line));
      if(instruction.args.back().kind != Operand::OP_GLOBAL) {
        fail(line, "eh_filter requires @symbol operands");
      }
      while(stream.consume(",")) {
        instruction.args.push_back(parse_operand(stream, line));
        if(instruction.args.back().kind != Operand::OP_GLOBAL) {
          fail(line, "eh_filter requires @symbol operands");
        }
      }
    }
    return finish_instruction();
  }

  if(stream.consume("eh_catch_all")) {
    instruction.kind = Instruction::IK_EH_CATCH_ALL;
    if(stream.consume(",")) {
      instruction.eh_selector = parse_integer_literal(stream, line, "EH selector");
      instruction.has_eh_selector = true;
      if(instruction.eh_selector <= 0) {
        fail(line, "EH selector must be positive");
      }
    }
    return finish_instruction();
  }

  if(stream.consume("eh_end")) {
    instruction.kind = Instruction::IK_EH_END;
    return finish_instruction();
  }

  if(stream.consume("throw")) {
    instruction.kind = Instruction::IK_THROW;
    instruction.type = parse_type(stream, line);
    if(instruction.type.text == "void") {
      fail(line, "throw requires non-void type");
    }
    instruction.first = parse_operand(stream, line);
    return finish_instruction();
  }

  if(stream.consume("resume")) {
    instruction.kind = Instruction::IK_RESUME;
    return finish_instruction();
  }

  if(stream.consume("jump")) {
    instruction.kind = Instruction::IK_JUMP;
    instruction.first = parse_operand(stream, line);
    if(instruction.first.kind != Operand::OP_LABEL) {
      fail(line, "jump requires block label");
    }
    return finish_instruction();
  }

  if(stream.consume("branch")) {
    instruction.kind = Instruction::IK_BRANCH;
    instruction.first = parse_operand(stream, line);
    stream.expect(line, ",");
    instruction.second = parse_operand(stream, line);
    stream.expect(line, ",");
    instruction.third = parse_operand(stream, line);
    if(instruction.second.kind != Operand::OP_LABEL ||
       instruction.third.kind != Operand::OP_LABEL) {
      fail(line, "branch requires block labels");
    }
    return finish_instruction();
  }

  if(stream.consume("switch")) {
    instruction.kind = Instruction::IK_SWITCH;
    instruction.first = parse_operand(stream, line);
    if(instruction.first.kind == Operand::OP_LABEL) {
      fail(line, "switch selector must be a value operand");
    }
    stream.expect(line, ",");
    instruction.second = parse_operand(stream, line);
    if(instruction.second.kind != Operand::OP_LABEL) {
      fail(line, "switch default target requires block label");
    }
    while(stream.consume(",")) {
      Operand case_value = parse_operand(stream, line);
      if(case_value.kind == Operand::OP_LABEL) {
        fail(line, "switch case value must be a value operand");
      }
      stream.expect(line, ":");
      Operand case_target = parse_operand(stream, line);
      if(case_target.kind != Operand::OP_LABEL) {
        fail(line, "switch case target requires block label");
      }
      instruction.args.push_back(case_value);
      instruction.args.push_back(case_target);
    }
    return finish_instruction();
  }

  if(stream.consume("return")) {
    instruction.kind = Instruction::IK_RETURN;
    instruction.type = parse_type(stream, line);
    if(instruction.type.text != "void") {
      instruction.first = parse_operand(stream, line);
    }
    return finish_instruction();
  }

  instruction.dest =
      parse_name_with_prefix(stream, line, '%', "destination temporary");
  stream.expect(line, "=");

  if(stream.consume("const")) {
    instruction.kind = Instruction::IK_CONST;
    instruction.type = parse_type(stream, line);
    instruction.first = parse_operand(stream, line);
    if(!operand_is_scalar_literal_for_type(instruction.first, instruction.type)) {
      fail(line, "const requires scalar literal");
    }
  } else if(stream.consume("copy")) {
    instruction.kind = Instruction::IK_COPY;
    instruction.type = parse_type(stream, line);
    instruction.first = parse_operand(stream, line);
  } else if(stream.consume("addr")) {
    instruction.kind = Instruction::IK_ADDR;
    instruction.first = parse_operand(stream, line);
    if(instruction.first.kind != Operand::OP_GLOBAL &&
       instruction.first.kind != Operand::OP_SLOT) {
      fail(line, "addr requires @name or $slot");
    }
    instruction.type.text = "ptr";
  } else if(stream.consume("load")) {
    instruction.kind = Instruction::IK_LOAD;
    instruction.type = parse_type(stream, line);
    instruction.first = parse_operand(stream, line);
  } else if(stream.consume("atomic_load")) {
    instruction.kind = Instruction::IK_ATOMIC_LOAD;
    instruction.type = parse_type(stream, line);
    instruction.first = parse_operand(stream, line);
    stream.expect(line, ",");
    parse_order_literal(instruction.second, "atomic order");
  } else if(stream.consume("index")) {
    instruction.kind = Instruction::IK_INDEX;
    instruction.op = parse_type(stream, line).text;
    parse_index_metadata(instruction.index_projection, stream, line);
    instruction.type.text = "ptr";
    instruction.first = parse_operand(stream, line);
    stream.expect(line, ",");
    instruction.second = parse_operand(stream, line);
  } else if(stream.consume("unary")) {
    instruction.kind = Instruction::IK_UNARY;
    if(stream.eof()) {
      fail(line, "expected unary operator");
    }
    instruction.op = stream.take();
    instruction.type = parse_type(stream, line);
    instruction.first = parse_operand(stream, line);
  } else if(stream.consume("binary")) {
    instruction.kind = Instruction::IK_BINARY;
    if(stream.eof()) {
      fail(line, "expected binary operator");
    }
    instruction.op = stream.take();
    instruction.type = parse_type(stream, line);
    instruction.first = parse_operand(stream, line);
    stream.expect(line, ",");
    instruction.second = parse_operand(stream, line);
  } else if(stream.consume("cmp")) {
    instruction.kind = Instruction::IK_CMP;
    if(stream.eof()) {
      fail(line, "expected compare predicate");
    }
    instruction.op = stream.take();
    instruction.type = parse_type(stream, line);
    instruction.first = parse_operand(stream, line);
    stream.expect(line, ",");
    instruction.second = parse_operand(stream, line);
  } else if(stream.consume("convert")) {
    instruction.kind = Instruction::IK_CONVERT;
    if(stream.eof()) {
      fail(line, "expected conversion operator");
    }
    instruction.op = stream.take();
    instruction.type = parse_type(stream, line);
    instruction.source_type = parse_type(stream, line);
    instruction.first = parse_operand(stream, line);
  } else if(stream.consume("atomic_add_fetch")) {
    instruction.kind = Instruction::IK_ATOMIC_ADD_FETCH;
    instruction.type = parse_type(stream, line);
    instruction.first = parse_operand(stream, line);
    stream.expect(line, ",");
    instruction.second = parse_operand(stream, line);
    stream.expect(line, ",");
    parse_order_literal(instruction.third, "atomic order");
  } else if(stream.consume("atomic_exchange")) {
    instruction.kind = Instruction::IK_ATOMIC_EXCHANGE;
    instruction.type = parse_type(stream, line);
    instruction.first = parse_operand(stream, line);
    stream.expect(line, ",");
    instruction.second = parse_operand(stream, line);
    stream.expect(line, ",");
    parse_order_literal(instruction.third, "atomic order");
  } else if(stream.consume("atomic_compare_exchange")) {
    instruction.kind = Instruction::IK_ATOMIC_COMPARE_EXCHANGE;
    instruction.type = parse_type(stream, line);
    instruction.first = parse_operand(stream, line);
    stream.expect(line, ",");
    instruction.second = parse_operand(stream, line);
    stream.expect(line, ",");
    instruction.third = parse_operand(stream, line);
    stream.expect(line, ",");
    Operand success_order;
    parse_order_literal(success_order, "success atomic order");
    instruction.args.push_back(success_order);
    stream.expect(line, ",");
    Operand failure_order;
    parse_order_literal(failure_order, "failure atomic order");
    instruction.args.push_back(failure_order);
  } else if(stream.consume("va_arg")) {
    instruction.kind = Instruction::IK_VA_ARG;
    instruction.type = parse_type(stream, line);
    instruction.first = parse_operand(stream, line);
    if(instruction.first.kind == Operand::OP_LABEL) {
      fail(line, "va_arg requires value operand");
    }
  } else if(stream.consume("stack_alloc")) {
    instruction.kind = Instruction::IK_STACK_ALLOC;
    instruction.type.text = "ptr";
    instruction.first = parse_operand(stream, line);
    if(instruction.first.kind == Operand::OP_LABEL) {
      fail(line, "stack_alloc requires value operand");
    }
  } else if(stream.consume("call")) {
    instruction.kind = Instruction::IK_CALL;
    instruction.type = parse_type(stream, line);
    instruction.first = parse_operand(stream, line);
    instruction.args = parse_call_arguments(stream, line);
    parse_call_signature(instruction, stream, line);
    if(instruction.has_call_signature &&
       instruction.call_return_type.text != instruction.type.text) {
      fail(line, "call signature return type does not match call result type");
    }
    if(instruction.first.kind != Operand::OP_GLOBAL && !instruction.has_call_signature) {
      fail(line, "indirect call requires explicit call signature");
    }
  } else if(stream.consume("exception")) {
    instruction.kind = Instruction::IK_EXCEPTION;
    instruction.type = parse_type(stream, line);
    if(instruction.type.text == "void") {
      fail(line, "exception requires non-void type");
    }
  } else if(stream.consume("exception_selector")) {
    instruction.kind = Instruction::IK_EXCEPTION_SELECTOR;
    instruction.type = parse_type(stream, line);
    if(instruction.type.text != "i32" && instruction.type.text != "i64") {
      fail(line, "exception_selector requires i32 or i64 type");
    }
  } else {
    fail(line, "unknown instruction form");
  }

  return finish_instruction();
}

Program parse_program_lines(const vector<LineInfo> & lines)
{
  Program program;
  set<string> top_level_names;
  set<string> thread_local_global_names;
  set<string> alias_object_symbols;

  size_t pos = 0;
  while(pos < lines.size()) {
    const LineInfo & line = lines[pos];
    if(line.text.compare(0, 13, "alias object ") == 0) {
      ObjectAlias alias = parse_object_alias(line);
      if(!alias_object_symbols.insert(alias.object_symbol).second) {
        fail(line, "duplicate object alias " + alias.object_symbol);
      }
      program.object_aliases.push_back(alias);
      ++pos;
      continue;
    }
    if(line.text.compare(0, 15, "declare global ") == 0) {
      GlobalDeclaration global = parse_global_declaration(line);
      if(!top_level_names.insert(global.name).second) {
        fail(line, "duplicate top-level symbol " + global.name);
      }
      if(global.storage == GSM_THREAD_LOCAL) {
        thread_local_global_names.insert(global.name);
      }
      program.global_declarations.push_back(global);
      ++pos;
      continue;
    }
    if(line.text.compare(0, 17, "declare function ") == 0) {
      FunctionDeclaration function = parse_function_declaration(line);
      if(!top_level_names.insert(function.name).second) {
        fail(line, "duplicate top-level symbol " + function.name);
      }
      program.function_declarations.push_back(function);
      ++pos;
      continue;
    }
    if(line.text.compare(0, 7, "global ") == 0) {
      if(line.text.find('{') != string::npos) {
        GlobalDefinition global = parse_structured_global_header(line);
        if(!top_level_names.insert(global.name).second) {
          fail(line, "duplicate top-level symbol " + global.name);
        }
        if(global.storage == GSM_THREAD_LOCAL) {
          thread_local_global_names.insert(global.name);
        }
        ++pos;
        bool closed = false;
        while(pos < lines.size()) {
          const LineInfo & item_line = lines[pos];
          if(item_line.text == "}") {
            closed = true;
            ++pos;
            break;
          }
          global.data_items.push_back(parse_global_data_item(item_line));
          ++pos;
        }
        if(!closed) {
          fail(line, "structured global is missing closing '}'");
        }
        if(global.data_items.empty()) {
          fail(line, "structured global requires at least one data item");
        }
        program.globals.push_back(global);
      } else {
        GlobalDefinition global = parse_scalar_global(line);
        if(!top_level_names.insert(global.name).second) {
          fail(line, "duplicate top-level symbol " + global.name);
        }
        if(global.storage == GSM_THREAD_LOCAL) {
          thread_local_global_names.insert(global.name);
        }
        program.globals.push_back(global);
        ++pos;
      }
      continue;
    }
    if(line.text.compare(0, 9, "function ") == 0) {
      Function function = parse_function_header(line);
      if(!top_level_names.insert(function.name).second) {
        fail(line, "duplicate top-level symbol " + function.name);
      }
      ++pos;
      Block * current_block = 0;
      string current_block_label;
      set<string> slot_names;
      set<string> block_names;
      vector<string> block_targets;
      while(pos < lines.size()) {
        const LineInfo & body_line = lines[pos];
        if(body_line.text == "}") {
          ++pos;
          break;
        }
        if(body_line.text.compare(0, 5, "slot ") == 0) {
          if(current_block) {
            fail(body_line, "slot declarations must appear before blocks");
          }
          pair<string, LowType> slot = parse_slot(body_line);
          if(!slot_names.insert(slot.first).second) {
            fail(body_line, "duplicate slot " + slot.first);
          }
          function.slots.push_back(slot);
          ++pos;
          continue;
        }
        if(body_line.text.compare(0, 6, "block ") == 0) {
          if(current_block &&
             (current_block->instructions.empty() ||
              !is_terminator_instruction(current_block->instructions.back()))) {
            fail(body_line, "block " + current_block_label + " is missing terminator");
          }
          function.blocks.push_back(Block());
          function.blocks.back().label = parse_block_label(body_line);
          if(!block_names.insert(function.blocks.back().label).second) {
            fail(body_line, "duplicate block " + function.blocks.back().label);
          }
          current_block_label = function.blocks.back().label;
          current_block = &function.blocks.back();
          ++pos;
          continue;
        }
        if(!current_block) {
          fail(body_line, "instruction outside block");
        }
        if(!current_block->instructions.empty() &&
           is_terminator_instruction(current_block->instructions.back())) {
          fail(body_line, "block " + current_block_label + " has instruction after terminator");
        }
        Instruction instruction = parse_instruction(body_line);
        collect_block_targets(instruction, block_targets);
        current_block->instructions.push_back(instruction);
        ++pos;
      }
      if(function.blocks.empty()) {
        fail(line, "function requires at least one block");
      }
      if(function.blocks.back().instructions.empty() ||
         !is_terminator_instruction(function.blocks.back().instructions.back())) {
        fail(line, "block " + function.blocks.back().label + " is missing terminator");
      }
      for(size_t ti = 0; ti < block_targets.size(); ++ti) {
        if(block_names.count(block_targets[ti]) == 0) {
          fail(line, "function " + function.name +
                     " references undefined block target " + block_targets[ti]);
        }
      }
      program.functions.push_back(function);
      continue;
    }
    fail(line, "expected declaration, global, object alias, or function definition");
  }

  for(size_t i = 0; i < program.object_aliases.size(); ++i) {
    if(top_level_names.count(program.object_aliases[i].target) == 0) {
      LineInfo line;
      line.file = "<program>";
      line.line = 0;
      fail(line, "object alias target is not a top-level symbol " +
                 program.object_aliases[i].target);
    }
  }

  map<string, string> tls_wrapper_by_global;
  for(size_t i = 0; i < program.function_declarations.size(); ++i) {
    const FunctionDeclaration & function = program.function_declarations[i];
    if(function.metadata.tls_for_symbol.empty()) {
      continue;
    }
    if(function.metadata.tls_for_symbol == function.name) {
      LineInfo line;
      line.file = "<program>";
      line.line = 0;
      fail(line, "tls_for metadata cannot target the wrapper function itself " +
                 function.name);
    }
    if(thread_local_global_names.count(function.metadata.tls_for_symbol) == 0) {
      LineInfo line;
      line.file = "<program>";
      line.line = 0;
      fail(line, "tls_for metadata target is not a thread_local global " +
                 function.metadata.tls_for_symbol);
    }
    map<string, string>::const_iterator found =
        tls_wrapper_by_global.find(function.metadata.tls_for_symbol);
    if(found != tls_wrapper_by_global.end() && found->second != function.name) {
      LineInfo line;
      line.file = "<program>";
      line.line = 0;
      fail(line, "duplicate tls_for wrapper for " +
                 function.metadata.tls_for_symbol);
    }
    tls_wrapper_by_global[function.metadata.tls_for_symbol] = function.name;
  }
  for(size_t i = 0; i < program.functions.size(); ++i) {
    const Function & function = program.functions[i];
    if(function.metadata.tls_for_symbol.empty()) {
      continue;
    }
    if(function.metadata.tls_for_symbol == function.name) {
      LineInfo line;
      line.file = "<program>";
      line.line = 0;
      fail(line, "tls_for metadata cannot target the wrapper function itself " +
                 function.name);
    }
    if(thread_local_global_names.count(function.metadata.tls_for_symbol) == 0) {
      LineInfo line;
      line.file = "<program>";
      line.line = 0;
      fail(line, "tls_for metadata target is not a thread_local global " +
                 function.metadata.tls_for_symbol);
    }
    map<string, string>::const_iterator found =
        tls_wrapper_by_global.find(function.metadata.tls_for_symbol);
    if(found != tls_wrapper_by_global.end() && found->second != function.name) {
      LineInfo line;
      line.file = "<program>";
      line.line = 0;
      fail(line, "duplicate tls_for wrapper for " +
                 function.metadata.tls_for_symbol);
    }
    tls_wrapper_by_global[function.metadata.tls_for_symbol] = function.name;
  }

  for(size_t i = 0; i < program.global_declarations.size(); ++i) {
    maybe_add_exported_symbol(program,
                              program.global_declarations[i].name,
                              program.global_declarations[i].metadata);
  }
  for(size_t i = 0; i < program.function_declarations.size(); ++i) {
    maybe_add_exported_symbol(program,
                              program.function_declarations[i].name,
                              program.function_declarations[i].metadata);
  }
  for(size_t i = 0; i < program.globals.size(); ++i) {
    maybe_add_exported_symbol(program,
                              program.globals[i].name,
                              program.globals[i].metadata);
  }
  for(size_t i = 0; i < program.functions.size(); ++i) {
    maybe_add_exported_symbol(program,
                              program.functions[i].name,
                              program.functions[i].metadata);
  }

  canonicalize_program_export_metadata(program);
  return program;
}

}  // namespace

bool is_plain_identifier_text(const string & text)
{
  return is_plain_identifier_text_impl(text);
}

string lowir_debug_value_temp_name(const string & source_name,
                                   size_t version)
{
  if(!is_plain_identifier_text_impl(source_name)) {
    throw logic_error("invalid LowIR debug value source name");
  }
  ostringstream out;
  out << "%dbg_" << source_name << "__" << version;
  return out.str();
}

bool lowir_debug_value_source_name(const string & temp_name,
                                   string & source_name)
{
  if(temp_name.compare(0, 5, "%dbg_") != 0) {
    return false;
  }
  const size_t split = temp_name.rfind("__");
  if(split == string::npos || split <= 5) {
    return false;
  }
  for(size_t i = split + 2; i < temp_name.size(); ++i) {
    if(!isdigit(static_cast<unsigned char>(temp_name[i]))) {
      return false;
    }
  }
  const string candidate = temp_name.substr(5, split - 5);
  if(!is_plain_identifier_text_impl(candidate)) {
    return false;
  }
  source_name = candidate;
  return true;
}

Program parse_program(const vector<string> & srcfiles)
{
  return parse_program_lines(load_lines(srcfiles));
}

Program parse_program_text(const string & text, const string & label)
{
  istringstream in(text);
  return parse_program_lines(load_lines_from_stream(in, label));
}

LowType parse_type_text(const string & text, const string & label)
{
  LineInfo line;
  line.file = label;
  line.line = 1;
  line.text = text;
  const vector<string> tokens = lex_line(text);
  TokenStream stream(tokens);
  const LowType type = parse_type(stream, line);
  if(!stream.eof()) {
    fail(line, "unexpected trailing tokens in type");
  }
  return type;
}

Operand parse_operand_text(const string & text, const string & label)
{
  LineInfo line;
  line.file = label;
  line.line = 1;
  line.text = text;
  const vector<string> tokens = lex_line(text);
  TokenStream stream(tokens);
  const Operand operand = parse_operand(stream, line);
  if(!stream.eof()) {
    fail(line, "unexpected trailing tokens in operand");
  }
  return operand;
}

GlobalDefinition::DataItem parse_global_data_item_text(const string & text,
                                                       const string & label)
{
  LineInfo line;
  line.file = label;
  line.line = 1;
  line.text = text;
  return parse_global_data_item(line);
}

Instruction parse_instruction_text(const string & text, const string & label)
{
  LineInfo line;
  line.file = label;
  line.line = 1;
  line.text = text;
  return parse_instruction(line);
}

namespace {

string dump_operand(const Operand & operand)
{
  if(!operand.text.empty()) {
    return operand.text;
  }
  if(operand.kind == Operand::OP_INTEGER) {
    return to_string(operand.int_value);
  }
  if(operand.kind == Operand::OP_FLOAT) {
    ostringstream out;
    out << operand.float_value;
    if(operand.literal_type.text == "f32") {
      out << "f";
    } else if(operand.literal_type.text == "f80") {
      out << "L";
    }
    return out.str();
  }
  return string();
}

string dump_type(const LowType & type)
{
  return type.text;
}

string dump_call_args(const vector<Operand> & args)
{
  ostringstream out;
  out << "(";
  for(size_t i = 0; i < args.size(); ++i) {
    if(i != 0) {
      out << ", ";
    }
    out << dump_operand(args[i]);
  }
  out << ")";
  return out.str();
}

void dump_parameter_metadata(const ParameterMetadata & metadata, ostringstream & out);
void dump_index_metadata(IndexProjectionKind projection, ostringstream & out);
void dump_function_metadata(const FunctionBoundaryMetadata & boundary,
                            const SymbolMetadata & metadata,
                            ostringstream & out);

void dump_call_signature_suffix(const Instruction & instruction, ostringstream & out)
{
  if(!instruction.has_call_signature) {
    return;
  }
  out << " as (";
  for(size_t i = 0; i < instruction.call_params.size(); ++i) {
    if(i != 0) {
      out << ", ";
    }
    out << instruction.call_params[i].name << " : "
        << dump_type(instruction.call_params[i].type);
    dump_parameter_metadata(instruction.call_params[i].metadata, out);
  }
  out << ") -> " << dump_type(instruction.call_return_type);
  dump_function_metadata(instruction.call_boundary, SymbolMetadata(), out);
}

void dump_instruction_debug_location(const Instruction & instruction,
                                     ostringstream & out)
{
  if(!instruction.debug_location.present()) {
    return;
  }
  out << " !dbg(" << instruction.debug_location.file
      << ", " << instruction.debug_location.line
      << ", " << instruction.debug_location.column << ")";
}

void dump_function_debug_location(const Function & function,
                                  ostringstream & out)
{
  if(!function.debug_location.present()) {
    return;
  }
  out << " !dbg(" << function.debug_location.file
      << ", " << function.debug_location.line
      << ", " << function.debug_location.column << ")";
}

void dump_instruction_line(const Instruction & instruction, ostringstream & out)
{
  const auto dump_storage_span = [&out](const Instruction & inst)
      {
        out << inst.byte_count << "x" << inst.byte_alignment;
      };
  switch(instruction.kind) {
    case Instruction::IK_STORE:
      out << "store " << dump_type(instruction.type) << " "
          << dump_operand(instruction.first) << ", " << dump_operand(instruction.second);
      return;
    case Instruction::IK_ATOMIC_STORE:
      out << "atomic_store " << dump_type(instruction.type) << " "
          << dump_operand(instruction.first) << ", " << dump_operand(instruction.second)
          << ", " << dump_operand(instruction.third);
      return;
    case Instruction::IK_CALL:
      if(instruction.dest.empty() && instruction.call_returns_void) {
        out << "call void " << dump_operand(instruction.first)
            << dump_call_args(instruction.args);
        dump_call_signature_suffix(instruction, out);
        return;
      }
      break;
    case Instruction::IK_ATOMIC_THREAD_FENCE:
      out << "atomic_thread_fence " << dump_operand(instruction.first);
      return;
    case Instruction::IK_ATOMIC_SIGNAL_FENCE:
      out << "atomic_signal_fence " << dump_operand(instruction.first);
      return;
    case Instruction::IK_VA_START:
      out << "va_start " << dump_operand(instruction.first);
      return;
    case Instruction::IK_COPYOBJ:
      out << "copyobj ";
      dump_storage_span(instruction);
      out << " "
          << dump_operand(instruction.first) << ", " << dump_operand(instruction.second);
      return;
    case Instruction::IK_ZEROINIT:
      out << "zeroinit ";
      dump_storage_span(instruction);
      out << " "
          << dump_operand(instruction.first);
      return;
    case Instruction::IK_EH_TRY:
      out << "eh_try " << dump_operand(instruction.first);
      return;
    case Instruction::IK_EH_CLEANUP:
      out << "eh_cleanup " << dump_operand(instruction.first);
      return;
    case Instruction::IK_EH_CLEANUP_CLAUSE:
      out << "eh_cleanup";
      return;
    case Instruction::IK_EH_CATCH:
      out << "eh_catch " << dump_operand(instruction.first);
      if(instruction.has_eh_selector) {
        out << ", " << instruction.eh_selector;
      }
      return;
    case Instruction::IK_EH_FILTER:
      out << "eh_filter";
      for(size_t i = 0; i < instruction.args.size(); ++i) {
        out << (i == 0 ? " " : ", ") << dump_operand(instruction.args[i]);
      }
      return;
    case Instruction::IK_EH_CATCH_ALL:
      out << "eh_catch_all";
      if(instruction.has_eh_selector) {
        out << ", " << instruction.eh_selector;
      }
      return;
    case Instruction::IK_EH_END:
      out << "eh_end";
      return;
    case Instruction::IK_THROW:
      out << "throw " << dump_type(instruction.type) << " "
          << dump_operand(instruction.first);
      return;
    case Instruction::IK_EXCEPTION:
      out << instruction.dest << " = exception " << dump_type(instruction.type);
      return;
    case Instruction::IK_EXCEPTION_SELECTOR:
      out << instruction.dest << " = exception_selector " << dump_type(instruction.type);
      return;
    case Instruction::IK_RESUME:
      out << "resume";
      return;
    case Instruction::IK_JUMP:
      out << "jump " << dump_operand(instruction.first);
      return;
    case Instruction::IK_BRANCH:
      out << "branch " << dump_operand(instruction.first) << ", "
          << dump_operand(instruction.second) << ", " << dump_operand(instruction.third);
      return;
    case Instruction::IK_SWITCH:
      out << "switch " << dump_operand(instruction.first) << ", "
          << dump_operand(instruction.second);
      for(size_t i = 0; i + 1 < instruction.args.size(); i += 2) {
        out << ", " << dump_operand(instruction.args[i]) << ":"
            << dump_operand(instruction.args[i + 1]);
      }
      return;
    case Instruction::IK_RETURN:
      out << "return " << dump_type(instruction.type);
      if(instruction.type.text != "void") {
        out << " " << dump_operand(instruction.first);
      }
      return;
    default:
      break;
  }

  out << instruction.dest << " = ";
  switch(instruction.kind) {
    case Instruction::IK_CONST:
      out << "const " << dump_type(instruction.type) << " " << dump_operand(instruction.first);
      return;
    case Instruction::IK_COPY:
      out << "copy " << dump_type(instruction.type) << " " << dump_operand(instruction.first);
      return;
    case Instruction::IK_ADDR:
      out << "addr " << dump_operand(instruction.first);
      return;
    case Instruction::IK_LOAD:
      out << "load " << dump_type(instruction.type) << " " << dump_operand(instruction.first);
      return;
    case Instruction::IK_ATOMIC_LOAD:
      out << "atomic_load " << dump_type(instruction.type) << " "
          << dump_operand(instruction.first) << ", " << dump_operand(instruction.second);
      return;
    case Instruction::IK_INDEX:
      out << "index " << instruction.op;
      dump_index_metadata(instruction.index_projection, out);
      out << " " << dump_operand(instruction.first)
          << ", " << dump_operand(instruction.second);
      return;
    case Instruction::IK_UNARY:
      out << "unary " << instruction.op << " " << dump_type(instruction.type)
          << " " << dump_operand(instruction.first);
      return;
    case Instruction::IK_BINARY:
      out << "binary " << instruction.op << " " << dump_type(instruction.type)
          << " " << dump_operand(instruction.first) << ", "
          << dump_operand(instruction.second);
      return;
    case Instruction::IK_CMP:
      out << "cmp " << instruction.op << " " << dump_type(instruction.type)
          << " " << dump_operand(instruction.first) << ", "
          << dump_operand(instruction.second);
      return;
    case Instruction::IK_CONVERT:
      out << "convert " << instruction.op << " "
          << dump_type(instruction.type) << " "
          << dump_type(instruction.source_type) << " "
          << dump_operand(instruction.first);
      return;
    case Instruction::IK_ATOMIC_ADD_FETCH:
      out << "atomic_add_fetch " << dump_type(instruction.type)
          << " " << dump_operand(instruction.first) << ", "
          << dump_operand(instruction.second) << ", "
          << dump_operand(instruction.third);
      return;
    case Instruction::IK_ATOMIC_EXCHANGE:
      out << "atomic_exchange " << dump_type(instruction.type)
          << " " << dump_operand(instruction.first) << ", "
          << dump_operand(instruction.second) << ", "
          << dump_operand(instruction.third);
      return;
    case Instruction::IK_ATOMIC_COMPARE_EXCHANGE:
      if(instruction.args.size() != 2) {
        throw logic_error("atomic_compare_exchange requires two order operands");
      }
      out << "atomic_compare_exchange " << dump_type(instruction.type)
          << " " << dump_operand(instruction.first) << ", "
          << dump_operand(instruction.second) << ", "
          << dump_operand(instruction.third) << ", "
          << dump_operand(instruction.args[0]) << ", "
          << dump_operand(instruction.args[1]);
      return;
    case Instruction::IK_VA_ARG:
      out << "va_arg " << dump_type(instruction.type) << " "
          << dump_operand(instruction.first);
      return;
    case Instruction::IK_STACK_ALLOC:
      out << "stack_alloc " << dump_operand(instruction.first);
      return;
    case Instruction::IK_CALL:
      out << "call " << dump_type(instruction.type) << " "
          << dump_operand(instruction.first) << dump_call_args(instruction.args);
      dump_call_signature_suffix(instruction, out);
      return;
    default:
      throw logic_error("unsupported LowIR dump instruction kind");
  }
}

void dump_global_metadata(GlobalStorageMode storage,
                          const SymbolMetadata & metadata,
                          ostringstream & out)
{
  const bool has_storage = storage != GSM_DEFAULT;
  const bool has_role = metadata.role != SR_NONE;
  const bool has_linkage = metadata.linkage != LLM_DEFAULT;
  const bool has_binding = metadata.binding != SBM_DEFAULT;
  const bool has_object = !metadata.object_symbol.empty();
  const bool has_keep_alias = metadata.keep_internal_alias;
  const bool has_prefer_local = metadata.prefer_local_object_binding;
  const bool has_object_root = metadata.object_output_root;
  const bool has_section_segment = !metadata.section_segment.empty();
  const bool has_section_name = !metadata.section_name.empty();
  if(!has_storage && !has_role && !has_linkage && !has_binding &&
     !has_object && !has_keep_alias && !has_prefer_local && !has_object_root &&
     !has_section_segment && !has_section_name) {
    return;
  }
  out << " [";
  bool need_comma = false;
  if(has_storage) {
    out << "storage=" << global_storage_text(storage);
    need_comma = true;
  }
  if(has_role) {
    if(need_comma) {
      out << ", ";
    }
    out << "role=" << symbol_role_text(metadata.role);
    need_comma = true;
  }
  if(has_linkage) {
    if(need_comma) {
      out << ", ";
    }
    out << "linkage=" << language_linkage_text(metadata.linkage);
    need_comma = true;
  }
  if(has_binding) {
    if(need_comma) {
      out << ", ";
    }
    out << "binding=" << symbol_binding_text(metadata.binding);
    need_comma = true;
  }
  if(has_object) {
    if(need_comma) {
      out << ", ";
    }
    out << "object=" << metadata.object_symbol;
    need_comma = true;
  }
  if(has_keep_alias) {
    if(need_comma) {
      out << ", ";
    }
    out << "keep_alias=yes";
    need_comma = true;
  }
  if(has_prefer_local) {
    if(need_comma) {
      out << ", ";
    }
    out << "prefer_local=yes";
    need_comma = true;
  }
  if(has_object_root) {
    if(need_comma) {
      out << ", ";
    }
    out << "object_root=yes";
    need_comma = true;
  }
  if(has_section_segment) {
    if(need_comma) {
      out << ", ";
    }
    out << "section_segment=" << metadata.section_segment;
    need_comma = true;
  }
  if(has_section_name) {
    if(need_comma) {
      out << ", ";
    }
    out << "section_name=" << metadata.section_name;
  }
  out << "]";
}

void dump_parameter_metadata(const ParameterMetadata & metadata, ostringstream & out)
{
  const bool has_pass = metadata.passing != PPM_DIRECT;
  const bool has_capture = metadata.capture != PCM_DEFAULT;
  const bool has_access = metadata.access != PAM_DEFAULT;
  const bool has_alias = metadata.alias != PALM_DEFAULT;
  if(!has_pass && !has_capture && !has_access && !has_alias) {
    return;
  }
  out << " [";
  bool need_comma = false;
  if(has_pass) {
    out << "pass=" << param_passing_mode_text(metadata.passing);
    need_comma = true;
  }
  if(has_capture) {
    if(need_comma) {
      out << ", ";
    }
    out << "capture=" << param_capture_mode_text(metadata.capture);
    need_comma = true;
  }
  if(has_access) {
    if(need_comma) {
      out << ", ";
    }
    out << "access=" << param_access_mode_text(metadata.access);
    need_comma = true;
  }
  if(has_alias) {
    if(need_comma) {
      out << ", ";
    }
    out << "alias=" << param_alias_mode_text(metadata.alias);
  }
  out << "]";
}

void dump_index_metadata(IndexProjectionKind projection, ostringstream & out)
{
  if(projection == IPK_NONE) {
    return;
  }
  out << " [projection=" << index_projection_text(projection) << "]";
}

void dump_function_metadata(const FunctionBoundaryMetadata & boundary,
                            const SymbolMetadata & metadata,
                            ostringstream & out)
{
  const bool has_arity = boundary.arity != CAM_FIXED;
  const bool has_effects = boundary.effects != CFXM_DEFAULT;
  const bool has_unwind = boundary.unwind != CUM_DEFAULT;
  const bool has_return = boundary.returns != CRM_DEFAULT;
  const bool has_role = metadata.role != SR_NONE;
  const bool has_linkage = metadata.linkage != LLM_DEFAULT;
  const bool has_binding = metadata.binding != SBM_DEFAULT;
  const bool has_object = !metadata.object_symbol.empty();
  const bool has_tls_for = !metadata.tls_for_symbol.empty();
  const bool has_keep_alias = metadata.keep_internal_alias;
  const bool has_prefer_local = metadata.prefer_local_object_binding;
  const bool has_object_root = metadata.object_output_root;
  const bool has_trivial_lifecycle = metadata.object_trivial_lifecycle;
  const bool has_force_inline = metadata.force_inline;
  if(!has_arity && !has_effects && !has_unwind && !has_return &&
     !has_role && !has_linkage && !has_binding &&
     !has_object && !has_tls_for && !has_keep_alias && !has_prefer_local &&
     !has_object_root && !has_trivial_lifecycle && !has_force_inline) {
    return;
  }
  out << " [";
  bool need_comma = false;
  if(has_arity) {
    out << "arity=" << call_arity_mode_text(boundary.arity);
    need_comma = true;
  }
  if(has_effects) {
    if(need_comma) {
      out << ", ";
    }
    out << "effects=" << call_effects_mode_text(boundary.effects);
    need_comma = true;
  }
  if(has_unwind) {
    if(need_comma) {
      out << ", ";
    }
    out << "unwind=" << call_unwind_mode_text(boundary.unwind);
    need_comma = true;
  }
  if(has_return) {
    if(need_comma) {
      out << ", ";
    }
    out << "return=" << call_return_mode_text(boundary.returns);
    need_comma = true;
  }
  if(has_role) {
    if(need_comma) {
      out << ", ";
    }
    out << "role=" << symbol_role_text(metadata.role);
    need_comma = true;
  }
  if(has_linkage) {
    if(need_comma) {
      out << ", ";
    }
    out << "linkage=" << language_linkage_text(metadata.linkage);
    need_comma = true;
  }
  if(has_binding) {
    if(need_comma) {
      out << ", ";
    }
    out << "binding=" << symbol_binding_text(metadata.binding);
    need_comma = true;
  }
  if(has_object) {
    if(need_comma) {
      out << ", ";
    }
    out << "object=" << metadata.object_symbol;
    need_comma = true;
  }
  if(has_tls_for) {
    if(need_comma) {
      out << ", ";
    }
    out << "tls_for=" << metadata.tls_for_symbol;
    need_comma = true;
  }
  if(has_keep_alias) {
    if(need_comma) {
      out << ", ";
    }
    out << "keep_alias=yes";
    need_comma = true;
  }
  if(has_prefer_local) {
    if(need_comma) {
      out << ", ";
    }
    out << "prefer_local=yes";
    need_comma = true;
  }
  if(has_object_root) {
    if(need_comma) {
      out << ", ";
    }
    out << "object_root=yes";
    need_comma = true;
  }
  if(has_trivial_lifecycle) {
    if(need_comma) {
      out << ", ";
    }
    out << "trivial_lifecycle=yes";
    need_comma = true;
  }
  if(has_force_inline) {
    if(need_comma) {
      out << ", ";
    }
    out << "force_inline=yes";
  }
  out << "]";
}

map<string, ir_model::ExportedSymbol> export_map(
    const vector<ir_model::ExportedSymbol> & exported_symbols)
{
  map<string, ir_model::ExportedSymbol> out;
  for(size_t i = 0; i < exported_symbols.size(); ++i) {
    out[exported_symbols[i].internal_symbol] = exported_symbols[i];
  }
  return out;
}

bool export_uses_default_object_name(const string & internal_name,
                                     const ir_model::ExportedSymbol & identity,
                                     SymbolBindingMode binding)
{
  const ir_model::ExportedSymbol synthesized =
      synthesized_export_identity(internal_name, binding);
  return identity.object_symbol == synthesized.object_symbol;
}

SymbolMetadata merged_symbol_metadata_for_dump(
    const string & internal_name,
    const SymbolMetadata & base,
    const map<string, ir_model::ExportedSymbol> & exports)
{
  SymbolMetadata out = base;
  map<string, ir_model::ExportedSymbol>::const_iterator found =
      exports.find(internal_name);
  if(found == exports.end()) {
    return out;
  }
  out.binding = binding_from_ir_symbol_linkage(found->second.linkage);
  if(!export_uses_default_object_name(internal_name, found->second, out.binding)) {
    out.object_symbol = found->second.object_symbol;
  }
  out.keep_internal_alias = found->second.keep_internal_alias;
  out.prefer_local_object_binding = found->second.prefer_local_object_binding;
  return out;
}

}  // namespace

void canonicalize_program_export_metadata(Program & program)
{
  const map<string, ir_model::ExportedSymbol> exports =
      export_map(program.exported_symbols);
  for(size_t i = 0; i < program.global_declarations.size(); ++i) {
    program.global_declarations[i].metadata =
        merged_symbol_metadata_for_dump(program.global_declarations[i].name,
                                        program.global_declarations[i].metadata,
                                        exports);
  }
  for(size_t i = 0; i < program.function_declarations.size(); ++i) {
    program.function_declarations[i].metadata =
        merged_symbol_metadata_for_dump(program.function_declarations[i].name,
                                        program.function_declarations[i].metadata,
                                        exports);
  }
  for(size_t i = 0; i < program.globals.size(); ++i) {
    program.globals[i].metadata =
        merged_symbol_metadata_for_dump(program.globals[i].name,
                                        program.globals[i].metadata,
                                        exports);
  }
  for(size_t i = 0; i < program.functions.size(); ++i) {
    program.functions[i].metadata =
        merged_symbol_metadata_for_dump(program.functions[i].name,
                                        program.functions[i].metadata,
                                        exports);
  }
}

string dump_program(const Program & program)
{
  ostringstream out;
  const map<string, ir_model::ExportedSymbol> exports =
      export_map(program.exported_symbols);
  for(size_t i = 0; i < program.global_declarations.size(); ++i) {
    const GlobalDeclaration & global = program.global_declarations[i];
    const SymbolMetadata metadata =
        merged_symbol_metadata_for_dump(global.name, global.metadata, exports);
    out << "declare global " << global.name;
    if(global.has_type) {
      out << " : " << dump_type(global.type);
    }
    dump_global_metadata(global.storage, metadata, out);
    out << "\n";
  }
  for(size_t i = 0; i < program.function_declarations.size(); ++i) {
    const FunctionDeclaration & function = program.function_declarations[i];
    const SymbolMetadata metadata =
        merged_symbol_metadata_for_dump(function.name, function.metadata, exports);
    out << "declare function " << function.name << "(";
    for(size_t pi = 0; pi < function.params.size(); ++pi) {
      if(pi != 0) {
        out << ", ";
      }
      out << function.params[pi].name << " : " << dump_type(function.params[pi].type);
      dump_parameter_metadata(function.params[pi].metadata, out);
    }
    out << ") -> " << dump_type(function.return_type);
    dump_function_metadata(function.boundary, metadata, out);
    out << "\n";
  }
  if((!program.global_declarations.empty() || !program.function_declarations.empty()) &&
     (!program.globals.empty() || !program.functions.empty())) {
    out << "\n";
  }
  for(size_t i = 0; i < program.globals.size(); ++i) {
    const GlobalDefinition & global = program.globals[i];
    const SymbolMetadata metadata =
        merged_symbol_metadata_for_dump(global.name, global.metadata, exports);
    if(global.structured) {
      out << "global " << global.name;
      dump_global_metadata(global.storage, metadata, out);
      out << " = {\n";
      for(size_t di = 0; di < global.data_items.size(); ++di) {
        const GlobalDefinition::DataItem & item = global.data_items[di];
        out << "  ";
        if(item.kind == GlobalDefinition::DataItem::ITEM_ZERO) {
          out << "zero " << item.zero_bytes;
        } else if(item.kind == GlobalDefinition::DataItem::ITEM_ADDR) {
          out << "ptr addr " << item.symbol;
          if(item.addr_addend != 0) {
            out << (item.addr_addend > 0 ? " + " : " - ")
                << (item.addr_addend > 0 ? item.addr_addend : -item.addr_addend);
          }
        } else {
          out << dump_type(item.type) << " " << dump_operand(item.literal_operand);
        }
        out << "\n";
      }
      out << "}";
    } else {
      out << "global " << global.name;
      out << " : " << dump_type(global.type);
      dump_global_metadata(global.storage, metadata, out);
      out << " = ";
      if(global.init_kind == GlobalDefinition::INIT_ZERO) {
        out << "zero";
      } else if(global.init_kind == GlobalDefinition::INIT_ADDR) {
        out << "addr " << dump_operand(global.init_operand);
        if(global.addr_addend != 0) {
          out << (global.addr_addend > 0 ? " + " : " - ")
              << (global.addr_addend > 0 ? global.addr_addend : -global.addr_addend);
        }
      } else {
        out << dump_operand(global.init_operand);
      }
    }
    out << "\n";
  }
  if(!program.globals.empty() && !program.functions.empty()) {
    out << "\n";
  }
  for(size_t i = 0; i < program.functions.size(); ++i) {
    const Function & function = program.functions[i];
    const SymbolMetadata metadata =
        merged_symbol_metadata_for_dump(function.name, function.metadata, exports);
    out << "function " << function.name << "(";
    for(size_t pi = 0; pi < function.params.size(); ++pi) {
      if(pi != 0) {
        out << ", ";
      }
      out << function.params[pi].name << " : " << dump_type(function.params[pi].type);
      dump_parameter_metadata(function.params[pi].metadata, out);
    }
    out << ") -> " << dump_type(function.return_type);
    dump_function_metadata(function.boundary, metadata, out);
    dump_function_debug_location(function, out);
    out << " {\n";
    for(size_t si = 0; si < function.slots.size(); ++si) {
      out << "  slot " << function.slots[si].first << " : "
          << dump_type(function.slots[si].second) << "\n";
    }
    if(!function.slots.empty()) {
      out << "\n";
    }
    for(size_t bi = 0; bi < function.blocks.size(); ++bi) {
      out << "  block " << function.blocks[bi].label << ":\n";
      for(size_t ii = 0; ii < function.blocks[bi].instructions.size(); ++ii) {
        out << "    ";
        dump_instruction_line(function.blocks[bi].instructions[ii], out);
        dump_instruction_debug_location(function.blocks[bi].instructions[ii], out);
        out << "\n";
      }
      if(bi + 1 != function.blocks.size()) {
        out << "\n";
      }
    }
    out << "}";
    if(i + 1 != program.functions.size()) {
      out << "\n";
    }
  }
  if(!program.object_aliases.empty()) {
    if(out.tellp() > 0) {
      out << "\n";
    }
    for(size_t i = 0; i < program.object_aliases.size(); ++i) {
      out << "alias object " << program.object_aliases[i].object_symbol
          << " = " << program.object_aliases[i].target << "\n";
    }
  }
  string text = out.str();
  if(!text.empty() && text[text.size() - 1] != '\n') {
    text += '\n';
  }
  return text;
}

string mangle_name(const string & qualified)
{
  string text = qualified;
  if(!text.empty() && text[0] == '@') {
    text.erase(text.begin());
  }
  if(!text.empty() && text[0] == '^') {
    text.erase(text.begin());
  }
  return symbol_linkage::mangle_symbol_name(text);
}

size_t type_size(const LowType & type)
{
  size_t object_size = 0;
  size_t object_alignment = 0;
  if(parse_object_type_text(type.text, object_size, object_alignment)) {
    (void)object_alignment;
    return object_size;
  }
  if(type.text == "void") {
    return 0;
  }
  if(type.text == "i1" || type.text == "i8" || type.text == "u8") {
    return 1;
  }
  if(type.text == "i16" || type.text == "u16") {
    return 2;
  }
  if(type.text == "i32" || type.text == "u32") {
    return 4;
  }
  if(type.text == "f32") {
    return 4;
  }
  if(type.text == "f64") {
    return 8;
  }
  if(type.text == "f80") {
    return 16;
  }
  if(type.text == "i64" || type.text == "ptr") {
    return 8;
  }
  if(type.text == "i128" || type.text == "u128") {
    return 16;
  }
  throw ParseError("unsupported type size " + type.text);
}

size_t type_alignment(const LowType & type)
{
  size_t object_size = 0;
  size_t object_alignment = 0;
  if(parse_object_type_text(type.text, object_size, object_alignment)) {
    return object_alignment;
  }
  if(type.text == "f80") {
    return 16;
  }
  if(type.text == "i128" || type.text == "u128") {
    return 16;
  }
  const size_t size = type_size(type);
  return min<size_t>(8, max<size_t>(1, size));
}

bool is_object_type(const LowType & type)
{
  size_t object_size = 0;
  size_t object_alignment = 0;
  return parse_object_type_text(type.text, object_size, object_alignment);
}

bool is_sign_extended_integer_type(const LowType & type)
{
  return type.text == "i8" || type.text == "i16" || type.text == "i32";
}

string instruction_result_storage_type(const Instruction & instruction)
{
  if(instruction.kind == Instruction::IK_CMP ||
     instruction.kind == Instruction::IK_ATOMIC_COMPARE_EXCHANGE) {
    return "i64";
  }
  if(instruction.kind == Instruction::IK_BINARY &&
     instruction.op == "sub" &&
     instruction.type.text == "ptr") {
    return "i64";
  }
  return instruction.type.text;
}

const char * symbol_role_text(SymbolRole role)
{
  switch(role) {
    case SR_NONE: return "none";
    case SR_ENTRY: return "entry";
    case SR_INIT: return "init";
    case SR_FINI: return "fini";
    case SR_EH_TOP: return "eh_top";
    case SR_EH_VALUE: return "eh_value";
    case SR_EH_TYPE: return "eh_type";
    case SR_EH_UNHANDLED: return "eh_unhandled";
    case SR_EH_ALLOCATE_EXCEPTION: return "eh_allocate_exception";
    case SR_EH_BEGIN_CATCH: return "eh_begin_catch";
    case SR_EH_CALL_UNEXPECTED: return "eh_call_unexpected";
    case SR_EH_CURRENT_EXCEPTION_TYPE: return "eh_current_exception_type";
    case SR_EH_END_CATCH: return "eh_end_catch";
    case SR_EH_RETHROW: return "eh_rethrow";
    case SR_EH_THROW: return "eh_throw";
    case SR_EH_PERSONALITY: return "eh_personality";
    case SR_EH_RESUME: return "eh_resume";
  }
  return "unknown";
}

const char * language_linkage_text(LanguageLinkageMode linkage)
{
  switch(linkage) {
  case LLM_DEFAULT: return "default";
  case LLM_C: return "c";
  case LLM_CPP: return "cpp";
  }
  return "unknown";
}

const char * symbol_binding_text(SymbolBindingMode binding)
{
  switch(binding) {
  case SBM_DEFAULT: return "default";
  case SBM_INTERNAL: return "internal";
  case SBM_STRONG: return "strong";
  case SBM_WEAK: return "weak";
  }
  return "unknown";
}

const char * global_storage_text(GlobalStorageMode storage)
{
  switch(storage) {
  case GSM_DEFAULT: return "default";
  case GSM_WRITABLE: return "writable";
  case GSM_READONLY: return "readonly";
  case GSM_THREAD_LOCAL: return "thread_local";
  }
  return "unknown";
}

const char * index_projection_text(IndexProjectionKind kind)
{
  switch(kind) {
  case IPK_NONE: return "none";
  case IPK_ARRAY_ELEMENT: return "array_element";
  case IPK_FIELD: return "field";
  case IPK_BASE_SUBOBJECT: return "base_subobject";
  case IPK_REFERENCE_FIELD: return "reference_field";
  }
  return "unknown";
}

const char * param_passing_mode_text(ParamPassingMode mode)
{
  switch(mode) {
  case PPM_DIRECT: return "direct";
  case PPM_INDIRECT_RESULT: return "indirect_result";
  case PPM_BY_ADDRESS: return "by_address";
  case PPM_REFERENCE: return "reference";
  case PPM_DECAY: return "decay";
  }
  return "unknown";
}

const char * param_capture_mode_text(ParamCaptureMode mode)
{
  switch(mode) {
  case PCM_DEFAULT: return "default";
  case PCM_NOCAPTURE: return "nocapture";
  case PCM_MAYCAPTURE: return "maycapture";
  }
  return "unknown";
}

const char * param_access_mode_text(ParamAccessMode mode)
{
  switch(mode) {
  case PAM_DEFAULT: return "default";
  case PAM_NONE: return "none";
  case PAM_READ: return "read";
  case PAM_WRITE: return "write";
  case PAM_READWRITE: return "readwrite";
  }
  return "unknown";
}

const char * param_alias_mode_text(ParamAliasMode mode)
{
  switch(mode) {
  case PALM_DEFAULT: return "default";
  case PALM_NOALIAS: return "noalias";
  }
  return "unknown";
}

const char * call_arity_mode_text(CallArityMode mode)
{
  switch(mode) {
  case CAM_FIXED: return "fixed";
  case CAM_VARIADIC: return "variadic";
  case CAM_PROTOTYPE_RELAXED: return "prototype_relaxed";
  }
  return "unknown";
}

const char * call_effects_mode_text(CallEffectsMode mode)
{
  switch(mode) {
  case CFXM_DEFAULT: return "default";
  case CFXM_READNONE: return "readnone";
  case CFXM_READONLY: return "readonly";
  case CFXM_READWRITE: return "readwrite";
  }
  return "unknown";
}

const char * call_unwind_mode_text(CallUnwindMode mode)
{
  switch(mode) {
  case CUM_DEFAULT: return "default";
  case CUM_MAY: return "may";
  case CUM_NO: return "no";
  }
  return "unknown";
}

const char * call_return_mode_text(CallReturnMode mode)
{
  switch(mode) {
  case CRM_DEFAULT: return "default";
  case CRM_RETURNS: return "returns";
  case CRM_NORETURN: return "noreturn";
  }
  return "unknown";
}

bool is_function_symbol_role(SymbolRole role)
{
  return role == SR_ENTRY ||
         role == SR_INIT ||
         role == SR_FINI ||
         role == SR_EH_UNHANDLED ||
         role == SR_EH_ALLOCATE_EXCEPTION ||
         role == SR_EH_BEGIN_CATCH ||
         role == SR_EH_CALL_UNEXPECTED ||
         role == SR_EH_CURRENT_EXCEPTION_TYPE ||
         role == SR_EH_END_CATCH ||
         role == SR_EH_RETHROW ||
         role == SR_EH_THROW ||
         role == SR_EH_PERSONALITY ||
         role == SR_EH_RESUME;
}

bool is_global_symbol_role(SymbolRole role)
{
  return role == SR_EH_TOP ||
         role == SR_EH_VALUE ||
         role == SR_EH_TYPE;
}

bool is_host_eh_symbol_role(SymbolRole role)
{
  return role == SR_EH_ALLOCATE_EXCEPTION ||
         role == SR_EH_BEGIN_CATCH ||
         role == SR_EH_CALL_UNEXPECTED ||
         role == SR_EH_CURRENT_EXCEPTION_TYPE ||
         role == SR_EH_END_CATCH ||
         role == SR_EH_RETHROW ||
         role == SR_EH_THROW ||
         role == SR_EH_PERSONALITY ||
         role == SR_EH_RESUME;
}

}  // namespace lowir_internal

namespace lowir_model {

LowirProgram parse_lowir_program_text(const std::string & text,
                                      const std::string & source_name)
{
  return lowir_internal::parse_program_text(text, source_name);
}

LowirProgram parse_lowir_program_files(const std::vector<std::string> & paths)
{
  return lowir_internal::parse_program(paths);
}

std::string serialize_lowir_program(const LowirProgram & program)
{
  return lowir_internal::dump_program(program);
}

void write_lowir_program_file(const std::string & path,
                              const LowirProgram & program)
{
  std::ofstream out(path.c_str());
  if(!out) {
    throw ParseError("unable to open LowIR output file: " + path);
  }
  out << serialize_lowir_program(program);
}

}  // namespace lowir_model

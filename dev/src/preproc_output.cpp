#include "preproc_output.h"

#include <ctime>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

#include "posttokenizer.h"
#include "preprocessor.h"
#include "file_timing.h"

namespace {

struct PostTokenOutputStream : IPostTokenOutputStream
{
  explicit PostTokenOutputStream(ostream & out) : out(out)
  {
    buffer.reserve(1 << 20);
  }

  ~PostTokenOutputStream()
  {
    flush();
  }

  ostream & out;
  string buffer;
  bool emitted_first_token = false;

  void note_emit()
  {
    if(!emitted_first_token) {
      emitted_first_token = true;
      file_timing::startup_mark("preproc.first_post_token_emit");
    }
  }

  void flush()
  {
    if(buffer.empty()) {
      return;
    }
    out.write(buffer.data(), static_cast<streamsize>(buffer.size()));
    buffer.clear();
  }

  void append(const char * text)
  {
    buffer += text;
    flush_if_large();
  }

  void append(const string & text)
  {
    buffer += text;
    flush_if_large();
  }

  void append_char(char value)
  {
    buffer.push_back(value);
    flush_if_large();
  }

  void append_size(size_t value)
  {
    buffer += to_string(value);
    flush_if_large();
  }

  void append_hex(const void * data, size_t nbytes)
  {
    buffer += hex_dump(data, nbytes);
    flush_if_large();
  }

  void flush_if_large()
  {
    if(buffer.size() >= (1 << 20)) {
      flush();
    }
  }

  void emit_invalid(const string & source) override
  {
    throw logic_error(string("Invalid token in sequence: ") + source);
  }

  void emit_simple(const string & source, ETokenType token_type) override
  {
    note_emit();
    append("simple ");
    append(source);
    append_char(' ');
    append(token_type_to_string(token_type));
    append_char('\n');
  }

  void emit_identifier(const string & source) override
  {
    note_emit();
    append("identifier ");
    append(source);
    append_char('\n');
  }

  void emit_literal(const string & source,
                    EFundamentalType type,
                    const void * data,
                    size_t nbytes) override
  {
    note_emit();
    append("literal ");
    append(source);
    append_char(' ');
    append(type_to_string(type));
    append_char(' ');
    append_hex(data, nbytes);
    append_char('\n');
  }

  void emit_literal_array(const string & source,
                          size_t num_elements,
                          EFundamentalType type,
                          const void * data,
                          size_t nbytes) override
  {
    note_emit();
    append("literal ");
    append(source);
    append(" array of ");
    append_size(num_elements);
    append_char(' ');
    append(type_to_string(type));
    append_char(' ');
    append_hex(data, nbytes);
    append_char('\n');
  }

  void emit_user_defined_literal_character(const string & source,
                                           const string & ud_suffix,
                                           EFundamentalType type,
                                           const void * data,
                                           size_t nbytes) override
  {
    note_emit();
    append("user-defined-literal ");
    append(source);
    append_char(' ');
    append(ud_suffix);
    append(" character ");
    append(type_to_string(type));
    append_char(' ');
    append_hex(data, nbytes);
    append_char('\n');
  }

  void emit_user_defined_literal_string_array(const string & source,
                                              const string & ud_suffix,
                                              size_t num_elements,
                                              EFundamentalType type,
                                              const void * data,
                                              size_t nbytes) override
  {
    note_emit();
    append("user-defined-literal ");
    append(source);
    append_char(' ');
    append(ud_suffix);
    append(" string array of ");
    append_size(num_elements);
    append_char(' ');
    append(type_to_string(type));
    append_char(' ');
    append_hex(data, nbytes);
    append_char('\n');
  }

  void emit_user_defined_literal_integer(const string & source,
                                         const string & ud_suffix,
                                         const string & prefix) override
  {
    note_emit();
    append("user-defined-literal ");
    append(source);
    append_char(' ');
    append(ud_suffix);
    append(" integer ");
    append(prefix);
    append_char('\n');
  }

  void emit_user_defined_literal_floating(const string & source,
                                          const string & ud_suffix,
                                          const string & prefix) override
  {
    note_emit();
    append("user-defined-literal ");
    append(source);
    append_char(' ');
    append(ud_suffix);
    append(" floating ");
    append(prefix);
    append_char('\n');
  }

  void emit_eof() override
  {
    note_emit();
    append("eof\n");
    flush();
  }
};

struct FirstTokenTimingSource : IPPTokenSource
{
  explicit FirstTokenTimingSource(IPPTokenSource & input)
    : input(input),
      seen_first(false)
  {}

  EPPToken get() override
  {
    if(!seen_first) {
      seen_first = true;
      file_timing::startup_mark("preproc.first_pp_get_begin");
      EPPToken token = input.get();
      file_timing::startup_mark("preproc.first_pp_get_end");
      return token;
    }
    return input.get();
  }

  IPPTokenSource & input;
  bool seen_first;
};

}  // namespace

void write_preprocessed_posttoken_output(ostream & out,
                                         const vector<string> & srcfiles,
                                         const CppPreprocessOptions & options)
{
  file_timing::startup_mark("preproc.output.enter");
  PostTokenOutputStream output(out);
  out << "preproc " << srcfiles.size() << '\n';
  file_timing::startup_mark("preproc.header_written");

  const time_t now = time(nullptr);
  for(size_t i = 0; i < srcfiles.size(); ++i) {
    file_timing::startup_mark("preproc.file_begin");
    out << "sof " << srcfiles[i] << '\n';
    Preprocessor preprocessor(srcfiles[i], now, options);
    file_timing::startup_mark("preproc.preprocessor_constructed");
    FirstTokenTimingSource timed_preprocessor(preprocessor);
    PostTokenizer posttokenizer(timed_preprocessor);
    file_timing::startup_mark("preproc.posttokenizer_constructed");
    file_timing::startup_mark("preproc.stream_begin");
    stream_post_tokens(posttokenizer, output);
    file_timing::startup_mark("preproc.stream_end");
  }
}

void write_preprocessed_posttoken_output_file(const string & outfile,
                                              const vector<string> & srcfiles,
                                              const CppPreprocessOptions & options)
{
  ofstream out(outfile.c_str());
  if(!out) {
    throw logic_error("unable to open output file");
  }
  write_preprocessed_posttoken_output(out, srcfiles, options);
}

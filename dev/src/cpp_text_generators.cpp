#include "cpp_text_generators.h"

#include <ctime>
#include <sstream>

using namespace std;

#include "cppast_ast.h"
#include "cppast_dump.h"
#include "cppast_parser.h"
#include "posttokenizer.h"
#include "preprocessor.h"
#include "recog_parser.h"
#include "recog_token_buffer.h"
#include "template_text_output.h"
#include "typesemantic.h"

string generate_cppast_translation_units(const vector<string> & srcfiles)
{
  const time_t now = time(nullptr);
  ostringstream buffer;
  buffer << srcfiles.size() << " translation units" << '\n';

  for(size_t i = 0; i < srcfiles.size(); ++i) {
    const string & srcfile = srcfiles[i];
    Preprocessor preprocessor(srcfile, now);
    SourceLocationTable source_locations;
    PostTokenizer posttokenizer(preprocessor, &source_locations, &preprocessor);
    RecogTokenizer tokenizer(posttokenizer);
    RecogTokenBuffer tokens(tokenizer, srcfile, &source_locations);
    CppAstParser parser(tokens);
    CppAstNode translation_unit;
    if(!parser.parse_translation_unit(translation_unit)) {
      throw logic_error(srcfile + ": " + parser.error());
    }

    buffer << "start translation unit " << (i + 1) << '\n';
    buffer << describe_cppast_translation_unit(translation_unit);
    buffer << "end translation unit" << '\n';
  }

  return buffer.str();
}

string generate_types_translation_units(const vector<string> & srcfiles)
{
  const time_t now = time(nullptr);
  ostringstream buffer;
  buffer << srcfiles.size() << " translation units" << '\n';

  for(size_t i = 0; i < srcfiles.size(); ++i) {
    const string & srcfile = srcfiles[i];
    Preprocessor preprocessor(srcfile, now);
    SourceLocationTable source_locations;
    PostTokenizer posttokenizer(preprocessor, &source_locations, &preprocessor);
    RecogTokenizer tokenizer(posttokenizer);
    RecogTokenBuffer tokens(tokenizer, srcfile, &source_locations);

    buffer << "start translation unit " << (i + 1) << '\n';
    buffer << describe_types_translation_unit(tokens);
    buffer << "end translation unit" << '\n';
  }

  return buffer.str();
}

string generate_calls_translation_units(const vector<string> & srcfiles)
{
  const time_t now = time(nullptr);
  ostringstream buffer;
  buffer << srcfiles.size() << " translation units" << '\n';

  for(size_t i = 0; i < srcfiles.size(); ++i) {
    const string & srcfile = srcfiles[i];
    Preprocessor preprocessor(srcfile, now);
    SourceLocationTable source_locations;
    PostTokenizer posttokenizer(preprocessor, &source_locations, &preprocessor);
    RecogTokenizer tokenizer(posttokenizer);
    RecogTokenBuffer tokens(tokenizer, srcfile, &source_locations);

    buffer << "start translation unit " << (i + 1) << '\n';
    buffer << describe_calls_translation_unit(tokens);
    buffer << "end translation unit" << '\n';
  }

  return buffer.str();
}

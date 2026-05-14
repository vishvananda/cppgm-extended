#include <iostream>

using namespace std;

#include "encoding.h"
#include "pptokenizer.h"

struct DebugPPTokenStream : IPPTokenStream
{
  void emit(const EPPTokenType type, const std::string & data = std::string())
  {
    switch(type) {
    case PP_WHITESPACE:
      cout << "whitespace-sequence 0 \n";
      break;
    case PP_NEW_LINE:
      cout << "new-line 0 \n";
      break;
    case PP_HEADER_NAME:
      write_token("header-name", data);
      break;
    case PP_IDENTIFIER:
      write_token("identifier", data);
      break;
    case PP_INT_LITERAL:
    case PP_FLOAT_LITERAL:
      write_token("pp-number", data);
      break;
    case PP_QUOTE_LITERAL:
        for(auto c : data) {
          if(c == '\'') {
            if(data[data.size() - 1] == c)
              write_token("character-literal", data);
            else
              write_token("user-defined-character-literal", data);
            break;
          } else if(c == '"') {
            if(data[data.size() - 1] == c)
              write_token("string-literal", data);
            else
              write_token("user-defined-string-literal", data);
            break;
          }
        }
      break;
    case PP_PREPROCESSING_OP:
      write_token("preprocessing-op-or-punc", data);
      break;
    case PP_NON_WHITESPACE:
      write_token("non-whitespace-character", data);
      break;
    case PP_EOF:
      cout << "eof\n";
      break;
    }
  }

private:
  void write_token(const string& type, const string& data)
  {
    cout << type << " " << data.size() << " ";
    cout.write(data.data(), data.size());
    cout.put('\n');
  }
};

int main(int argc, char** argv)
{
  (void)argc;
  (void)argv;
  cin.sync_with_stdio(false);
  cin.tie(nullptr);
  DebugPPTokenStream output;
  PPTokenizer tokenizer(cin.rdbuf());
  try
  {
    stream_pp_tokens(tokenizer, output);
  }
  catch (exception& e)
  {
    cerr << "ERROR:" << tokenizer.get_ln() << ":"
         << tokenizer.get_ch() << ":" << e.what() << '\n';
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

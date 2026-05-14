#include <iostream>
#include <stdexcept>
#include <string>

using namespace std;

#include "calculator.h"
#include "pptokenizer.h"

struct DebugRunner : IPPTokenStream
{
  DebugRunner() : empty(true), defined_state(DefinedState::None) {}

  void emit_stdout(const string& data)
  {
    stdout_buffer += data;
    stdout_buffer.push_back('\n');
  }

  void emit_stderr(const string& data)
  {
    stderr_buffer += "ERROR:";
    stderr_buffer += data;
    stderr_buffer.push_back('\n');
  }

  virtual void emit(const EPPTokenType type,
                    const string & data = string())
  {
    try {
      switch(defined_state) {
      case DefinedState::None:
        switch(type) {
        case PP_NEW_LINE:
          if(error.size()) {
            emit_stderr(error);
            emit_stdout("error");
            defined_state = DefinedState::None;
            error = string();
            empty = true;
          }
          if(empty)
            break;
          {
            string error_out;
            calculator.try_calculate(error_out);
            if(!error_out.empty()) {
              emit_stderr(error_out);
              emit_stdout("error");
            } else if(calculator.issigned) {
              emit_stdout(to_string((long long)calculator.value));
            } else {
              emit_stdout(to_string(calculator.value) + "u");
            }
          }
          empty = true;
          break;
        case PP_WHITESPACE:
          break;
        case PP_EOF:
          emit_stdout("eof");
          break;
        default:
          empty = false;
          if(type == PP_IDENTIFIER && data == "defined") {
            defined_state = DefinedState::Start;
          } else {
            calculator.accumulate(type, data);
          }
          break;
        }
        break;
      case DefinedState::Start:
        if(type == PP_PREPROCESSING_OP && data == "(") {
          defined_state = DefinedState::Paren;
        } else if(type == PP_IDENTIFIER) {
          if(data[0] % 2)
            calculator.accumulate(PP_INT_LITERAL, "1");
          else
            calculator.accumulate(PP_INT_LITERAL, "0");
          defined_state = DefinedState::None;
        } else if(type != PP_WHITESPACE) {
          throw logic_error("Expected whitespace or paren after defined.");
        }
        break;
      case DefinedState::Paren:
        if(type == PP_IDENTIFIER) {
          if(data[0] % 2)
            calculator.accumulate(PP_INT_LITERAL, "1");
          else
            calculator.accumulate(PP_INT_LITERAL, "0");
          defined_state = DefinedState::End;
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
    } catch (logic_error & e) {
      if(type == PP_NEW_LINE) {
        emit_stderr(e.what());
        emit_stdout("error");
        defined_state = DefinedState::None;
        error = string();
        empty = true;
      } else {
        error = e.what();
      }
    }
  }

  string error;
  string stdout_buffer;
  string stderr_buffer;
  bool empty;
  enum struct DefinedState {None, Start, Paren, End};
  DefinedState defined_state;
  Calculator calculator;
};

int main(int argc, char** argv)
{
  (void)argc;
  (void)argv;
  cin.sync_with_stdio(false);
  cin.tie(nullptr);
  cerr << nounitbuf;
  DebugRunner runner;
  PPTokenizer tokenizer(cin.rdbuf());
  try {
    stream_pp_tokens(tokenizer, runner);
  } catch (exception& e) {
    if(!runner.stdout_buffer.empty())
      cout << runner.stdout_buffer;
    if(!runner.stderr_buffer.empty())
      cerr << runner.stderr_buffer;
    cerr << "ERROR:" << tokenizer.get_ln() << ":"
         << tokenizer.get_ch() << ":" << e.what() << '\n';

    return EXIT_FAILURE;
  }
  if(!runner.stdout_buffer.empty())
    cout << runner.stdout_buffer;
  if(!runner.stderr_buffer.empty())
    cerr << runner.stderr_buffer;
  return EXIT_SUCCESS;
}

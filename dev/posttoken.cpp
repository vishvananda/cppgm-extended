#include <iostream>

using namespace std;

#include "pptokenizer.h"
#include "posttokenizer.h"


int main(int argc, char** argv)
{
  (void)argc;
  (void)argv;
  cin.sync_with_stdio(false);
  DebugPostTokenOutputStream output;
  PPTokenizer tokenizer(cin.rdbuf());
  PostTokenizer posttokenizer(tokenizer);
  try {
    stream_post_tokens(posttokenizer, output);
  } catch (exception& e) {
    cerr << "ERROR:" << tokenizer.get_ln() << ":"
         << tokenizer.get_ch() << ":" << e.what() << endl;

    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

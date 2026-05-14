#include <iostream>

using namespace std;

#include "posttokenizer.h"
#include "preprocessor.h"

int main(int argc, char** argv)
{
  (void)argc;
  (void)argv;
  cin.sync_with_stdio(false);
  DebugPostTokenOutputStream output;
  Preprocessor preprocessor;
  PostTokenizer posttokenizer(preprocessor);
  try {
    preprocessor.load(cin.rdbuf());
    stream_post_tokens(posttokenizer, output);
  } catch (exception& e) {
    cerr << "ERROR:" << e.what() << endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

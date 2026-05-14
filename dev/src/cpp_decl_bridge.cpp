#include <stdexcept>

using namespace std;

#include "cpp_decl_bridge.h"

namespace cpp_decl {

vector<RecogToken> slice_recog_tokens(const vector<RecogToken> & tokens,
                                      size_t start,
                                      size_t end)
{
  vector<RecogToken> slice(tokens.begin() + start, tokens.begin() + end);
  slice.push_back(RecogToken{RT_EOF, string(), static_cast<ETokenType>(0), 0});
  return slice;
}

vector<RecogToken> slice_recog_tokens(const IRecogTokenSequence & tokens,
                                      size_t start,
                                      size_t end)
{
  vector<RecogToken> slice = tokens.slice(start, end);
  slice.push_back(RecogToken{RT_EOF, string(), static_cast<ETokenType>(0), 0});
  return slice;
}

bool has_valid_node_span(const vector<RecogToken> & tokens,
                         const CppAstNode & node)
{
  return node.token_end > node.token_start && node.token_end <= tokens.size();
}

bool has_valid_node_span(const IRecogTokenSequence & tokens,
                         const CppAstNode & node)
{
  return node.token_end > node.token_start && node.token_end <= tokens.size();
}

vector<RecogToken> slice_tokens_for_node(const vector<RecogToken> & tokens,
                                         const CppAstNode & node)
{
  if(!has_valid_node_span(tokens, node)) {
    throw logic_error("invalid AST node token span");
  }
  return slice_recog_tokens(tokens, node.token_start, node.token_end);
}

vector<RecogToken> slice_tokens_for_node(const IRecogTokenSequence & tokens,
                                         const CppAstNode & node)
{
  if(!has_valid_node_span(tokens, node)) {
    throw logic_error("invalid AST node token span");
  }
  return slice_recog_tokens(tokens, node.token_start, node.token_end);
}

string node_text(const CppAstNode & node)
{
  return node.value;
}

const CppAstNode * find_child(const CppAstNode & node,
                              CppAstKind kind,
                              size_t ordinal)
{
  for(size_t i = 0; i < node.children.size(); ++i) {
    if(node.children[i].kind != kind) {
      continue;
    }
    if(ordinal == 0) {
      return &node.children[i];
    }
    --ordinal;
  }
  return nullptr;
}

}  // namespace cpp_decl

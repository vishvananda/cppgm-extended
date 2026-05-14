#include <algorithm>
#include <sstream>
#include <stdexcept>
#include <utility>

using namespace std;

#include "encoding.h"
#include "macroizer.h"

inline void replace_string(string& str, const string& olds, const string& news)
{
  size_t pos = 0;
  while((pos = str.find(olds, pos)) != string::npos)
  {
    str.replace(pos, olds.size(), news);
    pos += news.size();
  }
}

Macroizer::Macroizer() :
  context_provider([]() { return ExpansionContext(); }),
  tokens(default_tokens),
  macros(default_macros),
  input(nullptr),
  input_complete(true),
  session_active(false),
  macro(nullptr),
  cur_token(0),
  macro_state(MacroState::Start),
  nesting(0)
{
  default_macros.reserve(32);
}

Macroizer::TokenBuffer::TokenBuffer() :
  data(32),
  start(0),
  count(0)
{}

Macroizer::TokenBuffer::TokenBuffer(vector<PPToken> && initial_tokens) :
  data(32),
  start(0),
  count(0)
{
  ensure_capacity(initial_tokens.size());
  for(auto & token : initial_tokens) {
    push_back(std::move(token));
  }
}

bool Macroizer::TokenBuffer::empty() const
{
  return count == 0;
}

size_t Macroizer::TokenBuffer::size() const
{
  return count;
}

Macroizer::PPToken & Macroizer::TokenBuffer::front()
{
  return data[start];
}

const Macroizer::PPToken & Macroizer::TokenBuffer::front() const
{
  return data[start];
}

Macroizer::PPToken & Macroizer::TokenBuffer::operator[](size_t n)
{
  return data[index(n)];
}

const Macroizer::PPToken & Macroizer::TokenBuffer::operator[](size_t n) const
{
  return data[index(n)];
}

void Macroizer::TokenBuffer::clear()
{
  start = 0;
  count = 0;
}

void Macroizer::TokenBuffer::push_back(PPToken token)
{
  ensure_capacity(count + 1);
  data[index(count)] = std::move(token);
  ++count;
}

void Macroizer::TokenBuffer::pop_front()
{
  start = (start + 1) & (data.size() - 1);
  --count;
}

void Macroizer::TokenBuffer::erase_prefix(size_t n)
{
  if(n >= count) {
    clear();
    return;
  }
  start = (start + n) & (data.size() - 1);
  count -= n;
}

void Macroizer::TokenBuffer::prepend(vector<PPToken> & values)
{
  ensure_capacity(count + values.size());
  for(auto it = values.rbegin(); it != values.rend(); ++it) {
    start = (start - 1) & (data.size() - 1);
    data[start] = std::move(*it);
    ++count;
  }
}

size_t Macroizer::TokenBuffer::index(size_t n) const
{
  return (start + n) & (data.size() - 1);
}

void Macroizer::TokenBuffer::ensure_capacity(size_t required)
{
  if(required <= data.size()) {
    return;
  }
  size_t capacity = data.size();
  while(capacity < required) {
    capacity *= 2;
  }
  vector<PPToken> expanded(capacity);
  for(size_t i = 0; i < count; ++i) {
    expanded[i] = std::move((*this)[i]);
  }
  data.swap(expanded);
  start = 0;
}

Macroizer::Macroizer(vector<PPToken> && initial_tokens,
                     unordered_map<string, Macro> & macros,
                     const context_callable & provider) :
  context_provider(provider),
  default_tokens(std::move(initial_tokens)),
  tokens(default_tokens),
  macros(macros),
  input(nullptr),
  input_complete(true),
  session_active(!default_tokens.empty()),
  macro(nullptr),
  cur_token(0),
  macro_state(MacroState::Start),
  nesting(0)
{}

void Macroizer::set_context_provider(const context_callable & provider)
{
  if(provider) {
    context_provider = provider;
  } else {
    context_provider = []() { return ExpansionContext(); };
  }
}

void Macroizer::begin(IPPTokenSource & input_source, EPPToken first)
{
  default_tokens.clear();
  default_tokens.push_back(PPToken{first.type, std::move(first.data)});
  input = &input_source;
  input_complete = false;
  session_active = true;
  macro = nullptr;
  cur_token = 0;
  macro_state = MacroState::Start;
  args.clear();
  cur_arg.clear();
  nesting = 0;
}

bool Macroizer::active() const
{
  return session_active;
}

EPPToken Macroizer::get()
{
  auto next = get_pp();
  return EPPToken{next.type, std::move(next.data)};
}

inline bool Macroizer::pull_input()
{
  if(input == nullptr || input_complete) {
    return false;
  }
  auto next = input->get();
  if(next.type == PP_EOF) {
    input_complete = true;
    return false;
  }
  tokens.push_back(PPToken{next.type, std::move(next.data)});
  return true;
}

inline bool Macroizer::blacklist_contains(const BlacklistPtr & blacklist,
                                          const string & id)
{
  for(auto current = blacklist; current; current = current->parent) {
    if(current->id == id) {
      return true;
    }
  }
  return false;
}

inline Macroizer::BlacklistPtr Macroizer::merge_blacklists(
    BlacklistPtr base,
    const BlacklistPtr & extra)
{
  vector<string> ids;
  for(auto current = extra; current; current = current->parent) {
    if(!blacklist_contains(base, current->id)) {
      ids.push_back(current->id);
    }
  }
  for(auto it = ids.rbegin(); it != ids.rend(); ++it) {
    base = std::make_shared<Blacklist>(base, *it);
  }
  return base;
}

inline void Macroizer::add_expansion_blacklist(PPToken & target,
                                               const PPToken & macro_token,
                                               BlacklistPtr & shared_inherited_blacklist,
                                               BlacklistPtr & shared_direct_blacklist)
{
  const bool include_inherited =
      !macro_token.breaks_inherited_blacklist ||
      target.breaks_inherited_blacklist;
  if(target.blacklist) {
    if(include_inherited) {
      target.blacklist =
          merge_blacklists(target.blacklist, macro_token.blacklist);
    }
    if(!blacklist_contains(target.blacklist, macro_token.data)) {
      target.blacklist =
          std::make_shared<Blacklist>(target.blacklist, macro_token.data);
    }
    return;
  }
  BlacklistPtr & shared_blacklist =
      include_inherited ? shared_inherited_blacklist : shared_direct_blacklist;
  if(!shared_blacklist) {
    BlacklistPtr base = include_inherited ? macro_token.blacklist : nullptr;
    if(blacklist_contains(base, macro_token.data)) {
      shared_blacklist = base;
    } else {
      shared_blacklist =
          std::make_shared<Blacklist>(base, macro_token.data);
    }
  }
  target.blacklist = shared_blacklist;
}

inline string Macroizer::stringize(const vector<PPToken> & tokens) {
  string output;
  for(auto & token : tokens) {
    if(token.type == PP_WHITESPACE) {
      output += " ";
    } else if(token.type == PP_QUOTE_LITERAL){
      string out = token.data;
      replace_string(out, "\\", "\\\\");
      replace_string(out, "\"", "\\\"");
      output += out;
    } else {
      output += token.data;
    }
  }
  return output;
}

inline Macroizer::PPToken Macroizer::join_tokens(const PPToken & lhs,
                                                 const PPToken & rhs) {
  auto data = lhs.data + rhs.data;
  auto result = tokenize(data);
  auto len = result.size();
  if(len > 3) {
    throw logic_error("Join resulted in more than one token");
  } else if(len < 3) {
    throw logic_error("Join resulted in no token");
  }
  PPToken joined(result[0].type, result[0].data);
  joined.breaks_inherited_blacklist = true;
  return joined;
}

inline vector<Macroizer::PPToken> Macroizer::expand_tokens(
    vector<PPToken> input_tokens)
{
  Macroizer expander(std::move(input_tokens), macros, context_provider);
  vector<PPToken> result;
  result.reserve(expander.tokens.size());
  for(;;) {
    auto next = expander.get_pp();
    if(next.type == PP_EOF) {
      break;
    }
    result.push_back(next);
  }
  return result;
}

inline vector<Macroizer::PPToken> Macroizer::replace_param(
    const PPToken & token,
    bool recurse,
    vector<vector<PPToken> > * expanded_args,
    vector<bool> * expanded_arg_valid)
{
  if(token.type == PP_IDENTIFIER) {
    auto param_it = macro->param_indices.find(token.data);
    if(param_it != macro->param_indices.end()) {
      const size_t arg_index = param_it->second;
      vector<PPToken> result;
      if(recurse && expanded_args != nullptr && expanded_arg_valid != nullptr) {
        if(expanded_args->empty()) {
          expanded_args->resize(macro->params.size());
          expanded_arg_valid->assign(macro->params.size(), false);
        }
        if(!(*expanded_arg_valid)[arg_index]) {
          (*expanded_args)[arg_index] = args[arg_index];
          (*expanded_args)[arg_index] =
              expand_tokens(std::move((*expanded_args)[arg_index]));
          (*expanded_arg_valid)[arg_index] = true;
        }
        result = (*expanded_args)[arg_index];
      } else {
        result = args[arg_index];
      }
      if(recurse && (expanded_args == nullptr || expanded_arg_valid == nullptr)) {
        result = expand_tokens(std::move(result));
      }
      for(auto & param_token : result) {
        param_token.breaks_inherited_blacklist = true;
      }
      return result;
    }
  }
  vector<PPToken> result;
  result.push_back(token);
  return result;
}

inline vector<Macroizer::PPToken> Macroizer::handle_stringize(
    vector<PPToken>::iterator & it,
    const vector<PPToken>::iterator & end,
    bool recurse,
    vector<vector<PPToken> > * expanded_args,
    vector<bool> * expanded_arg_valid) {
  if(!macro->functional || it->type != PP_PREPROCESSING_OP ||
     (it->data != "#" && it->data != "%:")) {
    return replace_param(*it, recurse, expanded_args, expanded_arg_valid);
  }
  if(++it == end || (it->type == PP_WHITESPACE && ++it == end))
    throw logic_error("Missing value after # in macro replacement");
  vector<PPToken> result;
  auto output = stringize(replace_param(*it, false));
  result.push_back({PP_QUOTE_LITERAL, "\"" + output + "\""});
  return result;
}

inline vector<Macroizer::PPToken> Macroizer::handle_join(
    vector<PPToken>::iterator & it,
    const vector<PPToken>::iterator & end,
    vector<vector<PPToken> > * expanded_args,
    vector<bool> * expanded_arg_valid) {
  auto next = it;
  bool recurse;
  if(++next == end ||
     (next->type == PP_WHITESPACE && ++next == end) ||
     (next->type != PP_PREPROCESSING_OP ||
     (next->data != "##" && next->data != "%:%:"))) {
    recurse = true;
  } else {
    recurse = false;
  }
  auto result =
      handle_stringize(it, end, recurse, expanded_args, expanded_arg_valid);
  next = it;
  if(++next == end ||
     (next->type == PP_WHITESPACE && ++next == end) ||
     (next->type != PP_PREPROCESSING_OP ||
     (next->data != "##" && next->data != "%:%:"))) {
    return result;
  }

  while(next->type == PP_PREPROCESSING_OP &&
        (next->data == "##" || next->data == "%:%:")) {
    if(++next == end || (next->type == PP_WHITESPACE && ++next == end))
      break;
    auto rresult =
        handle_stringize(next, end, false, expanded_args, expanded_arg_valid);
    if(result.empty()) {
      result = std::move(rresult);
    } else if(!rresult.empty()) {
      auto lhs = result.back();
      result.pop_back();
      auto rhs = rresult.front();
      result.push_back(join_tokens(lhs, rhs));
      result.insert(result.end(),
                    std::make_move_iterator(rresult.begin() + 1),
                    std::make_move_iterator(rresult.end()));
    }
    it = next;
    if(++next == end || (next->type == PP_WHITESPACE && ++next == end))
      break;
  }
  return result;
}

inline void Macroizer::complete_arg() {
  bool allow_vargs = (!macro->params.empty() &&
                      macro->params.back() == "__VA_ARGS__");
  if(args.size() == macro->params.size()) {
    if(!allow_vargs && !cur_arg.empty()) {
      throw logic_error("Too many args to macro");
    }
    if(!args.empty() && !cur_arg.empty()) {
      auto & back = args.back();
      back.insert(back.end(),
                  std::make_move_iterator(cur_arg.begin()),
                  std::make_move_iterator(cur_arg.end()));
      cur_arg.clear();
    }
  } else {
    if(!cur_arg.empty() && cur_arg.back().type == PP_WHITESPACE) {
      cur_arg.pop_back();
    }
    args.emplace_back(std::move(cur_arg));
    cur_arg.clear();
  }
}

inline Macroizer::PPToken Macroizer::get_pp()
{
  if(!session_active) {
    return {PP_EOF, string()};
  }
  while(true) {
    if(tokens.empty()) {
      input = nullptr;
      input_complete = true;
      session_active = false;
      macro_state = MacroState::Start;
      cur_token = 0;
      return {PP_EOF, string()};
    }

    switch(macro_state) {
    case MacroState::Start:
      if(tokens.front().type == PP_IDENTIFIER) {
        auto macro_it = macros.find(tokens.front().data);
        if(macro_it != macros.end() &&
           !blacklist_contains(tokens.front().blacklist, tokens.front().data)) {
          macro = &macro_it->second;
          if(macro->functional) {
            macro_state = MacroState::Match;
            cur_token = 1;
            continue;
          }
          cur_token = 0;
          replace_tokens();
          cur_token = 0;
          continue;
        }
      }
      {
        auto out = std::move(tokens.front());
        tokens.pop_front();
        cur_token = 0;
        return out;
      }
      break;
    case MacroState::Match:
      if(cur_token >= tokens.size()) {
        if(pull_input()) {
          continue;
        }
        macro_state = MacroState::Start;
        auto out = tokens.front();
        tokens.pop_front();
        cur_token = 0;
        return out;
      }
      if(tokens[cur_token].type == PP_WHITESPACE) {
        ++cur_token;
        continue;
      }
      if(tokens[cur_token].type == PP_PREPROCESSING_OP &&
         tokens[cur_token].data == "(") {
        args.clear();
        cur_arg.clear();
        nesting = 0;
        macro_state = MacroState::Arg;
        ++cur_token;
        continue;
      }
      macro_state = MacroState::Start;
      {
        auto out = std::move(tokens.front());
        tokens.pop_front();
        cur_token = 0;
        return out;
      }
      break;
    case MacroState::Arg:
      if(cur_token >= tokens.size()) {
        if(pull_input()) {
          continue;
        }
        throw logic_error("Unterminated macro");
      }
      {
        auto n_params = macro->params.size();
        bool allow_vargs = (!macro->params.empty() &&
                            macro->params.back() == "__VA_ARGS__");
        auto & token = tokens[cur_token];
        if(token.type == PP_PREPROCESSING_OP) {
          if(token.data == "(") {
            ++nesting;
          } else if(token.data == ")") {
            if(nesting) {
              --nesting;
            } else {
              complete_arg();
              if(allow_vargs && args.size() + 1 == n_params) {
                args.emplace_back();
              }
              if(n_params > args.size()) {
                ostringstream msg;
                const ExpansionContext context = context_provider();
                msg << "Insufficient arguments for macro " << tokens.front().data
                    << " (expected " << n_params << ", got " << args.size() << ")";
                if(!context.file.empty()) {
                  msg << " at " << context.file << ":" << context.line;
                }
                msg << " near";
                const size_t preview_end = min(tokens.size(), cur_token + 1);
                for(size_t i = 0; i < preview_end; ++i) {
                  msg << (i == 0 ? " " : " ") << tokens[i].data;
                }
                throw logic_error(msg.str());
              }
              replace_tokens();
              args.clear();
              cur_arg.clear();
              macro_state = MacroState::Start;
              cur_token = 0;
              continue;
            }
          } else if(nesting == 0 &&
                    args.size() < n_params - 1 &&
                    token.data == ",") {
            complete_arg();
            ++cur_token;
            continue;
          }
        }
        if((allow_vargs && args.size() == n_params) ||
           !cur_arg.empty() ||
           token.type != PP_WHITESPACE) {
          cur_arg.push_back(token);
        }
        ++cur_token;
      }
      break;
    }
  }
}

inline void Macroizer::replace_tokens()
{
  auto token = std::move(tokens.front());
  tokens.erase_prefix(cur_token + 1);
  vector<vector<PPToken> > expanded_args;
  vector<bool> expanded_arg_valid;
  const bool replacement_tail_deferred_helper =
      macro->functional &&
      !macro->tokens.empty() &&
      macro->tokens.back().type == PP_IDENTIFIER &&
      macro->param_indices.find(macro->tokens.back().data) ==
          macro->param_indices.end();
  const string replacement_tail_identifier =
      replacement_tail_deferred_helper ? macro->tokens.back().data : string();
  const auto add_replacement_blacklists =
      [&](vector<PPToken> & replist)
      {
        BlacklistPtr inherited_expansion_blacklist;
        BlacklistPtr direct_expansion_blacklist;
        for(size_t i = 0; i < replist.size(); ++i) {
          PPToken & newtok = replist[i];
          if(newtok.type == PP_IDENTIFIER) {
            add_expansion_blacklist(newtok, token,
                                    inherited_expansion_blacklist,
                                    direct_expansion_blacklist);
            if(replacement_tail_deferred_helper &&
               i + 1 == replist.size() &&
               newtok.data == replacement_tail_identifier) {
              newtok.breaks_inherited_blacklist = true;
            }
          }
        }
      };
  if(macro->func == nullptr) {
    if(!macro->replacement_has_join &&
       (!macro->functional || !macro->replacement_has_stringize)) {
      if(macro->tokens.empty()) {
        return;
      }
      vector<PPToken> replist;
      replist.reserve(macro->tokens.size());
      for(const auto & rep : macro->tokens) {
        if(macro->functional && rep.type == PP_IDENTIFIER) {
          auto param_it = macro->param_indices.find(rep.data);
          if(param_it != macro->param_indices.end()) {
            vector<PPToken> expanded =
                replace_param(rep, true, &expanded_args, &expanded_arg_valid);
            replist.insert(replist.end(),
                           std::make_move_iterator(expanded.begin()),
                           std::make_move_iterator(expanded.end()));
            continue;
          }
        }
        replist.push_back(rep);
      }
      add_replacement_blacklists(replist);
      tokens.prepend(replist);
      return;
    }
  }
  vector<PPToken> replist;
  vector<PPToken> callable_tokens;
  vector<PPToken> * replacements = &macro->tokens;
  if(macro->func != nullptr) {
    auto ctx = context_provider();
    auto result = macro->func(ctx);
    callable_tokens.emplace_back(PPToken{result.type, result.data});
    replacements = &callable_tokens;
    replist.reserve(replacements->size());
  } else {
    replist.reserve(macro->tokens.size());
  }
  auto end = replacements->end();
  for(auto it = replacements->begin(); it != end; ++it) {
    auto rep = handle_join(it, end, &expanded_args, &expanded_arg_valid);
    replist.insert(replist.end(),
                   std::make_move_iterator(rep.begin()),
                   std::make_move_iterator(rep.end()));
  }
  add_replacement_blacklists(replist);
  tokens.prepend(replist);
}

vector<EPPToken> Macroizer::expand(vector<EPPToken> input_tokens)
{
  vector<PPToken> data;
  data.reserve(input_tokens.size());
  for(auto & token : input_tokens) {
    if(token.type != PP_EOF) {
      data.push_back(PPToken{token.type, std::move(token.data)});
    }
  }
  auto result = expand_tokens(std::move(data));
  vector<EPPToken> output;
  output.reserve(result.size());
  for(auto & token : result) {
    output.emplace_back(EPPToken{token.type, std::move(token.data)});
  }
  return output;
}

void Macroizer::macro_start(const string & id)
{
  next_macro.param_indices.clear();
  next_macro.params.clear();
  next_macro.tokens.clear();
  next_macro.func = nullptr;
  next_macro_id = id;
  next_macro.functional = false;
  next_macro.replacement_has_join = false;
  next_macro.replacement_has_stringize = false;
}

void Macroizer::macro_set_functional()
{
  next_macro.functional = true;
}

void Macroizer::macro_add_param(const string & id)
{
  if(next_macro.param_indices.count(id) != 0) {
    throw logic_error("Duplicate identifier in macro parameters");
  }
  next_macro.param_indices.emplace(id, next_macro.params.size());
  next_macro.params.push_back(id);
}

void Macroizer::macro_add_repl(const EPPTokenType type,
                               const std::string & data)
{
  if(type == PP_PREPROCESSING_OP && (data == "##" || data == "%:%:") &&
     next_macro.tokens.empty())
    throw logic_error("Illegal initial replacement of ##");
  if(type == PP_IDENTIFIER && data == "__VA_ARGS__" &&
     (next_macro.params.empty() || next_macro.params.back() != data))
    throw logic_error("Illegal use of __VA_ARGS__");
  if(!next_macro.tokens.empty() || type != PP_WHITESPACE) {
    next_macro.tokens.emplace_back(PPToken{type, data});
  }
}

void Macroizer::macro_finish()
{
  next_macro.replacement_has_join = false;
  next_macro.replacement_has_stringize = false;
  for(const auto & token : next_macro.tokens) {
    if(token.type == PP_PREPROCESSING_OP) {
      if(token.data == "##" || token.data == "%:%:") {
        next_macro.replacement_has_join = true;
      } else if(token.data == "#" || token.data == "%:") {
        next_macro.replacement_has_stringize = true;
      }
    }
  }

  if(!next_macro.tokens.empty()) {
    const auto & back = next_macro.tokens.back();
    if(back.type == PP_PREPROCESSING_OP &&
       (back.data == "##" || back.data == "%:%:"))
        throw logic_error("Illegal final replacement of ##");
  }

  if(!next_macro.params.empty()) {
    bool match_hash = false;
    for(const auto & token : next_macro.tokens) {
      if(match_hash) {
        if(token.type == PP_WHITESPACE)
          continue;
        if(token.type == PP_IDENTIFIER) {
          if(next_macro.param_indices.count(token.data) != 0) {
            match_hash = false;
            continue;
          }
        }
        break;
      }
      if(token.type == PP_PREPROCESSING_OP &&
         (token.data == "#" || token.data == "%:"))
        match_hash = true;
    }
    if(match_hash)
      throw logic_error("hash must be followed by argument");
  }

  if(!next_macro.tokens.empty() &&
     next_macro.tokens.back().type == PP_WHITESPACE) {
    next_macro.tokens.pop_back();
  }
  const ExpansionContext context = context_provider();
  next_macro.defined_in_system_header = context.system_header;
  auto existing = macros.find(next_macro_id);
  if(existing != macros.end()) {
    if(existing->second.functional != next_macro.functional ||
       existing->second.tokens != next_macro.tokens ||
       existing->second.params != next_macro.params) {
      if(existing->second.defined_in_system_header &&
         next_macro.defined_in_system_header) {
        existing->second = next_macro;
      } else {
        throw logic_error(string("Illegal redefinition of macro ") +
                          next_macro_id + " at " + context.file + ":" +
                          to_string(context.line));
      }
    }
  } else {
    macros[next_macro_id] = next_macro;
  }
  next_macro_id.clear();
  next_macro.param_indices.clear();
  next_macro.params.clear();
  next_macro.tokens.clear();
  next_macro.func = nullptr;
  next_macro.functional = false;
  next_macro.replacement_has_join = false;
  next_macro.replacement_has_stringize = false;
}

void Macroizer::macro_add(const string & id,
                          const EPPTokenType type,
                          const string & data) {
  macro_start(id);
  macro_add_repl(type, data);
  macro_finish();
}

void Macroizer::macro_add(const string & id,
                          const token_callable func) {
  macro_start(id);
  next_macro.func = func;
  macro_finish();
}

void Macroizer::macro_remove(const string & id) {
  macros.erase(id);
}

bool Macroizer::macro_exists(const string & id) {
  return macros.count(id) > 0;
}

#pragma once

#include <cstddef>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "pptokenizer.h"

struct ExpansionContext
{
  ExpansionContext() :
    file(),
    line(0),
    system_header(false),
    counter_value(nullptr)
  {}

  ExpansionContext(std::string file,
                   unsigned long long line,
                   bool system_header = false,
                   unsigned long long * counter_value = nullptr) :
    file(std::move(file)),
    line(line),
    system_header(system_header),
    counter_value(counter_value)
  {}

  std::string file;
  unsigned long long line;
  bool system_header;
  unsigned long long * counter_value;
};

typedef EPPToken (*token_callable)(const ExpansionContext &);
typedef std::function<ExpansionContext()> context_callable;

struct Macroizer
{
  Macroizer();
  void set_context_provider(const context_callable & provider);

  void macro_start(const std::string & id);
  void macro_set_functional();
  void macro_set_command_line();
  void macro_add_param(const std::string & id);
  void macro_add_repl(const EPPTokenType type,
                      const std::string & data = std::string());
  void macro_finish();
  void macro_add(const std::string & id,
                 const EPPTokenType,
                 const std::string & data = std::string());
  void macro_add(const std::string & id, const token_callable func);
  void macro_remove(const std::string & id);
  bool macro_exists(const std::string & id);

  void begin(IPPTokenSource & input, EPPToken first);
  EPPToken get();
  bool active() const;
  std::vector<EPPToken> expand(std::vector<EPPToken> input);

protected:
  struct Blacklist;
  typedef std::shared_ptr<const Blacklist> BlacklistPtr;

  struct Blacklist
  {
    Blacklist(const BlacklistPtr & parent, const std::string & id) :
      parent(parent),
      id(id)
    {}

    BlacklistPtr parent;
    std::string id;
  };

  struct PPToken
  {
    PPToken() :
      type(PP_EOF),
      breaks_inherited_blacklist(false)
    {}

    PPToken(EPPTokenType type, std::string data = std::string()) :
      type(type),
      data(std::move(data)),
      breaks_inherited_blacklist(false)
    {}

    EPPTokenType type;
    std::string data;
    BlacklistPtr blacklist;
    // Arguments and pasted tokens can select a later helper macro. When that
    // helper expands, inherited unavailable names must not suppress its body.
    bool breaks_inherited_blacklist;
    bool operator==(const PPToken &other) const
    {
      return (type == other.type &&
              data == other.data);
    }
  };

  struct Macro
  {
    Macro() :
      func(nullptr),
      functional(false),
      replacement_has_join(false),
      replacement_has_stringize(false),
      defined_on_command_line(false),
      defined_in_system_header(false)
    {};
    std::unordered_map<std::string, std::size_t> param_indices;
    std::vector<std::string> params;
    std::vector<PPToken> tokens;
    token_callable func;
    bool functional;
    bool replacement_has_join;
    bool replacement_has_stringize;
    bool defined_on_command_line;
    bool defined_in_system_header;
  };

  struct TokenBuffer
  {
    TokenBuffer();
    TokenBuffer(std::vector<PPToken> && initial_tokens);
    bool empty() const;
    std::size_t size() const;
    PPToken & front();
    const PPToken & front() const;
    PPToken & operator[](std::size_t n);
    const PPToken & operator[](std::size_t n) const;
    void clear();
    void push_back(PPToken token);
    void pop_front();
    void erase_prefix(std::size_t count);
    void prepend(std::vector<PPToken> & tokens);

  protected:
    std::size_t index(std::size_t n) const;
    void ensure_capacity(std::size_t required);

    std::vector<PPToken> data;
    std::size_t start;
    std::size_t count;
  };

  Macroizer(std::vector<PPToken> && tokens,
            std::unordered_map<std::string, Macro> & macros,
            const context_callable & provider);
  inline PPToken get_pp();
  inline bool pull_input();
  inline void replace_tokens();
  inline static bool blacklist_contains(const BlacklistPtr & blacklist,
                                        const std::string & id);
  inline BlacklistPtr merge_blacklists(BlacklistPtr base,
                                       const BlacklistPtr & extra);
  inline void add_expansion_blacklist(PPToken & target,
                                      const PPToken & macro_token,
                                      BlacklistPtr & shared_inherited_blacklist,
                                      BlacklistPtr & shared_direct_blacklist);
  inline std::vector<PPToken> replace_param(
      const PPToken & token,
      bool recurse,
      std::vector<std::vector<PPToken> > * expanded_args = nullptr,
      std::vector<bool> * expanded_arg_valid = nullptr);
  inline std::vector<PPToken> handle_stringize(
      std::vector<PPToken>::iterator & it,
      const std::vector<PPToken>::iterator & end,
      bool recurse,
      std::vector<std::vector<PPToken> > * expanded_args,
      std::vector<bool> * expanded_arg_valid);
  inline std::vector<PPToken> handle_join(
      std::vector<PPToken>::iterator & it,
      const std::vector<PPToken>::iterator & end,
      std::vector<std::vector<PPToken> > * expanded_args,
      std::vector<bool> * expanded_arg_valid);
  inline std::string stringize(const std::vector<PPToken> & tokens);
  inline PPToken join_tokens(const PPToken & lhs, const PPToken & rhs);
  inline void complete_arg();
  inline std::vector<PPToken> expand_tokens(std::vector<PPToken> input);
  enum struct MacroState {Start, Match, Arg};

  Macro next_macro;
  std::string next_macro_id;

  context_callable context_provider;
  TokenBuffer default_tokens;
  std::unordered_map<std::string, Macro> default_macros;
  TokenBuffer & tokens;
  std::unordered_map<std::string, Macro> & macros;
  IPPTokenSource * input;
  bool input_complete;
  bool session_active;
  Macro * macro;
  std::size_t cur_token;
  MacroState macro_state;
  std::vector< std::vector<PPToken> > args;
  std::vector<PPToken> cur_arg;
  int nesting;
};

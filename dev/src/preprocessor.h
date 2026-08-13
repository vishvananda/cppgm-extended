#pragma once

#include <deque>
#include <fstream>
#include <ctime>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "calculator.h"
#include "cpp_preprocess_options.h"
#include "macroizer.h"
#include "pptokenizer.h"
#include "source_location.h"

typedef std::pair<unsigned long int, unsigned long int> FileId;

const std::vector<std::string> & standard_include_paths();

struct Preprocessor : IPPTokenSource, ISourceLocationProvider
{
  Preprocessor(const std::string & file = std::string(),
               std::time_t now = 0,
               const CppPreprocessOptions & options = CppPreprocessOptions());
  EPPToken get() override;
  void load(const std::string & file);
  void load(std::streambuf * buf);
  void load(const std::string & file, std::streambuf * buf);
  inline const std::string file() const
      {return !files.empty() ? files.back()->file : "";};
  inline unsigned long long line() const
      {return !files.empty() ? files.back()->tokenizer.get_ln() : 0;};
  inline bool current_file_is_system_header() const
      {return !files.empty() && files.back()->system_header;};
  const std::string & current_source_file() const override
      {return last_output_from_macro_expansion_ ?
          macro_expansion_source_file_ :
          (cursor.token_file_ptr ? *cursor.token_file_ptr : cursor.token_file);};
  uint32_t current_source_line() const override
      {return last_output_from_macro_expansion_ ?
          macro_expansion_source_line_ : cursor.token_line;};
  uint32_t current_source_column() const override
      {return last_output_from_macro_expansion_ ?
          macro_expansion_source_column_ : cursor.token_column;};
  bool current_source_is_macro_expansion() const override
      {return last_output_from_macro_expansion_;};
  inline bool complete()
      {return cursor.complete() && !macroizer.active(); };
  const std::vector<std::string> & dependency_files() const
      {return dependency_files_;};
protected:
  void start_new_file();
  bool process(const EPPTokenType type, const std::string & data,
               bool allow_macro_start);
  void handle_hash_identifier(const std::string & data);
  void finish_include_directive(bool include_next);
  void finish_if_directive();
  void finish_line_directive();
  std::vector<std::string> build_include_search_dirs(bool quoted) const;
  std::size_t first_system_search_dir_index(
      const std::vector<std::string> & search_dirs) const;
  bool resolve_include(const std::string & name,
                       bool system,
                       bool include_next,
                       std::string & resolved_path,
                       FileId & file_id,
                       std::vector<std::string> & search_dirs,
                       std::size_t & search_dir_index) const;

  enum struct DirectiveState {Start, Inactive, Hash, Define, DefineIdent,
                              DefineReplacement, DefineParam, DefineSep,
                              DefineVa, Undef, UndefIdent, If, Elif,
                              NotIf, Else, EndIf, Error, Warning, Include,
                              Once, IncludeNext, Line, Ignore, Pragma, PragmaPack,
                              PragmaOp, PragmaOpLit, PragmaOpEnd};

  enum struct DefinedState {None, Start, Paren, NoParen, End};
  enum struct IfState {Start, If, NoElif, Else};
  void finish_pragma_operator();
  void finish_pragma_pack_directive();
  void inject_pragma_pack_marker(const std::string & marker);
  void handle_pragma_op_literal(const std::string & data,
                                DirectiveState resume_state);
  void begin_collected_directive(DirectiveState state);
  void append_directive_token(EPPTokenType type, const std::string & data);
  std::vector<EPPToken> expand_directive_tokens();
  struct IfStateStruct {
    bool active;
    bool parent_active;
    IfState state;
  };

  struct FileState
  {
    std::string file;
    std::vector<std::string> include_search_dirs;
    std::size_t include_search_index;
    bool include_search_index_valid;
    bool system_header;
    std::ifstream stream;
    PPTokenizer tokenizer;

    FileState(const std::string & file, bool system_header = false);
    FileState(std::streambuf * buf);
    FileState(const std::string & file, std::streambuf * buf);
  };

  struct InputToken
  {
    EPPToken token;
    bool line_start;
    const std::string * file;
    uint32_t line = 0;
    uint32_t column = 0;
  };

  struct TokenCursor
  {
    explicit TokenCursor(Preprocessor * owner);
    virtual EPPToken get();
    EPPToken get_collapsed();
    bool at_line_start() const;
    void clear_token_line_start();
    void reset_line_state();
    bool complete() const;

    Preprocessor * owner;
    bool has_lookahead;
    InputToken lookahead;
    bool line_start;
    bool token_line_start;
    std::string token_file;
    const std::string * token_file_ptr;
    uint32_t token_line;
    uint32_t token_column;

  protected:
    EPPToken resume_injected();
    EPPToken read_file();
    void update_line_state(const EPPToken & token);
  };

  struct RawInputSource : IPPTokenSource
  {
    explicit RawInputSource(TokenCursor * cursor) : cursor(cursor) {}
    EPPToken get() override;

    TokenCursor * cursor;
  };

  struct InjectedTokens
  {
    std::deque<EPPToken> tokens;
    bool has_resume_state;
    DirectiveState resume_state;
  };

  DirectiveState directive_state;
  DefinedState defined_state;
  EPPTokenType last_type;

  Macroizer macroizer;
  Calculator calculator;
  std::string error_msg;
  std::string warning_msg;
  std::vector<std::string> include_paths;
  std::vector<std::string> system_include_paths;
  std::vector<std::string> dependency_files_;
  std::set<std::string> dependency_file_set_;
  std::set<FileId> file_ids;
  bool enable_exceptions;
  bool emit_insignificant_whitespace;
  std::vector<IfStateStruct> ifstates;
  std::vector< std::unique_ptr<FileState> > files;
  TokenCursor cursor;
  RawInputSource raw_input;
  std::vector<InjectedTokens> injections;
  std::vector<EPPToken> directive_tokens;
  std::size_t pragma_op_nesting;
  unsigned long long counter_macro_value;
  bool last_output_from_macro_expansion_;
  std::string macro_expansion_source_file_;
  uint32_t macro_expansion_source_line_;
  uint32_t macro_expansion_source_column_;

  void note_dependency(const std::string & path, bool system);
};

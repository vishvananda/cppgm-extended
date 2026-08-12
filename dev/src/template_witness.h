#pragma once

#include <cctype>
#include <cstdlib>
#include <deque>
#include <map>
#include <regex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "cppast_ast.h"
#include "recog_token_buffer.h"
#include "semantic_source_use.h"
#include "template_lifecycle.h"
#include "template_model.h"
#include "witness_provenance.h"

namespace cpp_decl {
struct Type;
}

namespace semantic_model {
struct ClassInfo;
struct ClassTemplateDecl;
struct FunctionBinding;
struct Scope;
struct ValueBinding;
}

namespace template_api {

namespace template_witness_detail {
struct SourceTokenIndex;
}

enum class SourceTypeMaterializationOwner : unsigned char
{
  None,
  FunctionBody,
  DeclarationType,
  StaticMemberInitializer,
  VariableTemplateInitializer
};

enum class SourceTypeMaterializationOperation : unsigned char
{
  None,
  ContainingSemanticOwner,
  SourceTypeNode,
  StaticMemberInitializer,
  VariableTemplateInitializer
};

namespace source_type_materialization_detail {
void push(SourceTypeMaterializationOwner owner,
          SourceTypeMaterializationOperation operation,
          const CppAstNode * source_root,
          const cpp_decl::TemplateIdSyntax * source_syntax,
          const void * semantic_owner,
          bool semantic_owner_committed);
void pop();
}

enum class TemplateWitnessOrigin
{
  Source,
  Closure,
};

enum class TemplateClosureReason
{
  None,
  TrackInstantiation,
  RequireDefinition,
  EnsureDefinition,
  FinalizeClass,
};

enum class TemplateWitnessTriggerKind
{
  None,
  Function,
  Class,
  Variable,
};

enum class TemplateLifecycleEventKind
{
  RequireDefinition,
  EnsureDefinition,
  FunctionInstantiation,
  ClassInstantiation,
  VariableInstantiation,
  ClassFinalization,
};

struct TemplateWitnessEntryContext
{
  TemplateWitnessOrigin origin = TemplateWitnessOrigin::Source;
  TemplateClosureReason closure_reason = TemplateClosureReason::None;
  std::string trigger_entity;
  std::string trigger_decl_location;
  bool trigger_has_template_identity = false;
  TemplateWitnessTriggerKind trigger_kind = TemplateWitnessTriggerKind::None;
  const semantic_model::FunctionBinding * semantic_trigger_function = nullptr;
};

struct TemplateLifecycleEvent
{
  TemplateWitnessEntryContext entry_context;
  TemplateLifecycleEventKind kind = TemplateLifecycleEventKind::RequireDefinition;
  TemplateLifecycleCause cause = TemplateLifecycleCause::None;
  std::string location;
  std::string entity;
  std::string decl_location;
  std::string detail;
  bool entity_has_template_identity = false;
  bool entity_is_unnamed_class = false;
  bool entity_is_function_local_class = false;
  bool entity_is_function_local_class_member = false;
  bool entity_is_constexpr_function = false;
  bool entity_is_defaulted_copy_or_move_constructor = false;
  bool entity_is_defaulted_copy_or_move_assignment = false;
  bool entity_is_explicit_instantiation_definition = false;
  bool entity_is_constructor = false;
  bool entity_is_member_function_template = false;
  bool entity_is_standard_library_builtin = false;
  bool entity_definition_materialized_by_enclosing_closure = false;
  bool public_source_required = false;
  std::string normalized_entity;
  std::string normalized_trigger_entity;
  std::string owner_entity;
  std::string trigger_owner_entity;
  // Session-local semantic identity for a member variable. Public rendering
  // remains textual, but visibility policy compares the owner and member.
  const cpp_decl::Type * semantic_owner_type = nullptr;
  std::string semantic_member_name;
  bool template_related = false;
  bool directly_owned = true;
  bool cross_owner_dependency = false;
};

struct TemplateWitnessSourceRange
{
  std::string file;
  int begin_line = 0;
  int end_line = 0;
  int first_body_column = 1;
  std::vector<std::string> parameter_names;
};

struct TemplateWitnessTemplateHeaderContext
{
  std::string file;
  int begin_line = 0;
  int begin_column = 1;
  int end_line = 0;
  int end_column = 0;
  bool class_template = false;
  std::vector<std::string> parameter_names;
};

struct PendingVariableSourceUse
{
  const void * semantic_owner = nullptr;
  semantic_source_use::SemanticSourceUse source_use;
};

struct TemplateLifecycleIdentity
{
  std::size_t entity = 0;
  std::size_t owner = 0;
  std::size_t instantiation = 0;
  std::size_t event_anchor = 0;
  std::size_t detail = 0;
  TemplateLifecycleCause cause = TemplateLifecycleCause::None;
  unsigned int flags = 0;

  bool operator==(const TemplateLifecycleIdentity & other) const
  {
    return entity == other.entity && owner == other.owner &&
        instantiation == other.instantiation &&
        event_anchor == other.event_anchor && detail == other.detail &&
        cause == other.cause && flags == other.flags;
  }
};

struct TemplateLifecycleIdentityHash
{
  std::size_t operator()(const TemplateLifecycleIdentity & identity) const
  {
    std::size_t result = identity.entity;
    const std::size_t components[] = {
        identity.owner, identity.instantiation, identity.event_anchor,
        identity.detail, static_cast<std::size_t>(identity.cause),
        identity.flags};
    for(std::size_t i = 0; i < sizeof(components) / sizeof(components[0]); ++i) {
      result ^= components[i] + 0x9e3779b9u +
          (result << 6) + (result >> 2);
    }
    return result;
  }
};

enum TemplateWitnessValueStateFlag
{
  WitnessValueMemberSourceCaptureNoted = 1u << 0,
  WitnessValueStaticDefinitionReplayed = 1u << 1,
  WitnessValueStaticInitializerReplayed = 1u << 2,
  WitnessValueStaticDefinitionSourceCaptured = 1u << 3,
};

struct TemplateWitnessSession
{
  enum SourceValueDependency : unsigned char
  {
    SVD_UNKNOWN,
    SVD_FIXED,
    SVD_DEPENDENT
  };

  struct ParameterizedClassSourceOccurrence
  {
    const semantic_model::ClassTemplateDecl * origin = nullptr;
    cpp_decl::TemplateIdSourceDependency dependency =
        cpp_decl::TemplateIdSourceDependency::Unknown;
  };

  struct RetainedEnumValueBinding
  {
    const semantic_model::ValueBinding * binding = nullptr;
    const semantic_model::Scope * scope = nullptr;
  };

  std::string primary_source_file;
  std::vector<TemplateLifecycleEvent> lifecycle_events;
  std::unordered_map<TemplateLifecycleIdentity,
                     unsigned int,
                     TemplateLifecycleIdentityHash> lifecycle_transition_states;
  std::unordered_set<const semantic_model::FunctionBinding *>
      public_source_definition_dependencies;
  std::unordered_map<const semantic_model::ValueBinding *, unsigned int>
      value_state_flags;
  std::unordered_map<
      const semantic_model::FunctionBinding *,
      std::vector<template_model::TemplateValueDependency> >
      signature_value_dependencies;
  std::unordered_set<const semantic_model::ClassInfo *>
      source_capture_header_instantiation_tracked;
  semantic_source_use::SemanticSourceUseTable source_use_table;
  std::vector<PendingVariableSourceUse> pending_variable_source_uses;
  std::vector<std::string> inline_namespace_names;
  std::vector<TemplateWitnessSourceRange> template_body_ranges;
  std::vector<TemplateWitnessTemplateHeaderContext> template_header_contexts;
  std::unordered_map<const semantic_model::ValueBinding *,
                     SourceValueDependency> source_value_dependencies;
  std::map<std::pair<const semantic_model::ClassTemplateDecl *, std::string>,
           SourceValueDependency> source_class_value_dependencies;
  std::map<std::pair<const semantic_model::ClassTemplateDecl *, std::string>,
           SourceValueDependency> source_class_type_dependencies;
  std::unordered_set<uint32_t> fixed_class_argument_occurrences;
  std::unordered_map<uint32_t, ParameterizedClassSourceOccurrence>
      class_source_occurrences;
  std::map<std::pair<const semantic_model::ClassInfo *, std::string>,
           const semantic_model::ClassTemplateDecl *>
      reference_reset_class_template_sources;
  std::unordered_map<const semantic_model::ClassTemplateDecl *,
                     const semantic_model::ClassTemplateDecl *>
      reference_reset_replacement_sources;
  std::map<std::pair<std::string, long long>, RetainedEnumValueBinding>
      retained_enum_value_bindings;
};

struct TemplateWitnessContext
{
  TemplateWitnessSession * session = nullptr;
  semantic_source_use::SemanticSourceUseTable * source_use_table = nullptr;
  TemplateWitnessEntryContext entry_context;
  std::string public_use_location;
  bool public_source_use_active = false;
  std::string primary_source_file;
  const IRecogTokenSequence * token_sequence = nullptr;
  const SourceLocationTable * source_locations = nullptr;
  const template_witness_detail::SourceTokenIndex * source_token_index = nullptr;
};

std::string normalize_template_witness_source_location(
    const std::string & location);

class ScopedTemplateWitnessSession
{
public:
  explicit ScopedTemplateWitnessSession(TemplateWitnessSession * session);
  ~ScopedTemplateWitnessSession();

private:
  TemplateWitnessSession * previous_;
};

class ScopedTemplateWitnessEntryContext
{
public:
  ScopedTemplateWitnessEntryContext();
  explicit ScopedTemplateWitnessEntryContext(const TemplateWitnessEntryContext & context);
  ~ScopedTemplateWitnessEntryContext();

private:
  bool active_;
};

class ScopedTemplateWitnessSourceTypeLookup
{
public:
  explicit ScopedTemplateWitnessSourceTypeLookup(bool active = true);
  ~ScopedTemplateWitnessSourceTypeLookup();

private:
  bool active_;
};

class ScopedTemplateWitnessQualifiedMemberTypeLookup
{
public:
  explicit ScopedTemplateWitnessQualifiedMemberTypeLookup(bool active = true);
  ~ScopedTemplateWitnessQualifiedMemberTypeLookup();

private:
  bool active_;
};

class ScopedTemplateWitnessTypeLookupPause
{
public:
  explicit ScopedTemplateWitnessTypeLookupPause(bool active = true);
  ~ScopedTemplateWitnessTypeLookupPause();

private:
  bool active_;
};

class ScopedTemplateWitnessSourceCapturePause
{
public:
  explicit ScopedTemplateWitnessSourceCapturePause(bool active = true);
  ~ScopedTemplateWitnessSourceCapturePause();

private:
  bool active_;
};

class ScopedTemplateWitnessLifecyclePause
{
public:
  explicit ScopedTemplateWitnessLifecyclePause(bool active = true);
  ~ScopedTemplateWitnessLifecyclePause();

private:
  bool active_;
};

class ScopedTemplateWitnessFunctionCallSourceCapturePause
{
public:
  explicit ScopedTemplateWitnessFunctionCallSourceCapturePause(bool active = true);
  ~ScopedTemplateWitnessFunctionCallSourceCapturePause();

private:
  bool active_;
};

class ScopedTemplateWitnessDeclvalCallSourceCapturePause
{
public:
  explicit ScopedTemplateWitnessDeclvalCallSourceCapturePause(bool active = true);
  ~ScopedTemplateWitnessDeclvalCallSourceCapturePause();

private:
  bool active_;
};

namespace template_witness_detail {

struct ParsedSourceLocation
{
  bool valid = false;
  std::string file;
  int line = 0;
  int column = 0;
};

inline ParsedSourceLocation parse_source_location(const std::string & text)
{
  ParsedSourceLocation parsed;
  const std::size_t last_colon = text.rfind(':');
  if(last_colon == std::string::npos) {
    return parsed;
  }
  const std::size_t second_colon = text.rfind(':', last_colon - 1);
  if(second_colon == std::string::npos) {
    return parsed;
  }
  parsed.file = text.substr(0, second_colon);
  parsed.line = std::atoi(text.substr(second_colon + 1,
                                      last_colon - second_colon - 1).c_str());
  parsed.column = std::atoi(text.substr(last_colon + 1).c_str());
  parsed.valid = !parsed.file.empty();
  return parsed;
}

inline bool source_location_at_or_after(const ParsedSourceLocation & candidate,
                                        const ParsedSourceLocation & base)
{
  if(!candidate.valid || !base.valid || candidate.file != base.file) {
    return false;
  }
  if(candidate.line != base.line) {
    return candidate.line > base.line;
  }
  return candidate.column >= base.column;
}

inline std::string source_location_key(const std::string & file,
                                       int line,
                                       int column)
{
  return file + "\n" + std::to_string(line) + "\n" + std::to_string(column);
}

inline std::string source_location_key_for_location_id(
    const SourceLocationTable & source_locations,
    uint32_t location_id)
{
  if(location_id == 0 ||
     location_id >= source_locations.locations.size()) {
    return std::string();
  }
  const SourceLocation & location = source_locations.locations[location_id];
  if(location.file_index >= source_locations.files.size()) {
    return std::string();
  }
  return source_location_key(source_locations.files[location.file_index],
                             static_cast<int>(location.line),
                             static_cast<int>(location.column));
}

inline bool source_location_matches_parsed(
    const SourceLocationTable & source_locations,
    uint32_t location_id,
    const ParsedSourceLocation & target)
{
  if(!target.valid ||
     location_id == 0 ||
     location_id >= source_locations.locations.size()) {
    return false;
  }
  const SourceLocation & location = source_locations.locations[location_id];
  return location.file_index < source_locations.files.size() &&
      source_locations.files[location.file_index] == target.file &&
      static_cast<int>(location.line) == target.line &&
      static_cast<int>(location.column) == target.column;
}

inline bool source_location_is_at_or_after_parsed(
    const SourceLocationTable & source_locations,
    uint32_t location_id,
    const ParsedSourceLocation & base)
{
  if(!base.valid ||
     location_id == 0 ||
     location_id >= source_locations.locations.size()) {
    return false;
  }
  const SourceLocation & location = source_locations.locations[location_id];
  if(location.file_index >= source_locations.files.size() ||
     source_locations.files[location.file_index] != base.file) {
    return false;
  }
  if(static_cast<int>(location.line) != base.line) {
    return static_cast<int>(location.line) > base.line;
  }
  return static_cast<int>(location.column) >= base.column;
}

inline std::string source_location_for_location_id_raw(
    const SourceLocationTable & source_locations,
    uint32_t location_id)
{
  if(location_id == 0 ||
     location_id >= source_locations.locations.size()) {
    return std::string();
  }
  const SourceLocation & location = source_locations.locations[location_id];
  if(location.file_index >= source_locations.files.size()) {
    return std::string();
  }
  return source_locations.files[location.file_index] + ":" +
      std::to_string(location.line) + ":" + std::to_string(location.column);
}

struct SourceTokenIndex
{
  bool source_location_points_at_identifier_token(
      const TemplateWitnessContext & ctx,
      const ParsedSourceLocation & target,
      const std::string & identifier) const
  {
    std::size_t index = 0;
    return find_identifier_at_location(ctx, target, identifier, index);
  }

  bool source_location_identifier_token_followed_by(
      const TemplateWitnessContext & ctx,
      const ParsedSourceLocation & target,
      const std::string & identifier,
      const std::string & following_source) const
  {
    std::size_t index = 0;
    if(!find_identifier_at_location(ctx, target, identifier, index)) {
      return false;
    }
    const std::size_t token_count = ctx.token_sequence->size();
    const std::size_t next_index = index + 1;
    if(next_index >= token_count) {
      return false;
    }
    const RecogToken & next = ctx.token_sequence->peek(next_index);
    if(next.is_eof() || next.source != following_source || next.location_id == 0) {
      return false;
    }
    if(next.location_id >= ctx.source_locations->locations.size()) {
      return false;
    }
    const SourceLocation & location =
        ctx.source_locations->locations[next.location_id];
    return location.file_index < ctx.source_locations->files.size() &&
        ctx.source_locations->files[location.file_index] == target.file &&
        static_cast<int>(location.line) == target.line &&
        static_cast<int>(location.column) ==
            target.column + static_cast<int>(identifier.size());
  }

  bool source_location_id_points_at_identifier_token(
      const TemplateWitnessContext & ctx,
      uint32_t location_id,
      const std::string & identifier) const
  {
    std::size_t index = 0;
    return find_identifier_at_location_id(ctx, location_id, identifier, index);
  }

  bool source_location_id_identifier_token_followed_by(
      const TemplateWitnessContext & ctx,
      uint32_t location_id,
      const std::string & identifier,
      const std::string & following_source) const
  {
    std::size_t index = 0;
    if(!find_identifier_at_location_id(ctx, location_id, identifier, index)) {
      return false;
    }
    const std::size_t token_count = ctx.token_sequence->size();
    const std::size_t next_index = index + 1;
    if(next_index >= token_count) {
      return false;
    }
    const RecogToken & next = ctx.token_sequence->peek(next_index);
    if(next.is_eof() || next.source != following_source || next.location_id == 0) {
      return false;
    }
    if(location_id >= ctx.source_locations->locations.size() ||
       next.location_id >= ctx.source_locations->locations.size()) {
      return false;
    }
    const SourceLocation & location =
        ctx.source_locations->locations[location_id];
    const SourceLocation & next_location =
        ctx.source_locations->locations[next.location_id];
    return location.file_index < ctx.source_locations->files.size() &&
        next_location.file_index < ctx.source_locations->files.size() &&
        next_location.file_index == location.file_index &&
        next_location.line == location.line &&
        static_cast<int>(next_location.column) ==
            static_cast<int>(location.column) +
            static_cast<int>(identifier.size());
  }

  std::string source_location_for_identifier_token_on_or_after(
      const TemplateWitnessContext & ctx,
      const ParsedSourceLocation & base,
      const std::string & identifier,
      bool same_line_only,
      bool require_template_open) const
  {
    ensure_built(ctx);
    const std::unordered_map<std::string,
                             std::vector<std::size_t> >::const_iterator found =
        identifier_token_indices_.find(identifier);
    if(found == identifier_token_indices_.end()) {
      return std::string();
    }
    const std::vector<std::size_t> & indices = found->second;
    const std::size_t token_count = ctx.token_sequence->size();
    for(std::size_t n = 0; n < indices.size(); ++n) {
      const std::size_t index = indices[n];
      if(index >= token_count) {
        continue;
      }
      const RecogToken & token = ctx.token_sequence->peek(index);
      if(!token.is_identifier() ||
         token.source != identifier ||
         !source_location_is_at_or_after_parsed(*ctx.source_locations,
                                                token.location_id,
                                                base)) {
        continue;
      }
      const SourceLocation & location =
          ctx.source_locations->locations[token.location_id];
      if(same_line_only && static_cast<int>(location.line) != base.line) {
        continue;
      }
      if(require_template_open) {
        const std::size_t next_index = index + 1;
        if(next_index >= token_count) {
          return std::string();
        }
        const RecogToken & next = ctx.token_sequence->peek(next_index);
        if(next.is_eof() || next.source != "<") {
          continue;
        }
      }
      const std::string location_text =
          source_location_for_location_id_raw(*ctx.source_locations,
                                              token.location_id);
      return location_text.empty() ? std::string() :
          std::string(" at ") + location_text;
    }
    return std::string();
  }

  bool source_location_line_mentions_qualified_member_token(
      const TemplateWitnessContext & ctx,
      const ParsedSourceLocation & base,
      const std::string & member_name) const
  {
    if(member_name.empty() || !base.valid ||
       !(ctx.token_sequence && ctx.source_locations)) {
      return false;
    }
    ensure_built(ctx);
    const std::unordered_map<std::string,
                             std::vector<std::size_t> >::const_iterator found =
        identifier_token_indices_.find(member_name);
    if(found == identifier_token_indices_.end()) {
      return false;
    }
    const std::vector<std::size_t> & indices = found->second;
    const std::size_t token_count = ctx.token_sequence->size();
    for(std::size_t n = 0; n < indices.size(); ++n) {
      const std::size_t index = indices[n];
      if(index == 0 || index >= token_count) {
        continue;
      }
      const RecogToken & token = ctx.token_sequence->peek(index);
      if(!token.is_identifier() ||
         token.source != member_name ||
         token.location_id == 0 ||
         token.location_id >= ctx.source_locations->locations.size()) {
        continue;
      }
      const SourceLocation & location =
          ctx.source_locations->locations[token.location_id];
      if(location.file_index >= ctx.source_locations->files.size() ||
         ctx.source_locations->files[location.file_index] != base.file ||
         static_cast<int>(location.line) != base.line) {
        continue;
      }
      const RecogToken & previous = ctx.token_sequence->peek(index - 1);
      if(previous.is_eof() ||
         previous.source != "::" ||
         previous.location_id == 0 ||
         previous.location_id >= ctx.source_locations->locations.size()) {
        continue;
      }
      const SourceLocation & previous_location =
          ctx.source_locations->locations[previous.location_id];
      if(previous_location.file_index < ctx.source_locations->files.size() &&
         ctx.source_locations->files[previous_location.file_index] == base.file &&
         static_cast<int>(previous_location.line) == base.line &&
         static_cast<int>(previous_location.column) >= base.column &&
         static_cast<int>(location.column) ==
             static_cast<int>(previous_location.column) + 2) {
        return true;
      }
    }
    return false;
  }

private:
  void ensure_built(const TemplateWitnessContext & ctx) const
  {
    if(built_ &&
       indexed_token_sequence_ == ctx.token_sequence &&
       indexed_source_locations_ == ctx.source_locations) {
      return;
    }
    built_ = true;
    indexed_token_sequence_ = ctx.token_sequence;
    indexed_source_locations_ = ctx.source_locations;
    identifier_token_indices_.clear();
    if(!(ctx.token_sequence && ctx.source_locations)) {
      return;
    }

    const std::size_t token_count = ctx.token_sequence->size();
    for(std::size_t i = 0; i < token_count; ++i) {
      const RecogToken & token = ctx.token_sequence->peek(i);
      if(token.is_eof()) {
        break;
      }
      if(token.location_id == 0 || !token.is_identifier()) {
        continue;
      }
      identifier_token_indices_[token.source].push_back(i);
    }
  }

  bool find_identifier_at_location(const TemplateWitnessContext & ctx,
                                   const ParsedSourceLocation & target,
                                   const std::string & identifier,
                                   std::size_t & out_index) const
  {
    if(identifier.empty() || !target.valid ||
       !(ctx.token_sequence && ctx.source_locations)) {
      return false;
    }
    ensure_built(ctx);
    const std::unordered_map<std::string,
                             std::vector<std::size_t> >::const_iterator found =
        identifier_token_indices_.find(identifier);
    if(found == identifier_token_indices_.end()) {
      return false;
    }
    const std::vector<std::size_t> & indices = found->second;
    const std::size_t token_count = ctx.token_sequence->size();
    for(std::size_t n = 0; n < indices.size(); ++n) {
      const std::size_t index = indices[n];
      if(index >= token_count) {
        continue;
      }
      const RecogToken & token = ctx.token_sequence->peek(index);
      if(token.is_identifier() &&
         token.source == identifier &&
         source_location_matches_parsed(*ctx.source_locations,
                                        token.location_id,
                                        target)) {
        out_index = index;
        return true;
      }
    }
    return false;
  }

  bool find_identifier_at_location_id(const TemplateWitnessContext & ctx,
                                      uint32_t location_id,
                                      const std::string & identifier,
                                      std::size_t & out_index) const
  {
    if(identifier.empty() ||
       location_id == 0 ||
       !(ctx.token_sequence && ctx.source_locations)) {
      return false;
    }
    ensure_built(ctx);
    const std::unordered_map<std::string,
                             std::vector<std::size_t> >::const_iterator found =
        identifier_token_indices_.find(identifier);
    if(found == identifier_token_indices_.end()) {
      return false;
    }
    const std::vector<std::size_t> & indices = found->second;
    const std::size_t token_count = ctx.token_sequence->size();
    for(std::size_t n = 0; n < indices.size(); ++n) {
      const std::size_t index = indices[n];
      if(index >= token_count) {
        continue;
      }
      const RecogToken & token = ctx.token_sequence->peek(index);
      if(token.is_identifier() &&
         token.source == identifier &&
         token.location_id == location_id) {
        out_index = index;
        return true;
      }
    }
    return false;
  }

  mutable bool built_ = false;
  mutable const IRecogTokenSequence * indexed_token_sequence_ = nullptr;
  mutable const SourceLocationTable * indexed_source_locations_ = nullptr;
  mutable std::unordered_map<std::string, std::vector<std::size_t> >
      identifier_token_indices_;
};

inline std::string source_location_for_identifier_token_on_or_after(
    const TemplateWitnessContext & ctx,
    const std::string & base_location,
    const std::string & identifier,
    bool same_line_only = false,
    bool require_template_open = false)
{
  if(identifier.empty() || !(ctx.token_sequence && ctx.source_locations)) {
    return std::string();
  }
  const ParsedSourceLocation base =
      parse_source_location(normalize_template_witness_source_location(base_location));
  if(!base.valid) {
    return std::string();
  }
  if(ctx.source_token_index != nullptr) {
    return ctx.source_token_index->source_location_for_identifier_token_on_or_after(
        ctx,
        base,
        identifier,
        same_line_only,
        require_template_open);
  }

  const std::size_t token_count = ctx.token_sequence->size();
  bool reached_base = false;
  for(std::size_t i = 0; i < token_count; ++i) {
    const RecogToken & token = ctx.token_sequence->peek(i);
    if(token.is_eof()) {
      break;
    }
    if(token.location_id == 0) {
      continue;
    }
    const std::string token_location =
        ctx.source_locations->describe(token.location_id);
    const ParsedSourceLocation parsed =
        parse_source_location(normalize_template_witness_source_location(
            token_location));
    if(same_line_only &&
       parsed.valid &&
       parsed.file == base.file &&
       parsed.line > base.line) {
      break;
    }
    if(!reached_base) {
      reached_base = source_location_at_or_after(parsed, base);
      if(!reached_base) {
        continue;
      }
    }
    if(same_line_only && parsed.line != base.line) {
      continue;
    }
    if(token.is_identifier() && token.source == identifier) {
      if(require_template_open) {
        const std::size_t next_index = i + 1;
        if(next_index >= token_count) {
          return std::string();
        }
        const RecogToken & next = ctx.token_sequence->peek(next_index);
        if(next.is_eof() || next.source != "<") {
          continue;
        }
      }
      return std::string(" at ") + token_location;
    }
  }
  return std::string();
}

inline bool source_location_points_at_identifier_token(
    const TemplateWitnessContext & ctx,
    const std::string & location,
    const std::string & identifier)
{
  if(identifier.empty() || !(ctx.token_sequence && ctx.source_locations)) {
    return false;
  }
  const ParsedSourceLocation target =
      parse_source_location(normalize_template_witness_source_location(location));
  if(!target.valid) {
    return false;
  }
  if(ctx.source_token_index != nullptr) {
    return ctx.source_token_index->source_location_points_at_identifier_token(
        ctx,
        target,
        identifier);
  }

  const std::size_t token_count = ctx.token_sequence->size();
  for(std::size_t i = 0; i < token_count; ++i) {
    const RecogToken & token = ctx.token_sequence->peek(i);
    if(token.is_eof()) {
      break;
    }
    if(token.location_id == 0 || !token.is_identifier() ||
       token.source != identifier) {
      continue;
    }
    const ParsedSourceLocation parsed =
        parse_source_location(normalize_template_witness_source_location(
            ctx.source_locations->describe(token.location_id)));
    if(parsed.valid &&
       parsed.file == target.file &&
       parsed.line == target.line &&
       parsed.column == target.column) {
      return true;
    }
  }
  return false;
}

inline bool source_location_id_points_at_identifier_token(
    const TemplateWitnessContext & ctx,
    uint32_t location_id,
    const std::string & identifier)
{
  if(identifier.empty() ||
     location_id == 0 ||
     !(ctx.token_sequence && ctx.source_locations)) {
    return false;
  }
  if(ctx.source_token_index != nullptr) {
    return ctx.source_token_index->source_location_id_points_at_identifier_token(
        ctx,
        location_id,
        identifier);
  }

  const std::size_t token_count = ctx.token_sequence->size();
  for(std::size_t i = 0; i < token_count; ++i) {
    const RecogToken & token = ctx.token_sequence->peek(i);
    if(token.is_eof()) {
      break;
    }
    if(token.location_id == location_id &&
       token.is_identifier() &&
       token.source == identifier) {
      return true;
    }
  }
  return false;
}

inline bool source_location_identifier_token_followed_by(
    const TemplateWitnessContext & ctx,
    const std::string & location,
    const std::string & identifier,
    const std::string & following_source)
{
  if(identifier.empty() || following_source.empty() ||
     !(ctx.token_sequence && ctx.source_locations)) {
    return false;
  }
  const ParsedSourceLocation target =
      parse_source_location(normalize_template_witness_source_location(location));
  if(!target.valid) {
    return false;
  }
  if(ctx.source_token_index != nullptr) {
    return ctx.source_token_index->source_location_identifier_token_followed_by(
        ctx,
        target,
        identifier,
        following_source);
  }

  const std::size_t token_count = ctx.token_sequence->size();
  for(std::size_t i = 0; i < token_count; ++i) {
    const RecogToken & token = ctx.token_sequence->peek(i);
    if(token.is_eof()) {
      break;
    }
    if(token.location_id == 0 || !token.is_identifier() ||
       token.source != identifier) {
      continue;
    }
    const ParsedSourceLocation parsed =
        parse_source_location(normalize_template_witness_source_location(
            ctx.source_locations->describe(token.location_id)));
    if(!(parsed.valid &&
         parsed.file == target.file &&
         parsed.line == target.line &&
         parsed.column == target.column)) {
      continue;
    }
    const std::size_t next_index = i + 1;
    if(next_index >= token_count) {
      return false;
    }
    const RecogToken & next = ctx.token_sequence->peek(next_index);
    if(next.is_eof() || next.source != following_source ||
       next.location_id == 0) {
      return false;
    }
    const ParsedSourceLocation next_location =
        parse_source_location(normalize_template_witness_source_location(
            ctx.source_locations->describe(next.location_id)));
    return next_location.valid &&
        next_location.file == target.file &&
        next_location.line == target.line &&
        next_location.column ==
            target.column + static_cast<int>(identifier.size());
  }
  return false;
}

inline bool source_location_id_identifier_token_followed_by(
    const TemplateWitnessContext & ctx,
    uint32_t location_id,
    const std::string & identifier,
    const std::string & following_source)
{
  if(identifier.empty() ||
     following_source.empty() ||
     location_id == 0 ||
     !(ctx.token_sequence && ctx.source_locations)) {
    return false;
  }
  if(ctx.source_token_index != nullptr) {
    return ctx.source_token_index->source_location_id_identifier_token_followed_by(
        ctx,
        location_id,
        identifier,
        following_source);
  }

  const std::size_t token_count = ctx.token_sequence->size();
  for(std::size_t i = 0; i < token_count; ++i) {
    const RecogToken & token = ctx.token_sequence->peek(i);
    if(token.is_eof()) {
      break;
    }
    if(token.location_id != location_id ||
       !token.is_identifier() ||
       token.source != identifier) {
      continue;
    }
    const std::size_t next_index = i + 1;
    if(next_index >= token_count) {
      return false;
    }
    const RecogToken & next = ctx.token_sequence->peek(next_index);
    if(next.is_eof() || next.source != following_source || next.location_id == 0) {
      return false;
    }
    if(location_id >= ctx.source_locations->locations.size() ||
       next.location_id >= ctx.source_locations->locations.size()) {
      return false;
    }
    const SourceLocation & location =
        ctx.source_locations->locations[location_id];
    const SourceLocation & next_location =
        ctx.source_locations->locations[next.location_id];
    return location.file_index < ctx.source_locations->files.size() &&
        next_location.file_index < ctx.source_locations->files.size() &&
        next_location.file_index == location.file_index &&
        next_location.line == location.line &&
        static_cast<int>(next_location.column) ==
            static_cast<int>(location.column) +
            static_cast<int>(identifier.size());
  }
  return false;
}

inline bool source_location_line_mentions_qualified_member_token(
    const TemplateWitnessContext & ctx,
    const std::string & location,
    const std::string & member_name)
{
  if(member_name.empty() || !(ctx.token_sequence && ctx.source_locations)) {
    return false;
  }
  const ParsedSourceLocation base =
      parse_source_location(normalize_template_witness_source_location(location));
  if(!base.valid) {
    return false;
  }
  if(ctx.source_token_index != nullptr) {
    return ctx.source_token_index->
        source_location_line_mentions_qualified_member_token(ctx,
                                                            base,
                                                            member_name);
  }

  const std::size_t token_count = ctx.token_sequence->size();
  bool reached_base = false;
  for(std::size_t i = 0; i < token_count; ++i) {
    const RecogToken & token = ctx.token_sequence->peek(i);
    if(token.is_eof()) {
      break;
    }
    if(token.location_id == 0) {
      continue;
    }
    const ParsedSourceLocation parsed =
        parse_source_location(normalize_template_witness_source_location(
            ctx.source_locations->describe(token.location_id)));
    if(parsed.valid && parsed.file == base.file && parsed.line > base.line) {
      break;
    }
    if(!reached_base) {
      reached_base = source_location_at_or_after(parsed, base);
      if(!reached_base) {
        continue;
      }
    }
    if(!(parsed.valid && parsed.file == base.file && parsed.line == base.line)) {
      continue;
    }
    if(token.source != "::") {
      continue;
    }
    const std::size_t next_index = i + 1;
    if(next_index >= token_count) {
      return false;
    }
    const RecogToken & next = ctx.token_sequence->peek(next_index);
    if(next.is_eof() || next.source != member_name ||
       next.location_id == 0) {
      continue;
    }
    const ParsedSourceLocation next_location =
        parse_source_location(normalize_template_witness_source_location(
            ctx.source_locations->describe(next.location_id)));
    if(next_location.valid &&
       next_location.file == base.file &&
       next_location.line == base.line &&
       next_location.column == parsed.column + 2) {
      return true;
    }
  }
  return false;
}

inline std::string prefer_later_source_location(const std::string & first,
                                                const std::string & second)
{
  if(first.empty()) {
    return second;
  }
  if(second.empty()) {
    return first;
  }

  const ParsedSourceLocation parsed_first = parse_source_location(first);
  const ParsedSourceLocation parsed_second = parse_source_location(second);
  if(!parsed_first.valid || !parsed_second.valid ||
     parsed_first.file != parsed_second.file) {
    return first;
  }
  if(parsed_second.line > parsed_first.line) {
    return second;
  }
  if(parsed_second.line < parsed_first.line) {
    return first;
  }
  return parsed_second.column >= parsed_first.column ? second : first;
}

inline std::string prefer_earlier_source_location(const std::string & first,
                                                  const std::string & second)
{
  if(first.empty()) {
    return second;
  }
  if(second.empty()) {
    return first;
  }

  const ParsedSourceLocation parsed_first = parse_source_location(first);
  const ParsedSourceLocation parsed_second = parse_source_location(second);
  if(!parsed_first.valid || !parsed_second.valid ||
     parsed_first.file != parsed_second.file) {
    return first;
  }
  if(parsed_second.line < parsed_first.line) {
    return second;
  }
  if(parsed_second.line > parsed_first.line) {
    return first;
  }
  return parsed_second.column < parsed_first.column ? second : first;
}

inline std::string source_location_for_token_index(const TemplateWitnessContext & ctx,
                                                   std::size_t index)
{
  if(!(ctx.source_locations && ctx.token_sequence)) {
    return std::string();
  }

  const RecogToken & token = ctx.token_sequence->peek(index);
  if(token.location_id == 0) {
    return std::string();
  }
  const std::string location = ctx.source_locations->describe(token.location_id);
  if(location.empty() || location == "<unknown>") {
    return std::string();
  }
  return std::string(" at ") + location;
}

inline std::string source_location_for_location_id(const TemplateWitnessContext & ctx,
                                                  uint32_t location_id)
{
  if(!(ctx.source_locations && location_id != 0)) {
    return std::string();
  }
  const std::string location = ctx.source_locations->describe(location_id);
  if(location.empty() || location == "<unknown>") {
    return std::string();
  }
  return std::string(" at ") + location;
}

inline std::string source_location_for_ast_node_start(
    const TemplateWitnessContext & ctx,
    const CppAstNode & node)
{
  const std::string location =
      source_location_for_location_id(ctx, node.source_location_id);
  if(!location.empty()) {
    return location;
  }
  return source_location_for_token_index(ctx, node.token_start);
}

inline std::string preferred_fragment_use_location_recursive(
    const TemplateWitnessContext & ctx,
    const CppAstNode & node)
{
  std::string best = source_location_for_ast_node_start(ctx, node);
  for(std::size_t i = 0; i < node.children.size(); ++i) {
    best = prefer_later_source_location(
        best,
        preferred_fragment_use_location_recursive(ctx, node.children[i]));
  }
  return best;
}

inline void replace_all(std::string & text,
                        const std::string & needle,
                        const std::string & replacement)
{
  if(needle.empty()) {
    return;
  }
  std::string::size_type pos = 0;
  while((pos = text.find(needle, pos)) != std::string::npos) {
    text.replace(pos, needle.size(), replacement);
    pos += replacement.size();
  }
}

inline std::string normalize_compact_type_layout(const std::string & text)
{
  static const std::regex pointer_suffix_regex("([A-Za-z_0-9>\\)])\\*");
  static const std::regex rvalue_ref_suffix_regex("([A-Za-z_0-9>\\)])&&");
  static const std::regex lvalue_ref_suffix_regex("([A-Za-z_0-9>\\)])&");
  static const std::regex call_paren_regex("([A-Za-z_0-9>])\\(");
  static const std::regex pointer_const_regex("\\*\\s+const\\b");
  static const std::regex pointer_volatile_regex("\\*\\s+volatile\\b");
  static const std::regex pointer_rvalue_ref_regex("\\*\\s+&&");
  static const std::regex pointer_lvalue_ref_regex("\\*\\s+&");
  std::string out = text;
  out = std::regex_replace(out, pointer_suffix_regex, "$1 *");
  out = std::regex_replace(out, rvalue_ref_suffix_regex, "$1 &&");
  out = std::regex_replace(out, lvalue_ref_suffix_regex, "$1 &");
  out = std::regex_replace(out, call_paren_regex, "$1 (");
  replace_all(out, "operator (", "operator(");
  out = std::regex_replace(out, pointer_const_regex, "*const");
  out = std::regex_replace(out, pointer_volatile_regex, "*volatile");
  out = std::regex_replace(out, pointer_rvalue_ref_regex, "*&&");
  out = std::regex_replace(out, pointer_lvalue_ref_regex, "*&");
  return out;
}

inline std::string normalize_conversion_operator_type_names(
    const std::string & text)
{
  std::string out = text;
  const char * prefixes[] = { "operator struct ", "operator class " };
  for(std::size_t prefix_index = 0; prefix_index < 2; ++prefix_index) {
    const std::string prefix(prefixes[prefix_index]);
    std::string::size_type pos = 0;
    while((pos = out.find(prefix, pos)) != std::string::npos) {
      const std::string::size_type name_start = pos + prefix.size();
      std::string::size_type name_end = name_start;
      while(name_end < out.size()) {
        const char ch = out[name_end];
        if(std::isalnum(static_cast<unsigned char>(ch)) ||
           ch == '_' || ch == ':') {
          ++name_end;
          continue;
        }
        break;
      }
      if(name_end == name_start) {
        pos += prefix.size();
        continue;
      }
      std::string::size_type erase_end = name_end;
      if(erase_end < out.size() && out[erase_end] == '<') {
        int depth = 0;
        while(erase_end < out.size()) {
          if(out[erase_end] == '<') {
            ++depth;
          } else if(out[erase_end] == '>') {
            --depth;
            if(depth == 0) {
              ++erase_end;
              break;
            }
          }
          ++erase_end;
        }
      }
      const std::string name = out.substr(name_start, name_end - name_start);
      out.replace(pos, erase_end - pos, std::string("operator ") + name);
      pos += std::string("operator ").size() + name.size();
    }
  }
  return out;
}

inline std::string normalize_template_log_type_spellings(const std::string & text)
{
  static const std::regex std_inline_namespace_regex(
      "\\bstd::__(?:[0-9]+|ndk[0-9]+)::");
  static const std::regex local_regex("__local_\\d+");
  static const std::regex integer_suffix_regex("\\b([0-9]+)[uUlL]+\\b");
  static const std::regex const_before_indirection_regex(
      "\\b([A-Za-z_][A-Za-z0-9_:]*(?:<[^<>]*>)?)\\s+const(\\s*[*&])");
  static const std::regex const_suffix_regex(
      "\\b([A-Za-z_][A-Za-z0-9_:]*(?:<[^<>]*>)?)\\s+const\\b");
  static const std::regex double_pointer_space_regex("\\*\\s+\\*");
  static const std::regex function_scope_regex(
      "(^|::)([A-Za-z_~][A-Za-z0-9_~]*) \\(");
  std::string out = text;
  out = std::regex_replace(out, std_inline_namespace_regex, "std::");
  out = std::regex_replace(out, local_regex, "");
  replace_all(out, "unsigned long long int", "unsigned long long");
  replace_all(out, "unsigned long int", "unsigned long");
  replace_all(out, "long long int", "long long");
  replace_all(out, "long int", "long");
  replace_all(out, "short int", "short");
  replace_all(out, "unsigned int", "unsigned");
  replace_all(out, "signed int", "signed");
  out = std::regex_replace(out, integer_suffix_regex, "$1");
  out = std::regex_replace(out, const_before_indirection_regex,
                           "const $1$2");
  out = std::regex_replace(out, const_suffix_regex, "const $1");
  out = normalize_conversion_operator_type_names(out);
  while(true) {
    const std::string collapsed =
        std::regex_replace(out, double_pointer_space_regex, "**");
    if(collapsed == out) {
      break;
    }
    out = collapsed;
  }
  out = normalize_compact_type_layout(out);
  return std::regex_replace(out, function_scope_regex, "$1$2(");
}

inline std::string collapse_duplicate_owner_prefix(const std::string & entity)
{
  const std::string::size_type member_pos = entity.rfind("::");
  if(member_pos == std::string::npos) {
    return entity;
  }

  const std::string owner = entity.substr(0, member_pos);
  const std::string member = entity.substr(member_pos + 2);
  for(std::string::size_type split = owner.find("::");
      split != std::string::npos;
      split = owner.find("::", split + 2)) {
    const std::string owner_prefix = owner.substr(0, split);
    const std::string owner_suffix = owner.substr(split + 2);
    if(owner_prefix == owner_suffix) {
      return owner_suffix + "::" + member;
    }
  }
  return entity;
}

inline std::string normalize_template_log_entity(const std::string & entity)
{
  return normalize_template_log_type_spellings(
      collapse_duplicate_owner_prefix(entity));
}

inline std::string owner_entity(const std::string & normalized_entity)
{
  int angle_depth = 0;
  std::string::size_type member_pos = std::string::npos;
  for(std::string::size_type i = normalized_entity.size(); i > 1; --i) {
    const char ch = normalized_entity[i - 1];
    if(ch == '>') {
      ++angle_depth;
      continue;
    }
    if(ch == '<') {
      if(angle_depth > 0) {
        --angle_depth;
      }
      continue;
    }
    if(angle_depth == 0 &&
       ch == ':' &&
       normalized_entity[i - 2] == ':') {
      member_pos = i - 2;
      break;
    }
  }
  if(member_pos == std::string::npos) {
    return std::string();
  }
  return normalized_entity.substr(0, member_pos);
}

inline void refresh_lifecycle_event_metadata(TemplateLifecycleEvent & event)
{
  event.normalized_entity = normalize_template_log_entity(event.entity);
  event.normalized_trigger_entity =
      normalize_template_log_entity(event.entry_context.trigger_entity);
  event.owner_entity = owner_entity(event.normalized_entity);
  event.trigger_owner_entity = owner_entity(event.normalized_trigger_entity);
  event.template_related =
      event.normalized_entity.find('<') != std::string::npos ||
      event.normalized_trigger_entity.find('<') != std::string::npos ||
      (event.kind == TemplateLifecycleEventKind::ClassFinalization &&
       (event.cause ==
            TemplateLifecycleCause::ExplicitInstantiationDeclaration ||
        event.cause ==
            TemplateLifecycleCause::ExplicitInstantiationDefinition)) ||
      event.entity_has_template_identity ||
      event.entry_context.trigger_has_template_identity;
  event.directly_owned =
      event.normalized_trigger_entity.empty() ||
      event.normalized_trigger_entity == event.normalized_entity;
  event.cross_owner_dependency =
      !event.owner_entity.empty() &&
      !event.trigger_owner_entity.empty() &&
      event.owner_entity != event.trigger_owner_entity;
}

}  // namespace template_witness_detail

inline std::string preferred_fragment_use_location(const TemplateWitnessContext & ctx,
                                                   const CppAstNode & node)
{
  return template_witness_detail::preferred_fragment_use_location_recursive(ctx,
                                                                            node);
}

TemplateWitnessSession create_template_witness_session();

inline TemplateWitnessEntryContext make_template_closure_entry_context(
    TemplateClosureReason reason,
    const std::string & trigger_entity,
    const std::string & trigger_decl_location,
    bool trigger_has_template_identity = false,
    TemplateWitnessTriggerKind trigger_kind =
        TemplateWitnessTriggerKind::None)
{
  TemplateWitnessEntryContext context;
  context.origin = TemplateWitnessOrigin::Closure;
  context.closure_reason = reason;
  context.trigger_entity = trigger_entity;
  context.trigger_decl_location = trigger_decl_location;
  context.trigger_has_template_identity = trigger_has_template_identity;
  context.trigger_kind = trigger_kind;
  return context;
}

namespace template_witness_detail {

inline TemplateWitnessSession *& current_witness_session_storage()
{
  static thread_local TemplateWitnessSession * session = nullptr;
  return session;
}

inline std::deque<TemplateWitnessEntryContext> &
current_witness_entry_contexts_storage()
{
  static thread_local std::deque<TemplateWitnessEntryContext> contexts;
  return contexts;
}

inline int & current_source_type_lookup_depth_storage()
{
  static thread_local int depth = 0;
  return depth;
}

inline int & current_qualified_member_type_lookup_depth_storage()
{
  static thread_local int depth = 0;
  return depth;
}

inline int & current_type_lookup_pause_depth_storage()
{
  static thread_local int depth = 0;
  return depth;
}

inline int & current_source_capture_pause_depth_storage()
{
  static thread_local int depth = 0;
  return depth;
}

inline int & current_lifecycle_pause_depth_storage()
{
  static thread_local int depth = 0;
  return depth;
}

inline int & current_function_call_source_capture_pause_depth_storage()
{
  static thread_local int depth = 0;
  return depth;
}

inline int & current_declval_call_source_capture_pause_depth_storage()
{
  static thread_local int depth = 0;
  return depth;
}

}  // namespace template_witness_detail

inline TemplateWitnessSession * current_template_witness_session()
{
  return template_witness_detail::current_witness_session_storage();
}

bool template_witness_value_state_contains(
    const semantic_model::ValueBinding * binding,
    TemplateWitnessValueStateFlag flag);

void note_template_witness_value_state(
    const semantic_model::ValueBinding * binding,
    TemplateWitnessValueStateFlag flag);

std::vector<template_model::TemplateValueDependency> *
template_witness_signature_value_dependencies(
    const semantic_model::FunctionBinding * binding,
    bool create);

bool template_witness_source_capture_header_instantiation_tracked(
    const semantic_model::ClassInfo * info);

void set_template_witness_source_capture_header_instantiation_tracked(
    const semantic_model::ClassInfo * info,
    bool tracked);

inline semantic_source_use::SemanticSourceUseTable *
current_semantic_source_use_table()
{
  TemplateWitnessSession * session = current_template_witness_session();
  return session ? &session->source_use_table : nullptr;
}

inline bool semantic_source_use_capture_enabled()
{
  return current_semantic_source_use_table() != nullptr;
}

inline void record_template_witness_inline_namespace(
    const std::string & name)
{
  if(name.empty() || name == "<unnamed>") {
    return;
  }
  TemplateWitnessSession * session = current_template_witness_session();
  if(session == nullptr) {
    return;
  }
  for(std::size_t i = 0; i < session->inline_namespace_names.size(); ++i) {
    if(session->inline_namespace_names[i] == name) {
      return;
    }
  }
  session->inline_namespace_names.push_back(name);
}

inline void record_template_witness_template_body_range(
    const std::string & file,
    int begin_line,
    int end_line,
    int first_body_column,
    const std::vector<std::string> & parameter_names)
{
  if(file.empty() || begin_line <= 0 || end_line < begin_line) {
    return;
  }
  TemplateWitnessSession * session = current_template_witness_session();
  if(session == nullptr) {
    return;
  }
  TemplateWitnessSourceRange range;
  range.file = file;
  range.begin_line = begin_line;
  range.end_line = end_line;
  range.first_body_column = first_body_column <= 0 ? 1 : first_body_column;
  for(std::size_t i = 0; i < parameter_names.size(); ++i) {
    const std::string & name = parameter_names[i];
    if(name.empty()) {
      continue;
    }
    bool seen = false;
    for(std::size_t j = 0; j < range.parameter_names.size(); ++j) {
      if(range.parameter_names[j] == name) {
        seen = true;
        break;
      }
    }
    if(!seen) {
      range.parameter_names.push_back(name);
    }
  }
  for(std::size_t i = 0; i < session->template_body_ranges.size(); ++i) {
    TemplateWitnessSourceRange & existing =
        session->template_body_ranges[i];
    if(existing.file == range.file &&
       existing.begin_line == range.begin_line &&
       existing.end_line == range.end_line &&
       existing.first_body_column == range.first_body_column) {
      for(std::size_t j = 0; j < range.parameter_names.size(); ++j) {
        bool seen = false;
        for(std::size_t k = 0; k < existing.parameter_names.size(); ++k) {
          if(existing.parameter_names[k] == range.parameter_names[j]) {
            seen = true;
            break;
          }
        }
        if(!seen) {
          existing.parameter_names.push_back(range.parameter_names[j]);
        }
      }
      return;
    }
  }
  session->template_body_ranges.push_back(range);
}

inline void record_template_witness_template_body_range(
    const std::string & file,
    int begin_line,
    int end_line,
    int first_body_column)
{
  record_template_witness_template_body_range(
      file,
      begin_line,
      end_line,
      first_body_column,
      std::vector<std::string>());
}

inline void record_template_witness_template_header_context(
    const std::string & file,
    int begin_line,
    int begin_column,
    int end_line,
    int end_column,
    bool class_template,
    const std::vector<std::string> & parameter_names)
{
  if(file.empty() || begin_line <= 0 || end_line < begin_line ||
     parameter_names.empty()) {
    return;
  }
  TemplateWitnessSession * session = current_template_witness_session();
  if(session == nullptr) {
    return;
  }
  TemplateWitnessTemplateHeaderContext context;
  context.file = file;
  context.begin_line = begin_line;
  context.begin_column = begin_column <= 0 ? 1 : begin_column;
  context.end_line = end_line;
  context.end_column = end_column;
  context.class_template = class_template;
  for(std::size_t i = 0; i < parameter_names.size(); ++i) {
    const std::string & name = parameter_names[i];
    if(name.empty()) {
      continue;
    }
    bool seen = false;
    for(std::size_t j = 0; j < context.parameter_names.size(); ++j) {
      if(context.parameter_names[j] == name) {
        seen = true;
        break;
      }
    }
    if(!seen) {
      context.parameter_names.push_back(name);
    }
  }
  if(context.parameter_names.empty()) {
    return;
  }
  for(std::size_t i = 0; i < session->template_header_contexts.size(); ++i) {
    const TemplateWitnessTemplateHeaderContext & existing =
        session->template_header_contexts[i];
    if(existing.file == context.file &&
       existing.begin_line == context.begin_line &&
       existing.begin_column == context.begin_column &&
       existing.end_line == context.end_line &&
       existing.end_column == context.end_column &&
       existing.class_template == context.class_template &&
       existing.parameter_names == context.parameter_names) {
      return;
    }
  }
  session->template_header_contexts.push_back(context);
}

inline ScopedTemplateWitnessSession::ScopedTemplateWitnessSession(
    TemplateWitnessSession * session)
  : previous_(template_witness_detail::current_witness_session_storage())
{
  template_witness_detail::current_witness_session_storage() = session;
  if(session != nullptr) {
    template_witness_detail::current_witness_entry_contexts_storage().push_back(
        TemplateWitnessEntryContext());
  }
}

inline ScopedTemplateWitnessSession::~ScopedTemplateWitnessSession()
{
  if(template_witness_detail::current_witness_session_storage() != nullptr &&
     !template_witness_detail::current_witness_entry_contexts_storage().empty()) {
    template_witness_detail::current_witness_entry_contexts_storage().pop_back();
  }
  template_witness_detail::current_witness_session_storage() = previous_;
}

inline ScopedTemplateWitnessEntryContext::ScopedTemplateWitnessEntryContext()
  : active_(false)
{}

inline ScopedTemplateWitnessEntryContext::ScopedTemplateWitnessEntryContext(
    const TemplateWitnessEntryContext & context)
  : active_(template_witness_detail::current_witness_session_storage() != nullptr)
{
  if(active_) {
    template_witness_detail::current_witness_entry_contexts_storage().push_back(
        context);
  }
}

inline ScopedTemplateWitnessEntryContext::~ScopedTemplateWitnessEntryContext()
{
  if(active_ &&
     !template_witness_detail::current_witness_entry_contexts_storage().empty()) {
    template_witness_detail::current_witness_entry_contexts_storage().pop_back();
  }
}

inline ScopedTemplateWitnessSourceTypeLookup::ScopedTemplateWitnessSourceTypeLookup(
    bool active)
  : active_(active &&
            template_witness_detail::current_witness_session_storage() != nullptr)
{
  if(active_) {
    ++template_witness_detail::current_source_type_lookup_depth_storage();
  }
}

inline ScopedTemplateWitnessSourceTypeLookup::~ScopedTemplateWitnessSourceTypeLookup()
{
  if(active_) {
    --template_witness_detail::current_source_type_lookup_depth_storage();
  }
}

inline ScopedTemplateWitnessQualifiedMemberTypeLookup::
ScopedTemplateWitnessQualifiedMemberTypeLookup(bool active)
  : active_(active &&
            template_witness_detail::current_witness_session_storage() != nullptr)
{
  if(active_) {
    ++template_witness_detail::
        current_qualified_member_type_lookup_depth_storage();
  }
}

inline ScopedTemplateWitnessQualifiedMemberTypeLookup::
~ScopedTemplateWitnessQualifiedMemberTypeLookup()
{
  if(active_) {
    --template_witness_detail::
        current_qualified_member_type_lookup_depth_storage();
  }
}

inline ScopedTemplateWitnessTypeLookupPause::ScopedTemplateWitnessTypeLookupPause(
    bool active)
  : active_(active &&
            (template_witness_detail::current_source_type_lookup_depth_storage() > 0 ||
             template_witness_detail::
                 current_qualified_member_type_lookup_depth_storage() > 0))
{
  if(active_) {
    ++template_witness_detail::current_type_lookup_pause_depth_storage();
  }
}

inline ScopedTemplateWitnessTypeLookupPause::~ScopedTemplateWitnessTypeLookupPause()
{
  if(active_) {
    --template_witness_detail::current_type_lookup_pause_depth_storage();
  }
}

inline ScopedTemplateWitnessSourceCapturePause::ScopedTemplateWitnessSourceCapturePause(
    bool active)
  : active_(active &&
            template_witness_detail::current_witness_session_storage() != nullptr)
{
  if(active_) {
    ++template_witness_detail::current_source_capture_pause_depth_storage();
  }
}

inline ScopedTemplateWitnessSourceCapturePause::~ScopedTemplateWitnessSourceCapturePause()
{
  if(active_) {
    --template_witness_detail::current_source_capture_pause_depth_storage();
  }
}

inline ScopedTemplateWitnessLifecyclePause::ScopedTemplateWitnessLifecyclePause(
    bool active)
  : active_(active &&
            template_witness_detail::current_witness_session_storage() != nullptr)
{
  if(active_) {
    ++template_witness_detail::current_lifecycle_pause_depth_storage();
  }
}

inline ScopedTemplateWitnessLifecyclePause::~ScopedTemplateWitnessLifecyclePause()
{
  if(active_) {
    --template_witness_detail::current_lifecycle_pause_depth_storage();
  }
}

inline ScopedTemplateWitnessFunctionCallSourceCapturePause::
    ScopedTemplateWitnessFunctionCallSourceCapturePause(bool active)
  : active_(active &&
            template_witness_detail::current_witness_session_storage() != nullptr)
{
  if(active_) {
    ++template_witness_detail::
        current_function_call_source_capture_pause_depth_storage();
  }
}

inline ScopedTemplateWitnessFunctionCallSourceCapturePause::
    ~ScopedTemplateWitnessFunctionCallSourceCapturePause()
{
  if(active_) {
    --template_witness_detail::
        current_function_call_source_capture_pause_depth_storage();
  }
}

inline ScopedTemplateWitnessDeclvalCallSourceCapturePause::
    ScopedTemplateWitnessDeclvalCallSourceCapturePause(bool active)
  : active_(active &&
            template_witness_detail::current_witness_session_storage() != nullptr)
{
  if(active_) {
    ++template_witness_detail::
        current_declval_call_source_capture_pause_depth_storage();
  }
}

inline ScopedTemplateWitnessDeclvalCallSourceCapturePause::
    ~ScopedTemplateWitnessDeclvalCallSourceCapturePause()
{
  if(active_) {
    --template_witness_detail::
        current_declval_call_source_capture_pause_depth_storage();
  }
}

inline TemplateWitnessEntryContext current_template_witness_entry_context()
{
  if(template_witness_detail::current_witness_entry_contexts_storage().empty()) {
    return TemplateWitnessEntryContext();
  }
  return template_witness_detail::current_witness_entry_contexts_storage().back();
}

inline std::vector<const TemplateLifecycleEvent *>
template_witness_lifecycle_events_by_origin(
    const TemplateWitnessSession & session,
    TemplateWitnessOrigin origin)
{
  std::vector<const TemplateLifecycleEvent *> out;
  for(std::size_t i = 0; i < session.lifecycle_events.size(); ++i) {
    if(session.lifecycle_events[i].entry_context.origin == origin) {
      out.push_back(&session.lifecycle_events[i]);
    }
  }
  return out;
}

inline TemplateLifecycleCause template_lifecycle_cause_from_closure_reason(
    TemplateClosureReason reason)
{
  switch(reason) {
  case TemplateClosureReason::None:
    return TemplateLifecycleCause::None;
  case TemplateClosureReason::TrackInstantiation:
    return TemplateLifecycleCause::TrackInstantiation;
  case TemplateClosureReason::RequireDefinition:
    return TemplateLifecycleCause::RequireDefinition;
  case TemplateClosureReason::EnsureDefinition:
    return TemplateLifecycleCause::EnsureDefinition;
  case TemplateClosureReason::FinalizeClass:
    return TemplateLifecycleCause::FinalizeClass;
  }
  return TemplateLifecycleCause::None;
}

inline void note_template_witness_lifecycle_event(
    TemplateLifecycleEvent event
#if defined(CPPGM_ENABLE_WITNESS_PROVENANCE)
    ,
    witness_provenance::WitnessProducerSite producer_site =
        witness_provenance::WitnessProducerSite::Unknown
#endif
    )
{
  TemplateWitnessSession * session =
      template_witness_detail::current_witness_session_storage();
  // The typed lifecycle observer applies pause policy before reaching here.
  if(session == nullptr) {
    return;
  }
  event.entry_context = current_template_witness_entry_context();
  event.cause = event.cause == TemplateLifecycleCause::None ?
      template_lifecycle_cause_from_closure_reason(
          event.entry_context.closure_reason) :
      event.cause;
  template_witness_detail::refresh_lifecycle_event_metadata(event);
#if defined(CPPGM_ENABLE_WITNESS_PROVENANCE)
  const witness_provenance::ScopedLifecycleAttempt provenance_attempt(
      *session,
      producer_site,
      event);
#endif
  session->lifecycle_events.push_back(event);
}

inline bool template_witness_source_capture_enabled()
{
  return template_witness_detail::current_witness_session_storage() != nullptr &&
      template_witness_detail::current_source_capture_pause_depth_storage() == 0;
}

inline bool template_witness_function_call_source_capture_enabled()
{
  return template_witness_source_capture_enabled() &&
      template_witness_detail::
          current_function_call_source_capture_pause_depth_storage() == 0;
}

inline bool template_witness_declval_call_source_capture_enabled()
{
  return template_witness_detail::
      current_declval_call_source_capture_pause_depth_storage() == 0;
}

inline bool template_witness_source_type_lookup_active()
{
  return template_witness_detail::current_source_type_lookup_depth_storage() > 0 &&
      template_witness_detail::current_type_lookup_pause_depth_storage() == 0;
}

inline bool template_witness_qualified_member_type_lookup_active()
{
  return template_witness_detail::
             current_qualified_member_type_lookup_depth_storage() > 0 &&
      template_witness_detail::current_type_lookup_pause_depth_storage() == 0;
}

inline std::string normalize_template_witness_source_location(
    const std::string & location)
{
  std::string value = location;
  while(!value.empty() &&
        (value[0] == ' ' || value[0] == '\t' || value[0] == '\n' ||
         value[0] == '\r')) {
    value.erase(0, 1);
  }
  while(value.compare(0, 3, "at ") == 0) {
    value = value.substr(3);
    while(!value.empty() &&
          (value[0] == ' ' || value[0] == '\t' || value[0] == '\n' ||
           value[0] == '\r')) {
      value.erase(0, 1);
    }
  }
  return value;
}

std::string dump_template_witness_text(const TemplateWitnessSession & session,
                                       const std::string & source_path);
std::string dump_witness_text(const TemplateWitnessSession & session,
                              const std::string & source_path);
std::string dump_witness_debug_text(const TemplateWitnessSession & session,
                                    const std::string & source_path);

}  // namespace template_api

#include "witness_provenance.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <sstream>
#include <sys/stat.h>
#include <unistd.h>

#include "semantic_source_use.h"
#include "template_witness.h"

namespace witness_provenance {

#if defined(CPPGM_ENABLE_WITNESS_PROVENANCE)

namespace {

using semantic_source_use::SemanticSourceUse;
using semantic_source_use::SemanticSourceUseTable;

struct RowLineage
{
  std::uint64_t row_id = 0;
  std::set<WitnessProducerSite> producers;
};

struct SessionState
{
  const template_api::TemplateWitnessSession * session_address = nullptr;
  std::string source_path;
  std::vector<RowLineage> rows;
  std::map<std::string, std::set<WitnessProducerSite> > lifecycle_producers;
  std::vector<std::string> records;
  bool flushed = false;
};

struct SourceAttempt
{
  SessionState * state = nullptr;
  SemanticSourceUseTable before;
  std::vector<RowLineage> before_rows;
  SemanticSourceUse use;
  WitnessProducerSite producer = WitnessProducerSite::Unknown;
  std::vector<std::size_t> collided_indices;
};

struct LifecycleAttempt
{
  SessionState * state = nullptr;
  WitnessProducerSite producer = WitnessProducerSite::Unknown;
  std::string key;
  std::vector<std::string> before;
  std::set<WitnessProducerSite> collided_producers;
  bool key_existed_before = false;
  std::string kind;
  std::string location;
  std::string entity;
};

std::mutex & trace_mutex()
{
  static std::mutex value;
  return value;
}

std::vector<std::unique_ptr<SessionState> > & session_states()
{
  static std::vector<std::unique_ptr<SessionState> > value;
  return value;
}

std::map<std::uint64_t, SourceAttempt> & source_attempts()
{
  static std::map<std::uint64_t, SourceAttempt> value;
  return value;
}

std::map<std::uint64_t, LifecycleAttempt> & lifecycle_attempts()
{
  static std::map<std::uint64_t, LifecycleAttempt> value;
  return value;
}

std::uint64_t next_token()
{
  static std::uint64_t value = 1;
  return value++;
}

std::uint64_t next_row_id()
{
  static std::uint64_t value = 1;
  return value++;
}

std::uint64_t next_event_id()
{
  static std::uint64_t value = 1;
  return value++;
}

std::string json_escape(const std::string & value)
{
  std::ostringstream out;
  for(std::size_t i = 0; i < value.size(); ++i) {
    const unsigned char ch = static_cast<unsigned char>(value[i]);
    switch(ch) {
    case '\\': out << "\\\\"; break;
    case '"': out << "\\\""; break;
    case '\b': out << "\\b"; break;
    case '\f': out << "\\f"; break;
    case '\n': out << "\\n"; break;
    case '\r': out << "\\r"; break;
    case '\t': out << "\\t"; break;
    default:
      if(ch < 0x20) {
        static const char digits[] = "0123456789abcdef";
        out << "\\u00" << digits[(ch >> 4) & 0xf] << digits[ch & 0xf];
      } else {
        out << value[i];
      }
      break;
    }
  }
  return out.str();
}

std::string quoted(const std::string & value)
{
  return std::string("\"") + json_escape(value) + "\"";
}

const char * source_use_kind_name(semantic_source_use::SourceUseKind kind)
{
  switch(kind) {
  case semantic_source_use::SourceUseKind::FunctionCall: return "function_call";
  case semantic_source_use::SourceUseKind::ClassUse: return "class_use";
  case semantic_source_use::SourceUseKind::AliasUse: return "alias_use";
  case semantic_source_use::SourceUseKind::VariableUse: return "variable_use";
  }
  return "unknown";
}

const char * source_use_role_name(semantic_source_use::SourceUseRole role)
{
  switch(role) {
  case semantic_source_use::SourceUseRole::Unknown: return "unknown";
  case semantic_source_use::SourceUseRole::TypeUse: return "type";
  case semantic_source_use::SourceUseRole::MaterializedTypeUse: return "materialized_type";
  case semantic_source_use::SourceUseRole::QualifierUse: return "qualifier";
  case semantic_source_use::SourceUseRole::StaticMemberDefinitionOwner:
    return "static_member_definition_owner";
  case semantic_source_use::SourceUseRole::ValueUse: return "value";
  case semantic_source_use::SourceUseRole::CallUse: return "call";
  }
  return "unknown";
}

const char * source_use_ownership_name(
    semantic_source_use::SourceUseOwnership ownership)
{
  switch(ownership) {
  case semantic_source_use::SourceUseOwnership::Direct: return "direct";
  case semantic_source_use::SourceUseOwnership::NestedDerived: return "nested";
  case semantic_source_use::SourceUseOwnership::SourceOwned: return "source_owned";
  }
  return "unknown";
}

const char * source_selection_name(
    semantic_source_use::SourceSelectionKind selection)
{
  switch(selection) {
  case semantic_source_use::SourceSelectionKind::None: return "none";
  case semantic_source_use::SourceSelectionKind::Primary: return "primary";
  case semantic_source_use::SourceSelectionKind::PartialSpecialization:
    return "partial";
  case semantic_source_use::SourceSelectionKind::ExplicitSpecialization:
    return "explicit";
  case semantic_source_use::SourceSelectionKind::Instantiation:
    return "instantiation";
  }
  return "unknown";
}

const char * lifecycle_kind_name(template_api::TemplateLifecycleEventKind kind)
{
  switch(kind) {
  case template_api::TemplateLifecycleEventKind::RequireDefinition:
    return "require_definition";
  case template_api::TemplateLifecycleEventKind::EnsureDefinition:
    return "ensure_definition";
  case template_api::TemplateLifecycleEventKind::FunctionInstantiation:
    return "function_instantiation";
  case template_api::TemplateLifecycleEventKind::ClassInstantiation:
    return "class_instantiation";
  case template_api::TemplateLifecycleEventKind::AliasInstantiation:
    return "alias_instantiation";
  case template_api::TemplateLifecycleEventKind::VariableInstantiation:
    return "variable_instantiation";
  case template_api::TemplateLifecycleEventKind::ClassFinalization:
    return "class_finalization";
  }
  return "unknown";
}

std::string producer_array(const std::set<WitnessProducerSite> & sites)
{
  std::ostringstream out;
  out << '[';
  bool first = true;
  for(std::set<WitnessProducerSite>::const_iterator it = sites.begin();
      it != sites.end();
      ++it) {
    if(!first) {
      out << ',';
    }
    first = false;
    out << quoted(producer_site_name(*it));
  }
  out << ']';
  return out.str();
}

std::string producer_array(const std::vector<WitnessProducerSite> & sites)
{
  return producer_array(std::set<WitnessProducerSite>(sites.begin(), sites.end()));
}

SessionState & state_for_session_locked(
    const template_api::TemplateWitnessSession & session)
{
  std::vector<std::unique_ptr<SessionState> > & states = session_states();
  for(std::size_t i = 0; i < states.size(); ++i) {
    if(states[i]->session_address == &session) {
      if(states[i]->source_path.empty()) {
        states[i]->source_path = session.primary_source_file;
      }
      return *states[i];
    }
  }
  if(!session.primary_source_file.empty()) {
    for(std::size_t i = 0; i < states.size(); ++i) {
      if(!states[i]->flushed &&
         states[i]->source_path == session.primary_source_file) {
        states[i]->session_address = &session;
        return *states[i];
      }
    }
  }
  std::unique_ptr<SessionState> created(new SessionState());
  created->session_address = &session;
  created->source_path = session.primary_source_file;
  states.push_back(std::move(created));
  return *states.back();
}

void reconcile_rows(SessionState & state, const SemanticSourceUseTable & table)
{
  if(state.rows.size() == table.uses.size()) {
    return;
  }
  state.rows.resize(table.uses.size());
  for(std::size_t i = 0; i < state.rows.size(); ++i) {
    if(state.rows[i].row_id == 0) {
      state.rows[i].row_id = next_row_id();
    }
  }
}

bool source_rows_collide(const SemanticSourceUse & lhs,
                         const SemanticSourceUse & rhs)
{
  if(lhs == rhs) {
    return true;
  }
  if(lhs.kind != rhs.kind) {
    return false;
  }
  switch(lhs.kind) {
  case semantic_source_use::SourceUseKind::FunctionCall:
    return semantic_source_use::function_call_equivalent_ignoring_binding_spacing(
        lhs, rhs);
  case semantic_source_use::SourceUseKind::AliasUse:
    return semantic_source_use::alias_use_equivalent_ignoring_binding_spacing(
               lhs, rhs) ||
        (lhs.location == rhs.location && lhs.role == rhs.role &&
         lhs.template_name == rhs.template_name);
  case semantic_source_use::SourceUseKind::ClassUse:
    return semantic_source_use::class_use_equivalent_ignoring_binding_spacing(
               lhs, rhs) ||
        (lhs.location == rhs.location && lhs.role == rhs.role &&
         lhs.template_name == rhs.template_name);
  case semantic_source_use::SourceUseKind::VariableUse:
    return semantic_source_use::variable_use_equivalent_ignoring_location(lhs,
                                                                           rhs);
  }
  return false;
}

bool source_table_equal(const SemanticSourceUseTable & lhs,
                        const SemanticSourceUseTable & rhs)
{
  return lhs.uses == rhs.uses;
}

std::string source_use_changed_fields(const SemanticSourceUse & before,
                                      const SemanticSourceUse & after)
{
  std::vector<std::string> fields;
  if(before.location != after.location) fields.push_back("location");
  if(!(before.spelling_anchor == after.spelling_anchor))
    fields.push_back("anchor");
  if(!(before.selected_decl_anchor == after.selected_decl_anchor))
    fields.push_back("selected_decl_anchor");
  if(!(before.template_id_occurrence == after.template_id_occurrence))
    fields.push_back("occurrence");
  if(before.ownership != after.ownership) fields.push_back("ownership");
  if(before.selection != after.selection) fields.push_back("selection");
  if(!(before.selected_entity == after.selected_entity))
    fields.push_back("selected_entity");
  if(before.bindings != after.bindings) fields.push_back("bindings");
  if(before.specialization_bindings != after.specialization_bindings)
    fields.push_back("specialization_bindings");
  std::ostringstream out;
  for(std::size_t i = 0; i < fields.size(); ++i) {
    if(i != 0) out << ',';
    out << fields[i];
  }
  return out.str();
}

std::string source_attempt_record(const SourceAttempt & attempt,
                                  const std::string & action,
                                  const std::string & changed_fields,
                                  const std::set<WitnessProducerSite> & collided)
{
  const SemanticSourceUse & use = attempt.use;
  std::ostringstream out;
  out << "{\"record\":\"source_attempt\""
      << ",\"producer\":" << quoted(producer_site_name(attempt.producer))
      << ",\"action\":" << quoted(action)
      << ",\"kind\":" << quoted(source_use_kind_name(use.kind))
      << ",\"role\":" << quoted(source_use_role_name(use.role))
      << ",\"ownership\":" << quoted(source_use_ownership_name(use.ownership))
      << ",\"location\":" << quoted(use.location)
      << ",\"spelling_anchor\":" << quoted(use.spelling_anchor.location)
      << ",\"selected_entity\":" << quoted(use.selected_entity.name)
      << ",\"selected_decl\":" << quoted(use.selected_entity.decl_location)
      << ",\"template_name\":" << quoted(use.template_name)
      << ",\"selection\":" << quoted(source_selection_name(use.selection))
      << ",\"binding_count\":" << use.bindings.size()
      << ",\"specialization_binding_count\":"
      << use.specialization_bindings.size()
      << ",\"occurrence_present\":"
      << (use.template_id_occurrence.present ? "true" : "false")
      << ",\"occurrence_argument_count\":"
      << use.template_id_occurrence.arguments.size()
      << ",\"occurrence_function_result\":"
      << (use.template_id_occurrence.function_result_type_use ? "true" : "false")
      << ",\"occurrence_conversion_result\":"
      << (use.template_id_occurrence.conversion_result_type_use ? "true" : "false")
      << ",\"occurrence_synthesized\":"
      << (use.template_id_occurrence.synthesized ? "true" : "false")
      << ",\"changed_fields\":" << quoted(changed_fields)
      << ",\"collided_producers\":" << producer_array(collided)
      << '}';
  return out.str();
}

std::string lifecycle_event_key(const template_api::TemplateLifecycleEvent & event)
{
  std::ostringstream out;
  out << static_cast<int>(event.entry_context.origin) << '|'
      << static_cast<int>(event.entry_context.closure_reason) << '|'
      << static_cast<int>(event.kind) << '|'
      << static_cast<int>(event.cause) << '|'
      << event.location << '|' << event.entity << '|'
      << event.decl_location << '|' << event.detail;
  return out.str();
}

std::string lifecycle_event_full(const template_api::TemplateLifecycleEvent & event)
{
  std::ostringstream out;
  out << lifecycle_event_key(event) << '|'
      << event.entry_context.trigger_entity << '|'
      << event.entry_context.trigger_decl_location << '|'
      << event.entry_context.trigger_has_template_identity << '|'
      << event.entity_has_template_identity << '|'
      << event.entity_is_unnamed_class << '|'
      << event.entity_is_constexpr_function << '|'
      << event.entity_is_defaulted_copy_or_move_constructor << '|'
      << event.public_source_required;
  return out.str();
}

std::vector<std::string> lifecycle_fingerprints(
    const template_api::TemplateWitnessSession & session)
{
  std::vector<std::string> out;
  for(std::size_t i = 0; i < session.lifecycle_events.size(); ++i) {
    out.push_back(lifecycle_event_full(session.lifecycle_events[i]));
  }
  return out;
}

std::string safe_filename(const std::string & path)
{
  std::string value = path.empty() ? "unknown" : path;
  const std::string::size_type slash = value.find_last_of("/\\");
  if(slash != std::string::npos) {
    value = value.substr(slash + 1);
  }
  for(std::size_t i = 0; i < value.size(); ++i) {
    const unsigned char ch = static_cast<unsigned char>(value[i]);
    if(!std::isalnum(ch) && value[i] != '.' && value[i] != '-') {
      value[i] = '_';
    }
  }
  return value;
}

}  // namespace

const char * producer_site_name(WitnessProducerSite site)
{
#define CPPGM_PRODUCER_NAME(value, text) \
  case WitnessProducerSite::value: return text
  switch(site) {
  CPPGM_PRODUCER_NAME(Unknown, "unknown");
  CPPGM_PRODUCER_NAME(ClassCallsemantic10, "class.callsemantic.10");
  CPPGM_PRODUCER_NAME(ClassTemplateReference02, "class.class_template_reference.02");
  CPPGM_PRODUCER_NAME(AliasTemplateArgumentSemantics02,
                      "alias.template_argument_semantics.02");
  CPPGM_PRODUCER_NAME(AliasCallsemantic02, "alias.callsemantic.02");
  CPPGM_PRODUCER_NAME(FunctionSemanticTemplateFunction,
                      "function.semantic_template_function");
  CPPGM_PRODUCER_NAME(VariableTemplateInstantiation,
                      "variable.template_instantiation");
  CPPGM_PRODUCER_NAME(LifecycleTemplateApi01, "lifecycle.template_api.01");
  CPPGM_PRODUCER_NAME(LifecycleTemplateApi02, "lifecycle.template_api.02");
  CPPGM_PRODUCER_NAME(LifecycleTemplateApi03, "lifecycle.template_api.03");
  CPPGM_PRODUCER_NAME(LifecycleTemplateApi04, "lifecycle.template_api.04");
  CPPGM_PRODUCER_NAME(LifecycleTemplateApi05, "lifecycle.template_api.05");
  CPPGM_PRODUCER_NAME(LifecycleTemplateApi06, "lifecycle.template_api.06");
  CPPGM_PRODUCER_NAME(LifecycleTemplateApi07, "lifecycle.template_api.07");
  CPPGM_PRODUCER_NAME(LifecycleTemplateApi09, "lifecycle.template_api.09");
  CPPGM_PRODUCER_NAME(LifecycleCallsemantic01, "lifecycle.callsemantic.01");
  CPPGM_PRODUCER_NAME(LifecycleCallsemantic02, "lifecycle.callsemantic.02");
  CPPGM_PRODUCER_NAME(LifecycleConstantValueLookup02,
                      "lifecycle.constant_value_lookup.02");
  }
#undef CPPGM_PRODUCER_NAME
  return "unknown";
}

const char * upstream_route_name(WitnessUpstreamRoute route)
{
  switch(route) {
  case WitnessUpstreamRoute::NestedClassUseFromAstNode:
    return "nested_class_use.ast_node";
  case WitnessUpstreamRoute::NestedClassUseFromTemplateArguments:
    return "nested_class_use.template_arguments";
  case WitnessUpstreamRoute::DeducedClassUseFromResolvedAliasType:
    return "class_use.resolved_alias_type";
  }
  return "unknown";
}

bool enabled()
{
  static const bool value = []()
  {
    const char * directory = std::getenv("CPPGM_WITNESS_PROVENANCE_DIR");
    return directory != nullptr && directory[0] != '\0';
  }();
  return value;
}

ScopedSourceUseAttempt::ScopedSourceUseAttempt(
    template_api::TemplateWitnessSession * session,
    SemanticSourceUseTable * table,
    WitnessProducerSite producer,
    const SemanticSourceUse & use)
  : session_(session), table_(table)
{
  if(!enabled() || session_ == nullptr || table_ == nullptr) {
    return;
  }
  std::lock_guard<std::mutex> lock(trace_mutex());
  SessionState & state = state_for_session_locked(*session_);
  reconcile_rows(state, *table_);
  SourceAttempt attempt;
  attempt.state = &state;
  attempt.before = *table_;
  attempt.before_rows = state.rows;
  attempt.use = use;
  attempt.producer = producer;
  for(std::size_t i = 0; i < table_->uses.size(); ++i) {
    if(source_rows_collide(table_->uses[i], use)) {
      attempt.collided_indices.push_back(i);
    }
  }
  token_ = next_token();
  source_attempts()[token_] = attempt;
}

ScopedSourceUseAttempt::~ScopedSourceUseAttempt()
{
  if(token_ == 0 || session_ == nullptr || table_ == nullptr) {
    return;
  }
  std::lock_guard<std::mutex> lock(trace_mutex());
  std::map<std::uint64_t, SourceAttempt>::iterator found =
      source_attempts().find(token_);
  if(found == source_attempts().end()) {
    return;
  }
  SourceAttempt attempt = found->second;
  source_attempts().erase(found);
  SessionState & state = *attempt.state;

  std::set<WitnessProducerSite> collided_producers;
  for(std::size_t i = 0; i < attempt.collided_indices.size(); ++i) {
    const std::size_t index = attempt.collided_indices[i];
    if(index < attempt.before_rows.size()) {
      collided_producers.insert(attempt.before_rows[index].producers.begin(),
                                attempt.before_rows[index].producers.end());
    }
  }

  const bool unchanged = source_table_equal(attempt.before, *table_);
  bool exact_before = false;
  for(std::size_t i = 0; i < attempt.before.uses.size(); ++i) {
    exact_before = exact_before || attempt.before.uses[i] == attempt.use;
  }
  bool exact_after = false;
  for(std::size_t i = 0; i < table_->uses.size(); ++i) {
    exact_after = exact_after || table_->uses[i] == attempt.use;
  }

  std::string action;
  if(unchanged) {
    action = exact_before ? "exact_duplicate" : "rejected";
  } else if(table_->uses.size() > attempt.before.uses.size()) {
    action = "inserted";
  } else if(exact_after) {
    action = "replaced";
  } else {
    action = "enriched";
  }

  std::vector<RowLineage> after_rows(table_->uses.size());
  std::vector<char> used_before(attempt.before.uses.size(), 0);
  for(std::size_t i = 0; i < table_->uses.size(); ++i) {
    for(std::size_t j = 0; j < attempt.before.uses.size(); ++j) {
      if(!used_before[j] && table_->uses[i] == attempt.before.uses[j]) {
        after_rows[i] = attempt.before_rows[j];
        used_before[j] = 1;
        break;
      }
    }
    if(after_rows[i].row_id == 0) {
      for(std::size_t j = 0; j < attempt.before.uses.size(); ++j) {
        if(!used_before[j] &&
           source_rows_collide(table_->uses[i], attempt.before.uses[j])) {
          after_rows[i] = attempt.before_rows[j];
          used_before[j] = 1;
          break;
        }
      }
    }
    if(after_rows[i].row_id == 0) {
      after_rows[i].row_id = next_row_id();
    }
    if(source_rows_collide(table_->uses[i], attempt.use)) {
      after_rows[i].producers.insert(attempt.producer);
      for(std::size_t j = 0; j < attempt.collided_indices.size(); ++j) {
        const std::size_t index = attempt.collided_indices[j];
        if(index < attempt.before_rows.size()) {
          after_rows[i].producers.insert(
              attempt.before_rows[index].producers.begin(),
              attempt.before_rows[index].producers.end());
        }
      }
    }
  }
  state.rows.swap(after_rows);

  std::string changed_fields;
  for(std::size_t i = 0; i < attempt.before.uses.size(); ++i) {
    if(!source_rows_collide(attempt.before.uses[i], attempt.use)) {
      continue;
    }
    for(std::size_t j = 0; j < table_->uses.size(); ++j) {
      if(source_rows_collide(attempt.before.uses[i], table_->uses[j]) &&
         !(attempt.before.uses[i] == table_->uses[j])) {
        changed_fields = source_use_changed_fields(attempt.before.uses[i],
                                                   table_->uses[j]);
        break;
      }
    }
    if(!changed_fields.empty()) break;
  }
  state.records.push_back(
      source_attempt_record(attempt, action, changed_fields, collided_producers));
}

ScopedLifecycleAttempt::ScopedLifecycleAttempt(
    template_api::TemplateWitnessSession & session,
    WitnessProducerSite producer,
    const template_api::TemplateLifecycleEvent & event)
  : session_(&session)
{
  if(!enabled()) {
    return;
  }
  std::lock_guard<std::mutex> lock(trace_mutex());
  SessionState & state = state_for_session_locked(session);
  LifecycleAttempt attempt;
  attempt.state = &state;
  attempt.producer = producer;
  attempt.key = lifecycle_event_key(event);
  attempt.before = lifecycle_fingerprints(session);
  const std::map<std::string, std::set<WitnessProducerSite> >::const_iterator
      collided = state.lifecycle_producers.find(attempt.key);
  if(collided != state.lifecycle_producers.end()) {
    attempt.key_existed_before = true;
    attempt.collided_producers = collided->second;
  }
  if(!attempt.key_existed_before) {
    for(std::size_t i = 0; i < session.lifecycle_events.size(); ++i) {
      if(lifecycle_event_key(session.lifecycle_events[i]) == attempt.key) {
        attempt.key_existed_before = true;
        break;
      }
    }
  }
  attempt.kind = lifecycle_kind_name(event.kind);
  attempt.location = event.location;
  attempt.entity = event.entity;
  token_ = next_token();
  lifecycle_attempts()[token_] = attempt;
}

ScopedLifecycleAttempt::~ScopedLifecycleAttempt()
{
  if(token_ == 0 || session_ == nullptr) {
    return;
  }
  std::lock_guard<std::mutex> lock(trace_mutex());
  std::map<std::uint64_t, LifecycleAttempt>::iterator found =
      lifecycle_attempts().find(token_);
  if(found == lifecycle_attempts().end()) {
    return;
  }
  LifecycleAttempt attempt = found->second;
  lifecycle_attempts().erase(found);
  const std::vector<std::string> after = lifecycle_fingerprints(*session_);
  std::string action = "inserted";
  if(after == attempt.before) {
    action = "exact_duplicate";
  } else if(after.size() == attempt.before.size() &&
            attempt.key_existed_before) {
    action = "enriched";
  }
  attempt.state->lifecycle_producers[attempt.key].insert(attempt.producer);
  std::ostringstream record;
  record << "{\"record\":\"lifecycle_attempt\""
         << ",\"producer\":" << quoted(producer_site_name(attempt.producer))
         << ",\"action\":" << quoted(action)
         << ",\"kind\":" << quoted(attempt.kind)
         << ",\"location\":" << quoted(attempt.location)
         << ",\"entity\":" << quoted(attempt.entity)
         << ",\"collided_producers\":"
         << producer_array(attempt.collided_producers)
         << '}';
  attempt.state->records.push_back(record.str());
}

void note_upstream_route(template_api::TemplateWitnessSession * session,
                         WitnessUpstreamRoute route)
{
  if(!enabled() || session == nullptr) {
    return;
  }
  std::lock_guard<std::mutex> lock(trace_mutex());
  SessionState & state = state_for_session_locked(*session);
  std::ostringstream record;
  record << "{\"record\":\"upstream_route\",\"route\":"
         << quoted(upstream_route_name(route)) << '}';
  state.records.push_back(record.str());
}

std::vector<RendererEventLineage> renderer_table_lineages(
    const template_api::TemplateWitnessSession & session)
{
  std::vector<RendererEventLineage> out;
  if(!enabled()) {
    return out;
  }
  std::lock_guard<std::mutex> lock(trace_mutex());
  SessionState & state = state_for_session_locked(session);
  reconcile_rows(state, session.source_use_table);
  out.resize(state.rows.size());
  for(std::size_t i = 0; i < state.rows.size(); ++i) {
    out[i].event_id = next_event_id();
    out[i].table_row_id = state.rows[i].row_id;
    out[i].producers.assign(state.rows[i].producers.begin(),
                            state.rows[i].producers.end());
  }
  return out;
}

void note_renderer_action(const template_api::TemplateWitnessSession & session,
                          const std::string & source_path,
                          const std::string & pass,
                          const std::string & action,
                          const RendererEventLineage & lineage,
                          const std::string & kind,
                          const std::string & location,
                          const std::string & template_name,
                          const std::string & changed_fields)
{
  if(!enabled()) return;
  std::lock_guard<std::mutex> lock(trace_mutex());
  SessionState & state = state_for_session_locked(session);
  std::ostringstream record;
  record << "{\"record\":\"renderer_action\""
         << ",\"source\":" << quoted(source_path)
         << ",\"pass\":" << quoted(pass)
         << ",\"action\":" << quoted(action)
         << ",\"event_id\":" << lineage.event_id
         << ",\"table_row_id\":" << lineage.table_row_id
         << ",\"producers\":" << producer_array(lineage.producers)
         << ",\"kind\":" << quoted(kind)
         << ",\"location\":" << quoted(location)
         << ",\"template_name\":" << quoted(template_name)
         << ",\"changed_fields\":" << quoted(changed_fields)
         << '}';
  state.records.push_back(record.str());
}

void note_renderer_final_visible(
    const template_api::TemplateWitnessSession & session,
    const std::string & source_path,
    const RendererEventLineage & lineage,
    const std::string & kind,
    const std::string & location,
    const std::string & template_name)
{
  if(!enabled()) return;
  std::lock_guard<std::mutex> lock(trace_mutex());
  SessionState & state = state_for_session_locked(session);
  std::ostringstream record;
  record << "{\"record\":\"final_visible\""
         << ",\"source\":" << quoted(source_path)
         << ",\"event_id\":" << lineage.event_id
         << ",\"table_row_id\":" << lineage.table_row_id
         << ",\"producers\":" << producer_array(lineage.producers)
         << ",\"kind\":" << quoted(kind)
         << ",\"location\":" << quoted(location)
         << ",\"template_name\":" << quoted(template_name)
         << '}';
  state.records.push_back(record.str());
}

void finish_session(const template_api::TemplateWitnessSession & session,
                    const std::string & source_path)
{
  if(!enabled()) return;
  std::lock_guard<std::mutex> lock(trace_mutex());
  SessionState & state = state_for_session_locked(session);
  if(state.flushed) return;
  reconcile_rows(state, session.source_use_table);
  for(std::size_t i = 0; i < session.source_use_table.uses.size(); ++i) {
    const SemanticSourceUse & use = session.source_use_table.uses[i];
    std::ostringstream record;
    record << "{\"record\":\"final_table_row\""
           << ",\"row_id\":" << state.rows[i].row_id
           << ",\"producers\":" << producer_array(state.rows[i].producers)
           << ",\"kind\":" << quoted(source_use_kind_name(use.kind))
           << ",\"location\":" << quoted(use.location)
           << ",\"template_name\":" << quoted(use.template_name)
           << '}';
    state.records.push_back(record.str());
  }
  for(std::size_t i = 0; i < session.lifecycle_events.size(); ++i) {
    const template_api::TemplateLifecycleEvent & event =
        session.lifecycle_events[i];
    const std::string key = lifecycle_event_key(event);
    const std::map<std::string, std::set<WitnessProducerSite> >::const_iterator
        producers = state.lifecycle_producers.find(key);
    const std::set<WitnessProducerSite> empty;
    std::ostringstream record;
    record << "{\"record\":\"final_lifecycle_event\""
           << ",\"producers\":"
           << producer_array(producers != state.lifecycle_producers.end() ?
                                 producers->second : empty)
           << ",\"kind\":" << quoted(lifecycle_kind_name(event.kind))
           << ",\"location\":" << quoted(event.location)
           << ",\"entity\":" << quoted(event.entity)
           << '}';
    state.records.push_back(record.str());
  }

  const char * directory_value = std::getenv("CPPGM_WITNESS_PROVENANCE_DIR");
  if(directory_value == nullptr || directory_value[0] == '\0') return;
  const std::string directory(directory_value);
  (void)::mkdir(directory.c_str(), 0777);
  static std::uint64_t file_counter = 1;
  std::ostringstream filename;
  filename << directory << '/' << safe_filename(source_path) << '.'
           << static_cast<long long>(::getpid()) << '.' << file_counter++
           << ".jsonl";
  std::ofstream out(filename.str().c_str());
  for(std::size_t i = 0; i < state.records.size(); ++i) {
    out << state.records[i] << '\n';
  }
  state.flushed = true;
}

#endif

}  // namespace witness_provenance

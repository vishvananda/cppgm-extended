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
  std::set<WitnessUpstreamRoute> upstream_routes;
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

WitnessUpstreamRoute & current_upstream_route()
{
  static thread_local WitnessUpstreamRoute value =
      WitnessUpstreamRoute::Unknown;
  return value;
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
  case semantic_source_use::SourceUseRole::DeclvalCall: return "declval_call";
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

std::string upstream_route_array(const std::set<WitnessUpstreamRoute> & routes)
{
  std::ostringstream out;
  out << '[';
  bool first = true;
  for(std::set<WitnessUpstreamRoute>::const_iterator it = routes.begin();
      it != routes.end();
      ++it) {
    if(*it == WitnessUpstreamRoute::Unknown) {
      continue;
    }
    if(!first) {
      out << ',';
    }
    first = false;
    out << quoted(upstream_route_name(*it));
  }
  out << ']';
  return out.str();
}

std::string upstream_route_array(
    const std::vector<WitnessUpstreamRoute> & routes)
{
  return upstream_route_array(
      std::set<WitnessUpstreamRoute>(routes.begin(), routes.end()));
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

std::string source_attempt_record(const SemanticSourceUse & use,
                                  WitnessProducerSite producer,
                                  WitnessUpstreamRoute upstream_route,
                                  const std::string & action,
                                  const std::set<WitnessProducerSite> & collided)
{
  std::ostringstream out;
  out << "{\"record\":\"source_attempt\""
      << ",\"producer\":" << quoted(producer_site_name(producer))
      << ",\"upstream_route\":"
      << quoted(upstream_route_name(upstream_route))
      << ",\"action\":" << quoted(action)
      << ",\"kind\":" << quoted(source_use_kind_name(use.kind))
      << ",\"role\":" << quoted(source_use_role_name(use.role))
      << ",\"ownership\":" << quoted(source_use_ownership_name(use.ownership))
      << ",\"location\":" << quoted(use.location)
      << ",\"spelling_anchor\":" << quoted(use.location)
      << ",\"selected_entity\":"
      << quoted(use.selected.empty() ? use.template_name : use.selected)
      << ",\"selected_decl\":" << quoted(use.selected_entity_decl_location)
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
      << ",\"occurrence_source_spelled\":"
      << (use.template_id_occurrence.source_spelled ? "true" : "false")
      << ",\"occurrence_in_template_body\":"
      << (use.template_id_occurrence.in_template_body ? "true" : "false")
      << ",\"occurrence_has_dependent_argument\":"
      << (use.template_id_occurrence.has_dependent_argument ? "true" : "false")
      << ",\"occurrence_has_current_specialization_argument\":"
      << (use.template_id_occurrence.has_current_specialization_argument ?
              "true" : "false")
      << ",\"occurrence_current_specialization_use\":"
      << (use.template_id_occurrence.current_specialization_use ?
              "true" : "false")
      << ",\"binding_sources\":[";
  for(std::size_t i = 0; i < use.bindings.size(); ++i) {
    if(i != 0) out << ',';
    out << quoted(use.bindings[i].source);
  }
  out << "]"
      << ",\"binding_args\":[";
  for(std::size_t i = 0; i < use.bindings.size(); ++i) {
    if(i != 0) out << ',';
    out << quoted(use.bindings[i].arg);
  }
  out << "]"
      << ",\"binding_type_like\":[";
  for(std::size_t i = 0; i < use.bindings.size(); ++i) {
    if(i != 0) out << ',';
    out << (use.bindings[i].type_like ? "true" : "false");
  }
  out << "]"
      << ",\"binding_function_type_argument\":[";
  for(std::size_t i = 0; i < use.bindings.size(); ++i) {
    if(i != 0) out << ',';
    out << (use.bindings[i].function_type_argument ? "true" : "false");
  }
  out << "]"
      << ",\"binding_structured_type_spelling\":[";
  for(std::size_t i = 0; i < use.bindings.size(); ++i) {
    if(i != 0) out << ',';
    out << (use.bindings[i].structured_type_spelling ? "true" : "false");
  }
  out << "]"
      << ",\"occurrence_argument_texts\":[";
  for(std::size_t i = 0;
      i < use.template_id_occurrence.arguments.size();
      ++i) {
    if(i != 0) out << ',';
    out << quoted(use.template_id_occurrence.arguments[i].text);
  }
  out << "]"
      << ",\"occurrence_argument_semantic_texts\":[";
  for(std::size_t i = 0;
      i < use.template_id_occurrence.arguments.size();
      ++i) {
    if(i != 0) out << ',';
    out << quoted(use.template_id_occurrence.arguments[i].semantic_text);
  }
  out << "]"
      << ",\"occurrence_argument_dependent\":[";
  for(std::size_t i = 0;
      i < use.template_id_occurrence.arguments.size();
      ++i) {
    if(i != 0) out << ',';
    out << (use.template_id_occurrence.arguments[i].dependent ?
                "true" : "false");
  }
  out << "]"
      << ",\"specialization_binding_sources\":[";
  for(std::size_t i = 0; i < use.specialization_bindings.size(); ++i) {
    if(i != 0) out << ',';
    out << quoted(use.specialization_bindings[i].source);
  }
  out << "]"
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
  CPPGM_PRODUCER_NAME(ClassTemplateReference02, "class.class_template_reference.02");
  CPPGM_PRODUCER_NAME(AliasCanonicalOccurrence,
                      "alias.canonical_occurrence");
  CPPGM_PRODUCER_NAME(FunctionSemanticTemplateFunction,
                      "function.semantic_template_function");
  CPPGM_PRODUCER_NAME(VariableTemplateInstantiation,
                      "variable.template_instantiation");
  CPPGM_PRODUCER_NAME(LifecycleTransitionObserver01,
                      "lifecycle.transition_observer.01");
  }
#undef CPPGM_PRODUCER_NAME
  return "unknown";
}

const char * upstream_route_name(WitnessUpstreamRoute route)
{
  switch(route) {
  case WitnessUpstreamRoute::Unknown:
    return "unknown";
  case WitnessUpstreamRoute::AliasCanonicalOccurrence:
    return "alias.canonical_occurrence";
  case WitnessUpstreamRoute::ClassResolvedTemplateId:
    return "class.resolved_template_id";
  case WitnessUpstreamRoute::ClassDeclarationTypeSource:
    return "class.declaration_type_source";
  case WitnessUpstreamRoute::ClassExplicitSpecializationSource:
    return "class.explicit_specialization_source";
  case WitnessUpstreamRoute::ClassQualifiedValueSource:
    return "class.qualified_value_source";
  case WitnessUpstreamRoute::ClassNestedSourceTemplateId:
    return "class.nested_source_template_id";
  case WitnessUpstreamRoute::FunctionConstantValueLookup:
    return "function.constant_value_lookup";
  case WitnessUpstreamRoute::FunctionConversion:
    return "function.conversion";
  case WitnessUpstreamRoute::FunctionDeclval:
    return "function.declval";
  case WitnessUpstreamRoute::FunctionOverloadResolution:
    return "function.overload_resolution";
  case WitnessUpstreamRoute::VariableDirectInstantiation:
    return "variable.direct_instantiation";
  case WitnessUpstreamRoute::VariableInitializerReplay:
    return "variable.initializer_replay";
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

ScopedUpstreamRoute::ScopedUpstreamRoute(WitnessUpstreamRoute route)
  : previous_(current_upstream_route())
{
  current_upstream_route() = route;
}

ScopedUpstreamRoute::~ScopedUpstreamRoute()
{
  current_upstream_route() = previous_;
}

void note_source_use_record(
    template_api::TemplateWitnessSession * session,
    SemanticSourceUseTable * table,
    WitnessProducerSite producer,
    const SemanticSourceUse & use,
    const semantic_source_use::SourceUseRecordResult & result)
{
  if(!enabled() || session == nullptr || table == nullptr) {
    return;
  }
  std::lock_guard<std::mutex> lock(trace_mutex());
  SessionState & state = state_for_session_locked(*session);
  const bool inserted =
      result.action == semantic_source_use::SourceUseRecordAction::Inserted;
  std::set<WitnessProducerSite> collided_producers;
  if(!inserted && result.row_index < state.rows.size()) {
    collided_producers = state.rows[result.row_index].producers;
  }

  reconcile_rows(state, *table);
  if(result.row_index < state.rows.size()) {
    state.rows[result.row_index].producers.insert(producer);
    if(current_upstream_route() != WitnessUpstreamRoute::Unknown) {
      state.rows[result.row_index].upstream_routes.insert(
          current_upstream_route());
    }
  }

  const std::string action = inserted ? "inserted" :
      (result.action == semantic_source_use::SourceUseRecordAction::ExactReplay ?
          "exact_duplicate" : "rejected");
  state.records.push_back(
      source_attempt_record(use,
                            producer,
                            current_upstream_route(),
                            action,
                            collided_producers));
}

void note_lifecycle_record(
    template_api::TemplateWitnessSession & session,
    WitnessProducerSite producer,
    const template_api::TemplateLifecycleEvent & event)
{
  if(!enabled()) {
    return;
  }
  std::lock_guard<std::mutex> lock(trace_mutex());
  SessionState & state = state_for_session_locked(session);
  const std::string key = lifecycle_event_key(event);
  std::set<WitnessProducerSite> collided_producers;
  const std::map<std::string, std::set<WitnessProducerSite> >::const_iterator
      collided = state.lifecycle_producers.find(key);
  if(collided != state.lifecycle_producers.end()) {
    collided_producers = collided->second;
  }
  state.lifecycle_producers[key].insert(producer);
  std::ostringstream record;
  record << "{\"record\":\"lifecycle_attempt\""
         << ",\"producer\":" << quoted(producer_site_name(producer))
         << ",\"action\":\"inserted\""
         << ",\"kind\":" << quoted(lifecycle_kind_name(event.kind))
         << ",\"location\":" << quoted(event.location)
         << ",\"entity\":" << quoted(event.entity)
         << ",\"cause\":" << static_cast<int>(event.cause)
         << ",\"entry_origin\":"
         << static_cast<int>(event.entry_context.origin)
         << ",\"closure_reason\":"
         << static_cast<int>(event.entry_context.closure_reason)
         << ",\"trigger_entity\":"
         << quoted(event.entry_context.trigger_entity)
         << ",\"trigger_decl_location\":"
         << quoted(event.entry_context.trigger_decl_location)
         << ",\"detail\":" << quoted(event.detail)
         << ",\"public_source_required\":"
         << (event.public_source_required ? "true" : "false")
         << ",\"collided_producers\":"
         << producer_array(collided_producers)
         << '}';
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
    out[i].upstream_routes.assign(
        state.rows[i].upstream_routes.begin(),
        state.rows[i].upstream_routes.end());
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
         << ",\"upstream_routes\":"
         << upstream_route_array(lineage.upstream_routes)
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
         << ",\"upstream_routes\":"
         << upstream_route_array(lineage.upstream_routes)
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
           << ",\"upstream_routes\":"
           << upstream_route_array(state.rows[i].upstream_routes)
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

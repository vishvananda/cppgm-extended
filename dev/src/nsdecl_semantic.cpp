#include "nsdecl_semantic.h"

#include <map>
#include <memory>
#include <sstream>

using namespace std;

#include "callsemantic.h"
#include "cpp_decl_bridge.h"
#include "cpp_decl_model.h"

namespace {

struct NamespaceSummary
{
  string name;
  bool named = false;
  bool is_inline = false;
  vector<pair<string, cpp_decl::TypePtr> > variables;
  vector<pair<string, cpp_decl::TypePtr> > functions;
  vector<unique_ptr<NamespaceSummary> > namespaces;
  map<string, size_t> variable_index;
  map<string, size_t> function_index;
  map<string, NamespaceSummary *> namespace_index;
};

string unqualified_name(const string & name)
{
  const size_t pos = name.rfind("::");
  return pos == string::npos ? name : name.substr(pos + 2);
}

vector<string> strip_current_path_prefix(const vector<string> & current_path,
                                         const cpp_decl::QualifiedName & qualified)
{
  vector<string> result = qualified.qualifiers;
  if(result.size() >= current_path.size()) {
    bool matches = true;
    for(size_t i = 0; i < current_path.size(); ++i) {
      if(result[i] != current_path[i]) {
        matches = false;
        break;
      }
    }
    if(matches) {
      result.erase(result.begin(), result.begin() + current_path.size());
    }
  }
  return result;
}

NamespaceSummary & ensure_namespace(NamespaceSummary & parent,
                                    const string & name,
                                    bool is_inline)
{
  map<string, NamespaceSummary *>::const_iterator found = parent.namespace_index.find(name);
  if(found != parent.namespace_index.end()) {
    if(is_inline && !found->second->is_inline) {
      throw logic_error("non-inline namespace cannot be reopened as inline");
    }
    found->second->is_inline = found->second->is_inline || is_inline;
    return *found->second;
  }

  unique_ptr<NamespaceSummary> created(new NamespaceSummary());
  created->name = name;
  created->named = name != "<unnamed>";
  created->is_inline = is_inline;
  NamespaceSummary * result = created.get();
  parent.namespace_index[name] = result;
  parent.namespaces.push_back(std::move(created));
  return *result;
}

NamespaceSummary & ensure_namespace_path(NamespaceSummary & summary,
                                         const vector<string> & qualifiers)
{
  NamespaceSummary * current = &summary;
  for(size_t i = 0; i < qualifiers.size(); ++i) {
    current = &ensure_namespace(*current, qualifiers[i], false);
  }
  return *current;
}

void collect_nsdecl_member(NamespaceSummary & summary,
                           const CallSemNode & node,
                           const vector<string> & current_path);

void collect_nsdecl_namespace(NamespaceSummary & summary,
                              const CallSemNode & node,
                              const vector<string> & current_path)
{
  NamespaceSummary & nested = ensure_namespace(summary, node.text, node.is_inline_namespace);
  vector<string> nested_path = current_path;
  if(node.text != "<unnamed>") {
    nested_path.push_back(node.text);
  }
  for(size_t i = 0; i < node.children.size(); ++i) {
    collect_nsdecl_member(nested, node.children[i], nested_path);
  }
}

void collect_nsdecl_member(NamespaceSummary & summary,
                           const CallSemNode & node,
                           const vector<string> & current_path)
{
  const string & emitted_name =
      callsem_resolved_name(node).empty() ? node.text.str() : callsem_resolved_name(node);

  if(node.kind == CallSemKind::namespace_definition) {
    collect_nsdecl_namespace(summary, node, current_path);
    return;
  }

  if(node.kind == CallSemKind::variable) {
    NamespaceSummary * target = &summary;
    string name = emitted_name;
    const cpp_decl::QualifiedName * qualified = callsem_qualified_name_syntax(node).get();
    if(qualified && (qualified->rooted || !qualified->qualifiers.empty())) {
      const vector<string> relative_qualifiers =
          strip_current_path_prefix(current_path, *qualified);
      target = &ensure_namespace_path(summary, relative_qualifiers);
      name = qualified->name;
    }
    if(target->variable_index.find(name) == target->variable_index.end()) {
      target->variable_index[name] = target->variables.size();
      target->variables.push_back(make_pair(name, node.semantic_type));
    }
    return;
  }

  if(node.kind == CallSemKind::function_declaration ||
     node.kind == CallSemKind::function_definition) {
    NamespaceSummary * target = &summary;
    string name = emitted_name;
    const cpp_decl::QualifiedName * qualified = callsem_qualified_name_syntax(node).get();
    if(qualified && (qualified->rooted || !qualified->qualifiers.empty())) {
      const vector<string> relative_qualifiers =
          strip_current_path_prefix(current_path, *qualified);
      target = &ensure_namespace_path(summary, relative_qualifiers);
      name = qualified->name;
    }
    const string key = unqualified_name(name);
    if(target->function_index.find(key) == target->function_index.end()) {
      target->function_index[key] = target->functions.size();
      target->functions.push_back(make_pair(unqualified_name(name), node.semantic_type));
    }
    return;
  }
}

NamespaceSummary summarize_nsdecl_output(const CallSemNode & node)
{
  if(node.kind != CallSemKind::translation_unit) {
    throw logic_error("nsdecl adapter expected translation-unit output");
  }

  NamespaceSummary root;
  for(size_t i = 0; i < node.children.size(); ++i) {
    collect_nsdecl_member(root, node.children[i], vector<string>());
  }
  return root;
}

void describe_namespace(ostringstream & out,
                        const NamespaceSummary & summary,
                        bool is_global)
{
  if(is_global || !summary.named) {
    out << "start unnamed namespace\n";
  } else {
    out << "start namespace " << summary.name << '\n';
  }

  if(!is_global && summary.is_inline) {
    out << "inline namespace\n";
  }

  for(size_t i = 0; i < summary.variables.size(); ++i) {
    out << "variable " << summary.variables[i].first << " "
        << cpp_decl::describe_type(summary.variables[i].second) << '\n';
  }

  for(size_t i = 0; i < summary.functions.size(); ++i) {
    out << "function " << summary.functions[i].first << " "
        << cpp_decl::describe_type(summary.functions[i].second) << '\n';
  }

  for(size_t i = 0; i < summary.namespaces.size(); ++i) {
    describe_namespace(out, *summary.namespaces[i], false);
  }

  out << "end namespace\n";
}

string describe_nsdecl_translation_unit_impl(const CallSemNode & node)
{
  const NamespaceSummary summary = summarize_nsdecl_output(node);
  ostringstream out;
  describe_namespace(out, summary, true);
  return out.str();
}

}  // namespace

std::string describe_nsdecl_translation_unit(IRecogTokenSequence & tokens)
{
  return describe_nsdecl_translation_unit_impl(analyze_calls_translation_unit(tokens));
}

std::string describe_nsdecl_translation_unit(const std::vector<RecogToken> & tokens)
{
  return describe_nsdecl_translation_unit_impl(analyze_calls_translation_unit(tokens));
}

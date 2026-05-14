#define IN_GCC 1

#include "gcc-plugin.h"
#include "plugin-version.h"

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "cp/cp-tree.h"
#include "input.h"
#include "tree.h"
#include "tree-pretty-print.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <set>
#include <sstream>
#include <string>
#include <vector>

int plugin_is_GPL_compatible;

namespace {

struct PluginConfig {
  std::string file_filter;
  std::string symbol_filter;
  bool skip_system_headers = true;
  bool enable_decls = true;
  bool enable_types = false;
  bool enable_pregenericize = true;
  bool walk_bodies = true;
};

PluginConfig g_config;
std::set<const_tree> g_seen_decl_events;
std::set<const_tree> g_seen_type_events;
std::set<std::string> g_seen_call_events;

std::string jsonEscape(const std::string &value) {
  std::string out;
  out.reserve(value.size() + 8);
  for (char ch : value) {
    switch (ch) {
    case '\\':
      out += "\\\\";
      break;
    case '"':
      out += "\\\"";
      break;
    case '\n':
      out += "\\n";
      break;
    case '\r':
      out += "\\r";
      break;
    case '\t':
      out += "\\t";
      break;
    default:
      out += ch;
      break;
    }
  }
  return out;
}

std::string renderLocation(location_t loc) {
  if (LOCATION_LOCUS(loc) == UNKNOWN_LOCATION) {
    return "";
  }
  expanded_location expanded = expand_location(loc);
  if (expanded.file == nullptr) {
    return "";
  }
  std::ostringstream out;
  out << expanded.file << ':' << expanded.line << ':' << expanded.column;
  return out.str();
}

bool shouldSkipLocation(location_t loc) {
  if (LOCATION_LOCUS(loc) == UNKNOWN_LOCATION) {
    return false;
  }
  if (g_config.skip_system_headers && in_system_header_at(loc)) {
    return true;
  }
  if (g_config.file_filter.empty()) {
    return false;
  }
  const std::string rendered = renderLocation(loc);
  return rendered.find(g_config.file_filter) == std::string::npos;
}

bool matchesSymbolFilter(const std::string &symbol) {
  return g_config.symbol_filter.empty() ||
         symbol.find(g_config.symbol_filter) != std::string::npos;
}

std::string renderDecl(tree decl, int flags = TFF_SCOPE) {
  if (decl == NULL_TREE) {
    return "";
  }
  return decl_as_string(decl, flags);
}

std::string renderType(tree type, int flags = TFF_SCOPE) {
  if (type == NULL_TREE) {
    return "";
  }
  return type_as_string(type, flags);
}

std::string renderTree(tree node) {
  if (node == NULL_TREE) {
    return "";
  }
  if (TYPE_P(node)) {
    return renderType(node);
  }
  if (DECL_P(node)) {
    return renderDecl(node);
  }
  char *raw = print_generic_expr_to_str(node);
  if (raw == nullptr) {
    return "";
  }
  std::string out(raw);
  free(raw);
  return out;
}

std::string templateDeclKind(tree tmpl) {
  if (tmpl == NULL_TREE || TREE_CODE(tmpl) != TEMPLATE_DECL) {
    return "unknown";
  }
  if (DECL_FUNCTION_TEMPLATE_P(tmpl)) {
    return "function";
  }
  if (DECL_CLASS_TEMPLATE_P(tmpl)) {
    return "class";
  }
  if (DECL_ALIAS_TEMPLATE_P(tmpl)) {
    return "alias";
  }
  if (DECL_TYPE_TEMPLATE_P(tmpl)) {
    return "type";
  }
  if (DECL_TEMPLATE_RESULT(tmpl) != NULL_TREE &&
      TREE_CODE(DECL_TEMPLATE_RESULT(tmpl)) == VAR_DECL) {
    return "variable";
  }
  return "unknown";
}

std::string declKind(tree decl) {
  if (decl == NULL_TREE) {
    return "unknown";
  }
  switch (TREE_CODE(decl)) {
  case FUNCTION_DECL:
    return "function";
  case VAR_DECL:
    return "variable";
  case TYPE_DECL:
    return "type";
  case FIELD_DECL:
    return "field";
  case TEMPLATE_DECL:
    return "template";
  default:
    return "unknown";
  }
}

std::string specializationKindFromDecl(tree decl) {
  if (decl == NULL_TREE || !DECL_LANG_SPECIFIC(decl) || !DECL_TEMPLATE_INFO(decl)) {
    return "";
  }
  if (DECL_EXPLICIT_INSTANTIATION(decl)) {
    return "explicit_instantiation";
  }
  if (DECL_TEMPLATE_SPECIALIZATION(decl)) {
    tree info = DECL_TEMPLATE_INFO(decl);
    if (info != NULL_TREE && TI_PARTIAL_INFO(info) != NULL_TREE) {
      return "partial";
    }
    return "explicit_specialization";
  }
  if (DECL_IMPLICIT_INSTANTIATION(decl)) {
    return "primary";
  }
  return "unknown";
}

std::string specializationKindFromType(tree type) {
  if (type == NULL_TREE || !CLASS_TYPE_P(type) || TYPE_TEMPLATE_INFO(type) == NULL_TREE) {
    return "";
  }
  if (CLASSTYPE_EXPLICIT_INSTANTIATION(type)) {
    return "explicit_instantiation";
  }
  if (CLASSTYPE_TEMPLATE_SPECIALIZATION(type)) {
    if (TI_PARTIAL_INFO(TYPE_TEMPLATE_INFO(type)) != NULL_TREE) {
      return "partial";
    }
    return "explicit_specialization";
  }
  if (CLASSTYPE_IMPLICIT_INSTANTIATION(type)) {
    return "primary";
  }
  return "unknown";
}

tree templateInfoForType(tree type) {
  if (type == NULL_TREE || !CLASS_TYPE_P(type)) {
    return NULL_TREE;
  }
  return TYPE_TEMPLATE_INFO(type);
}

bool hasConcreteTemplateArgs(tree args) {
  return args != NULL_TREE && !any_dependent_template_arguments_p(args);
}

std::string renderTemplateArgument(tree arg);

std::vector<std::string> renderTemplateArgumentLevel(tree level) {
  std::vector<std::string> rendered;
  if (level == NULL_TREE) {
    return rendered;
  }
  const int arg_count = NUM_TMPL_ARGS(level);
  rendered.reserve(arg_count);
  for (int i = 0; i < arg_count; ++i) {
    rendered.push_back(renderTemplateArgument(TREE_VEC_ELT(level, i)));
  }
  return rendered;
}

std::string renderTemplateArgument(tree arg) {
  if (arg == NULL_TREE) {
    return "";
  }
  if (TYPE_P(arg)) {
    return renderType(arg);
  }
  if (ARGUMENT_PACK_P(arg)) {
    std::ostringstream out;
    out << "pack<";
    tree pack_args = ARGUMENT_PACK_ARGS(arg);
    const int arg_count = pack_args != NULL_TREE ? TREE_VEC_LENGTH(pack_args) : 0;
    for (int i = 0; i < arg_count; ++i) {
      if (i != 0) {
        out << ", ";
      }
      out << renderTemplateArgument(TREE_VEC_ELT(pack_args, i));
    }
    out << '>';
    return out.str();
  }
  if (PACK_EXPANSION_P(arg)) {
    return renderTemplateArgument(PACK_EXPANSION_PATTERN(arg)) + "...";
  }
  return renderTree(arg);
}

std::string renderTemplateArgs(tree args) {
  if (args == NULL_TREE) {
    return "[]";
  }
  std::ostringstream out;
  out << '[';
  const int depth = TMPL_ARGS_DEPTH(args);
  for (int level = 1; level <= depth; ++level) {
    if (level != 1) {
      out << ", ";
    }
    tree level_args = TMPL_ARGS_LEVEL(args, level);
    out << '[';
    const int arg_count = level_args != NULL_TREE ? NUM_TMPL_ARGS(level_args) : 0;
    for (int i = 0; i < arg_count; ++i) {
      if (i != 0) {
        out << ", ";
      }
      out << renderTemplateArgument(TMPL_ARG(args, level, i));
    }
    out << ']';
  }
  out << ']';
  return out.str();
}

std::string renderTemplateBindings(tree tmpl, tree args) {
  if (tmpl == NULL_TREE || args == NULL_TREE || TREE_CODE(tmpl) != TEMPLATE_DECL) {
    return "[]";
  }
  tree parms = DECL_TEMPLATE_PARMS(tmpl);
  if (parms == NULL_TREE) {
    return "[]";
  }
  std::ostringstream out;
  out << '[';
  const int depth = std::min<int>(TMPL_ARGS_DEPTH(args), TMPL_PARMS_DEPTH(parms));
  bool first = true;
  for (int level = 1; level <= depth; ++level) {
    tree parm_level = TREE_VALUE(TREE_VEC_ELT(parms, level - 1));
    tree arg_level = TMPL_ARGS_LEVEL(args, level);
    if (parm_level == NULL_TREE || arg_level == NULL_TREE) {
      continue;
    }
    const int parm_count = TREE_VEC_LENGTH(parm_level);
    const int arg_count = NUM_TMPL_ARGS(arg_level);
    const int non_default_count =
        NON_DEFAULT_TEMPLATE_ARGS_COUNT(arg_level) != NULL_TREE
            ? TREE_INT_CST_LOW(NON_DEFAULT_TEMPLATE_ARGS_COUNT(arg_level))
            : arg_count;
    for (int i = 0; i < parm_count && i < arg_count; ++i) {
      tree parm = TREE_VALUE(TREE_VEC_ELT(parm_level, i));
      const char *name = parm != NULL_TREE && DECL_NAME(parm) != NULL_TREE
                             ? IDENTIFIER_POINTER(DECL_NAME(parm))
                             : "";
      if (!first) {
        out << ", ";
      }
      first = false;
      out << "{\"param\":\"" << jsonEscape(name) << "\",\"arg\":\""
          << jsonEscape(renderTemplateArgument(TREE_VEC_ELT(arg_level, i)))
          << "\",\"source\":\""
          << (i < non_default_count ? "non_default" : "defaulted") << "\"}";
    }
  }
  out << ']';
  return out.str();
}

std::string renderPartialSelection(tree info) {
  if (info == NULL_TREE || TI_PARTIAL_INFO(info) == NULL_TREE) {
    return "";
  }
  tree partial = TI_TEMPLATE(TI_PARTIAL_INFO(info));
  return renderDecl(partial, TFF_SCOPE | TFF_TEMPLATE_HEADER);
}

void emitJsonObject(const std::vector<std::pair<std::string, std::string>> &fields) {
  std::ostringstream out;
  out << '{';
  for (std::size_t i = 0; i < fields.size(); ++i) {
    if (i != 0) {
      out << ',';
    }
    out << '"' << fields[i].first << "\":" << fields[i].second;
  }
  out << '}';
  out << '\n';
  const std::string rendered = out.str();
  std::fwrite(rendered.data(), 1, rendered.size(), stderr);
}

std::string asJsonString(const std::string &value) {
  return std::string("\"") + jsonEscape(value) + "\"";
}

void emitTemplateDeclEvent(tree tmpl) {
  if (tmpl == NULL_TREE || TREE_CODE(tmpl) != TEMPLATE_DECL) {
    return;
  }
  if (!g_seen_decl_events.insert(tmpl).second) {
    return;
  }
  tree result = DECL_TEMPLATE_RESULT(tmpl);
  const location_t loc = DECL_SOURCE_LOCATION(result != NULL_TREE ? result : tmpl);
  if (shouldSkipLocation(loc)) {
    return;
  }
  const std::string display_name =
      renderDecl(result != NULL_TREE ? result : tmpl,
                 TFF_SCOPE | TFF_TEMPLATE_HEADER | TFF_NO_OMIT_DEFAULT_TEMPLATE_ARGUMENTS);
  if (!matchesSymbolFilter(display_name)) {
    return;
  }
  emitJsonObject({
      {"kind", asJsonString("template_decl")},
      {"template_kind", asJsonString(templateDeclKind(tmpl))},
      {"name", asJsonString(display_name)},
      {"location", asJsonString(renderLocation(loc))},
      {"primary", PRIMARY_TEMPLATE_P(tmpl) ? "true" : "false"},
      {"member_template", DECL_MEMBER_TEMPLATE_P(tmpl) ? "true" : "false"},
  });
}

void emitTemplatedDeclUseEvent(tree decl) {
  if (decl == NULL_TREE || !DECL_P(decl) || !DECL_LANG_SPECIFIC(decl) ||
      DECL_TEMPLATE_INFO(decl) == NULL_TREE) {
    return;
  }
  if (TREE_CODE(decl) != FUNCTION_DECL && TREE_CODE(decl) != VAR_DECL) {
    return;
  }
  if (!DECL_TEMPLATE_INSTANTIATION(decl) && !DECL_TEMPLATE_SPECIALIZATION(decl) &&
      !DECL_EXPLICIT_INSTANTIATION(decl)) {
    return;
  }
  if (!hasConcreteTemplateArgs(DECL_TI_ARGS(decl))) {
    return;
  }
  if (!g_seen_decl_events.insert(decl).second) {
    return;
  }
  const location_t loc = DECL_SOURCE_LOCATION(decl);
  if (shouldSkipLocation(loc)) {
    return;
  }
  const std::string display_name =
      renderDecl(decl, TFF_SCOPE | TFF_RETURN_TYPE | TFF_NO_OMIT_DEFAULT_TEMPLATE_ARGUMENTS);
  if (!matchesSymbolFilter(display_name)) {
    return;
  }
  tree primary = DECL_TI_TEMPLATE(decl);
  const std::string partial_selection = renderPartialSelection(DECL_TEMPLATE_INFO(decl));
  std::vector<std::pair<std::string, std::string>> fields = {
      {"kind", asJsonString("templated_decl_use")},
      {"decl_kind", asJsonString(declKind(decl))},
      {"name", asJsonString(display_name)},
      {"location", asJsonString(renderLocation(loc))},
      {"selection", asJsonString(specializationKindFromDecl(decl))},
      {"template", asJsonString(renderDecl(most_general_template(primary), TFF_SCOPE | TFF_TEMPLATE_HEADER))},
      {"args", asJsonString(renderTemplateArgs(DECL_TI_ARGS(decl)))},
      {"bindings", renderTemplateBindings(most_general_template(primary), DECL_TI_ARGS(decl))},
  };
  if (!partial_selection.empty()) {
    fields.push_back({"selected_partial", asJsonString(partial_selection)});
  }
  emitJsonObject(fields);
}

void emitClassTypeEvent(tree type) {
  if (type == NULL_TREE || !CLASS_TYPE_P(type) || templateInfoForType(type) == NULL_TREE) {
    return;
  }
  if (!CLASSTYPE_USE_TEMPLATE(type)) {
    return;
  }
  if (!hasConcreteTemplateArgs(CLASSTYPE_TI_ARGS(type))) {
    return;
  }
  if (!g_seen_type_events.insert(type).second) {
    return;
  }
  location_t loc = UNKNOWN_LOCATION;
  if (TYPE_NAME(type) != NULL_TREE && DECL_P(TYPE_NAME(type))) {
    loc = DECL_SOURCE_LOCATION(TYPE_NAME(type));
  }
  if (shouldSkipLocation(loc)) {
    return;
  }
  const std::string display_name = renderType(type, TFF_SCOPE | TFF_NO_OMIT_DEFAULT_TEMPLATE_ARGUMENTS);
  if (!matchesSymbolFilter(display_name)) {
    return;
  }
  tree info = templateInfoForType(type);
  tree primary = CLASSTYPE_TI_TEMPLATE(type);
  const std::string partial_selection = renderPartialSelection(info);
  std::vector<std::pair<std::string, std::string>> fields = {
      {"kind", asJsonString("class_type_use")},
      {"name", asJsonString(display_name)},
      {"location", asJsonString(renderLocation(loc))},
      {"selection", asJsonString(specializationKindFromType(type))},
      {"template", asJsonString(renderDecl(most_general_template(primary), TFF_SCOPE | TFF_TEMPLATE_HEADER))},
      {"args", asJsonString(renderTemplateArgs(CLASSTYPE_TI_ARGS(type)))},
      {"bindings", renderTemplateBindings(most_general_template(primary), CLASSTYPE_TI_ARGS(type))},
  };
  if (!partial_selection.empty()) {
    fields.push_back({"selected_partial", asJsonString(partial_selection)});
  }
  emitJsonObject(fields);
}

tree maybeTemplateCallee(tree call) {
  if (call == NULL_TREE) {
    return NULL_TREE;
  }
  tree callee = CALL_EXPR_FN(call);
  if (callee == NULL_TREE) {
    return NULL_TREE;
  }
  STRIP_ANY_LOCATION_WRAPPER(callee);
  STRIP_NOPS(callee);
  if (TREE_CODE(callee) == ADDR_EXPR) {
    callee = TREE_OPERAND(callee, 0);
  } else if (TREE_CODE(callee) == COMPONENT_REF) {
    tree member = TREE_OPERAND(callee, 1);
    if (member != NULL_TREE && DECL_P(member)) {
      callee = member;
    }
  }
  if (callee == NULL_TREE || !DECL_P(callee) || !DECL_LANG_SPECIFIC(callee) ||
      DECL_TEMPLATE_INFO(callee) == NULL_TREE) {
    return NULL_TREE;
  }
  if (!hasConcreteTemplateArgs(DECL_TI_ARGS(callee))) {
    return NULL_TREE;
  }
  return callee;
}

tree walkFunctionBody(tree *tp, int *walk_subtrees, void *) {
  tree node = *tp;
  if (node == NULL_TREE) {
    return NULL_TREE;
  }
  if (TREE_CODE(node) != CALL_EXPR) {
    return NULL_TREE;
  }
  tree callee = maybeTemplateCallee(node);
  if (callee == NULL_TREE) {
    return NULL_TREE;
  }
  const location_t loc = EXPR_HAS_LOCATION(node) ? EXPR_LOCATION(node) : DECL_SOURCE_LOCATION(callee);
  if (shouldSkipLocation(loc)) {
    return NULL_TREE;
  }
  const std::string display_name =
      renderDecl(callee, TFF_SCOPE | TFF_RETURN_TYPE | TFF_NO_OMIT_DEFAULT_TEMPLATE_ARGUMENTS);
  if (!matchesSymbolFilter(display_name)) {
    return NULL_TREE;
  }
  std::ostringstream key;
  key << renderLocation(loc) << '|' << display_name;
  if (!g_seen_call_events.insert(key.str()).second) {
    return NULL_TREE;
  }
  tree primary = DECL_TI_TEMPLATE(callee);
  emitJsonObject({
      {"kind", asJsonString("function_call")},
      {"location", asJsonString(renderLocation(loc))},
      {"selected", asJsonString(display_name)},
      {"selection", asJsonString(specializationKindFromDecl(callee))},
      {"template", asJsonString(renderDecl(most_general_template(primary), TFF_SCOPE | TFF_TEMPLATE_HEADER))},
      {"args", asJsonString(renderTemplateArgs(DECL_TI_ARGS(callee)))},
      {"bindings", renderTemplateBindings(most_general_template(primary), DECL_TI_ARGS(callee))},
  });
  *walk_subtrees = 0;
  return NULL_TREE;
}

void onStartUnit(void *, void *) {
  g_seen_decl_events.clear();
  g_seen_type_events.clear();
  g_seen_call_events.clear();
}

void onFinishDecl(void *event_data, void *) {
  tree decl = static_cast<tree>(event_data);
  if (decl == NULL_TREE) {
    return;
  }
  if (TREE_CODE(decl) == TEMPLATE_DECL) {
    emitTemplateDeclEvent(decl);
  }
}

void onFinishType(void *event_data, void *) {
  tree type = static_cast<tree>(event_data);
  emitClassTypeEvent(type);
}

void onPreGenericize(void *event_data, void *) {
  tree fndecl = static_cast<tree>(event_data);
  if (fndecl == NULL_TREE || TREE_CODE(fndecl) != FUNCTION_DECL) {
    return;
  }
  if (!g_config.walk_bodies) {
    return;
  }
  tree body = DECL_SAVED_TREE(fndecl);
  if (body == NULL_TREE) {
    return;
  }
  cp_walk_tree_without_duplicates(&body, walkFunctionBody, nullptr);
}

void parseArgs(plugin_name_args *plugin_info) {
  if (const char *value = std::getenv("GCC_TMPL_METRICS_FILE_SUBSTR")) {
    g_config.file_filter = value;
  }
  if (const char *value = std::getenv("GCC_TMPL_METRICS_SYMBOL_SUBSTR")) {
    g_config.symbol_filter = value;
  }
  if (const char *value = std::getenv("GCC_TMPL_METRICS_INCLUDE_SYSTEM_HEADERS")) {
    g_config.skip_system_headers = std::strcmp(value, "0") != 0;
  }
  if (const char *value = std::getenv("GCC_TMPL_METRICS_ENABLE_DECLS")) {
    g_config.enable_decls = std::strcmp(value, "0") != 0;
  }
  if (const char *value = std::getenv("GCC_TMPL_METRICS_ENABLE_TYPES")) {
    g_config.enable_types = std::strcmp(value, "0") != 0;
  }
  if (const char *value = std::getenv("GCC_TMPL_METRICS_ENABLE_PREGENERICIZE")) {
    g_config.enable_pregenericize = std::strcmp(value, "0") != 0;
  }
  if (const char *value = std::getenv("GCC_TMPL_METRICS_WALK_BODIES")) {
    g_config.walk_bodies = std::strcmp(value, "0") != 0;
  }
  for (int i = 0; i < plugin_info->argc; ++i) {
    const plugin_argument &arg = plugin_info->argv[i];
    const char *key = arg.key != nullptr ? arg.key : "";
    const char *value = arg.value != nullptr ? arg.value : "";
    if (std::strcmp(key, "file_substr") == 0) {
      g_config.file_filter = value;
    } else if (std::strcmp(key, "symbol_substr") == 0) {
      g_config.symbol_filter = value;
    } else if (std::strcmp(key, "include_system_headers") == 0) {
      g_config.skip_system_headers = false;
    }
  }
}

} // namespace

int plugin_init(plugin_name_args *plugin_info, plugin_gcc_version *version) {
  if (!plugin_default_version_check(version, &gcc_version)) {
    return 1;
  }
  parseArgs(plugin_info);
  register_callback(plugin_info->base_name, PLUGIN_START_UNIT, onStartUnit, nullptr);
  if (g_config.enable_decls) {
    register_callback(plugin_info->base_name, PLUGIN_FINISH_DECL, onFinishDecl, nullptr);
  }
  if (g_config.enable_types) {
    register_callback(plugin_info->base_name, PLUGIN_FINISH_TYPE, onFinishType, nullptr);
  }
  if (g_config.enable_pregenericize) {
    register_callback(plugin_info->base_name, PLUGIN_PRE_GENERICIZE, onPreGenericize, nullptr);
  }
  return 0;
}

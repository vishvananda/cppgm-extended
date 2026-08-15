#include "callsemantic/memory_census.h"
#include "class_template_mangle_info.h"

#include <algorithm>
#include <array>
#include <cstdlib>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <ostream>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

using namespace std;

namespace callsemantic {
namespace {

using namespace cpp_decl;
using namespace semantic_model;
using callsemantic_internal::IdentifierTokenSet;
using template_model::TemplateArgument;
using template_model::TemplateParameterInfo;
struct MemoryCensusBucket
{
  size_t count = 0;
  size_t bytes = 0;
};

size_t string_storage_bytes(const string & value);

enum ExactStringCategory
{
  ESC_TYPE_DISPLAY,
  ESC_TYPE_KEY,
  ESC_TYPE_QUALIFIED_NAME,
  ESC_TYPE_MANGLE,
  ESC_TEMPLATE_ARGUMENT,
  ESC_CLASS_QUALIFIED_NAME,
  ESC_CLASS_ARGUMENT_TEXT,
  ESC_CLASS_INSTANTIATION_KEY,
  ESC_SCOPE_NAME,
  ESC_SCOPE_NAMED_TYPE_KEY,
  ESC_FUNCTION_NAME,
  ESC_FUNCTION_INSTANTIATION_KEY,
  ESC_CLASS_TEMPLATE_INSTANTIATION_KEY,
  ESC_CLASS_TEMPLATE_REFERENCE_KEY,
  ESC_COUNT
};

const char * exact_string_category_name(ExactStringCategory category)
{
  static const char * names[ESC_COUNT] = {
      "type.display",
      "type.key",
      "type.qualified-name",
      "type.mangle",
      "template-argument",
      "class.qualified-name",
      "class.argument-text",
      "class.instantiation-key",
      "scope.name",
      "scope.named-type-key",
      "function.name",
      "function.instantiation-key",
      "class-template.instantiation-key",
      "class-template.reference-key"};
  return names[static_cast<size_t>(category)];
}

struct ExactStringPointerHash
{
  size_t operator()(const string * value) const
  {
    return hash<string>()(*value);
  }
};

struct ExactStringPointerEqual
{
  bool operator()(const string * lhs, const string * rhs) const
  {
    return *lhs == *rhs;
  }
};

struct ExactStringRetentionEntry
{
  size_t occurrences = 0;
  size_t storage_bytes = 0;
  array<size_t, ESC_COUNT> category_occurrences;
  array<size_t, ESC_COUNT> category_storage_bytes;

  ExactStringRetentionEntry()
  {
    category_occurrences.fill(0);
    category_storage_bytes.fill(0);
  }
};

class ExactStringRetentionCensus
{
public:
  void note(ExactStringCategory category, const string & value)
  {
    if(!seen_objects_.insert(&value).second) {
      return;
    }
    const size_t bytes = string_storage_bytes(value);
    ExactStringRetentionEntry & entry = entries_[&value];
    ++entry.occurrences;
    entry.storage_bytes += bytes;
    ++entry.category_occurrences[static_cast<size_t>(category)];
    entry.category_storage_bytes[static_cast<size_t>(category)] += bytes;
    ++category_occurrences_[static_cast<size_t>(category)];
    category_storage_bytes_[static_cast<size_t>(category)] += bytes;
  }

  void dump(ostream & out) const
  {
    struct Row
    {
      const string * value = nullptr;
      const ExactStringRetentionEntry * entry = nullptr;
      size_t one_copy_bytes = 0;
      size_t redundant_bytes = 0;
    };

    size_t total_storage_bytes = 0;
    size_t one_copy_storage_bytes = 0;
    size_t duplicate_objects = 0;
    array<size_t, ESC_COUNT> category_unique_contents = {};
    array<size_t, ESC_COUNT> category_duplicate_objects = {};
    array<size_t, ESC_COUNT> category_one_copy_storage_bytes = {};
    vector<Row> rows;
    rows.reserve(entries_.size());
    for(EntryMap::const_iterator it = entries_.begin();
        it != entries_.end();
        ++it) {
      const size_t one_copy_bytes =
          it->second.storage_bytes == 0 ? 0 : it->first->size() + 1;
      const size_t redundant_bytes =
          it->second.storage_bytes > one_copy_bytes ?
              it->second.storage_bytes - one_copy_bytes :
              0;
      total_storage_bytes += it->second.storage_bytes;
      one_copy_storage_bytes += one_copy_bytes;
      if(it->second.occurrences > 1) {
        duplicate_objects += it->second.occurrences - 1;
      }
      for(size_t category = 0; category < ESC_COUNT; ++category) {
        const size_t category_occurrences =
            it->second.category_occurrences[category];
        if(category_occurrences == 0) {
          continue;
        }
        ++category_unique_contents[category];
        category_duplicate_objects[category] += category_occurrences - 1;
        if(it->second.category_storage_bytes[category] != 0) {
          category_one_copy_storage_bytes[category] +=
              it->first->size() + 1;
        }
      }
      if(redundant_bytes != 0) {
        Row row;
        row.value = it->first;
        row.entry = &it->second;
        row.one_copy_bytes = one_copy_bytes;
        row.redundant_bytes = redundant_bytes;
        rows.push_back(row);
      }
    }
    sort(rows.begin(),
         rows.end(),
         [](const Row & lhs, const Row & rhs)
         {
           if(lhs.redundant_bytes != rhs.redundant_bytes) {
             return lhs.redundant_bytes > rhs.redundant_bytes;
           }
           return lhs.value->size() > rhs.value->size();
         });

    out << "semantic-memory-exact-strings"
        << " objects=" << seen_objects_.size()
        << " unique-contents=" << entries_.size()
        << " duplicate-objects=" << duplicate_objects
        << " storage-bytes=" << total_storage_bytes
        << " one-copy-bytes=" << one_copy_storage_bytes
        << " exact-redundant-bytes="
        << (total_storage_bytes - one_copy_storage_bytes)
        << '\n';
    for(size_t category = 0; category < ESC_COUNT; ++category) {
      out << "semantic-memory-exact-string-category"
          << " kind="
          << exact_string_category_name(
                 static_cast<ExactStringCategory>(category))
          << " objects=" << category_occurrences_[category]
          << " unique-contents=" << category_unique_contents[category]
          << " duplicate-objects=" << category_duplicate_objects[category]
          << " storage-bytes=" << category_storage_bytes_[category]
          << " one-copy-bytes="
          << category_one_copy_storage_bytes[category]
          << " exact-redundant-bytes="
          << (category_storage_bytes_[category] -
              category_one_copy_storage_bytes[category])
          << '\n';
    }
    const size_t row_count = min<size_t>(rows.size(), 40);
    for(size_t i = 0; i < row_count; ++i) {
      const Row & row = rows[i];
      string prefix = row.value->substr(0, 120);
      for(size_t j = 0; j < prefix.size(); ++j) {
        if(prefix[j] == '\n' || prefix[j] == '\r' || prefix[j] == '\t') {
          prefix[j] = ' ';
        }
      }
      out << "semantic-memory-exact-string-duplicate"
          << " rank=" << i + 1
          << " length=" << row.value->size()
          << " objects=" << row.entry->occurrences
          << " storage-bytes=" << row.entry->storage_bytes
          << " exact-redundant-bytes=" << row.redundant_bytes
          << " owners=";
      bool first_owner = true;
      for(size_t category = 0; category < ESC_COUNT; ++category) {
        if(row.entry->category_occurrences[category] == 0) {
          continue;
        }
        if(!first_owner) {
          out << ',';
        }
        first_owner = false;
        out << exact_string_category_name(
                   static_cast<ExactStringCategory>(category))
            << ':' << row.entry->category_occurrences[category];
      }
      out << " prefix={" << prefix << "}\n";
    }
  }

private:
  typedef unordered_map<const string *,
                        ExactStringRetentionEntry,
                        ExactStringPointerHash,
                        ExactStringPointerEqual> EntryMap;
  EntryMap entries_;
  unordered_set<const string *> seen_objects_;
  array<size_t, ESC_COUNT> category_occurrences_ = {};
  array<size_t, ESC_COUNT> category_storage_bytes_ = {};
};

class MemoryCensus
{
public:
  void note(const string & kind,
            size_t bytes,
            size_t count = 1)
  {
    MemoryCensusBucket & bucket = buckets_[kind];
    bucket.count += count;
    bucket.bytes += bytes;
  }

  void note_detail(const string & kind,
                   size_t bytes,
                   size_t count = 1)
  {
    MemoryCensusBucket & bucket = detail_buckets_[kind];
    bucket.count += count;
    bucket.bytes += bytes;
  }

  bool first_class_template_mangle_info(
      const ClassTemplateSpecializationMangleInfo * info)
  {
    return seen_class_template_mangle_infos_.insert(info).second;
  }

  bool first_class_template_mangle_arguments(const void * arguments)
  {
    return seen_class_template_mangle_arguments_.insert(arguments).second;
  }

  bool first_template_argument_text(const string * text)
  {
    return !text || seen_template_argument_texts_.insert(text).second;
  }

  void dump(ostream & out) const
  {
    vector<pair<string, MemoryCensusBucket> > ordered(buckets_.begin(), buckets_.end());
    sort(ordered.begin(),
         ordered.end(),
         [](const pair<string, MemoryCensusBucket> & lhs,
            const pair<string, MemoryCensusBucket> & rhs)
    {
      if(lhs.second.bytes != rhs.second.bytes) {
        return lhs.second.bytes > rhs.second.bytes;
      }
      return lhs.first < rhs.first;
    });

    size_t total_bytes = 0;
    size_t total_count = 0;
    for(size_t i = 0; i < ordered.size(); ++i) {
      total_bytes += ordered[i].second.bytes;
      total_count += ordered[i].second.count;
      out << "semantic-memory"
          << " kind=" << ordered[i].first
          << " count=" << ordered[i].second.count
          << " bytes=" << ordered[i].second.bytes
          << '\n';
    }
    out << "semantic-memory-total"
        << " categories=" << ordered.size()
        << " count=" << total_count
        << " bytes=" << total_bytes
        << '\n';

    vector<pair<string, MemoryCensusBucket> > details(detail_buckets_.begin(),
                                                      detail_buckets_.end());
    sort(details.begin(),
         details.end(),
         [](const pair<string, MemoryCensusBucket> & lhs,
            const pair<string, MemoryCensusBucket> & rhs)
    {
      if(lhs.second.bytes != rhs.second.bytes) {
        return lhs.second.bytes > rhs.second.bytes;
      }
      return lhs.first < rhs.first;
    });
    for(size_t i = 0; i < details.size(); ++i) {
      out << "semantic-memory-detail"
          << " kind=" << details[i].first
          << " count=" << details[i].second.count
          << " bytes=" << details[i].second.bytes
          << '\n';
    }
  }

private:
  map<string, MemoryCensusBucket> buckets_;
  map<string, MemoryCensusBucket> detail_buckets_;
  unordered_set<const ClassTemplateSpecializationMangleInfo *>
      seen_class_template_mangle_infos_;
  unordered_set<const void *> seen_class_template_mangle_arguments_;
  unordered_set<const string *> seen_template_argument_texts_;
};
size_t string_storage_bytes(const string & value)
{
  const char * data = value.data();
  const char * object_begin = reinterpret_cast<const char *>(&value);
  const char * object_end = object_begin + sizeof(value);
  if(data >= object_begin && data < object_end) {
    return 0;
  }
  return value.capacity() + 1;
}

size_t callsem_text_storage_bytes(const CallSemText &)
{
  return 0;
}

size_t callsem_resolved_name_storage_bytes(const CallSemNode & node)
{
  return node.extra ? callsem_text_storage_bytes(node.extra->resolved_name) : 0;
}

template<class T>
size_t vector_storage_bytes(const vector<T> & value)
{
  return value.capacity() * sizeof(T);
}

template<class T>
size_t vector_storage_bytes(const CppAstLazyVector<T> & value)
{
  return value.as_vector().capacity() * sizeof(T);
}

size_t vector_storage_bytes(const CallSemChildren & value)
{
  return value.storage_bytes();
}

template<class K, class V, class C, class A>
size_t map_storage_bytes(const map<K, V, C, A> & value)
{
  return value.size() * sizeof(typename map<K, V, C, A>::value_type);
}

template<class K, class V, class H, class E, class A>
size_t unordered_map_storage_bytes(const unordered_map<K, V, H, E, A> & value)
{
  return value.size() * sizeof(typename unordered_map<K, V, H, E, A>::value_type) +
         value.bucket_count() * sizeof(void *);
}

template<class T, class C, class A>
size_t set_storage_bytes(const set<T, C, A> & value)
{
  return value.size() * sizeof(typename set<T, C, A>::value_type);
}

template<class T, class H, class E, class A>
size_t unordered_set_storage_bytes(const unordered_set<T, H, E, A> & value)
{
  return value.size() * sizeof(typename unordered_set<T, H, E, A>::value_type) +
         value.bucket_count() * sizeof(void *);
}

size_t symbol_identity_payload_bytes(const symbol_linkage::SymbolIdentity & symbol)
{
  return string_storage_bytes(symbol.internal_symbol) +
         string_storage_bytes(symbol.object_symbol) +
         string_storage_bytes(symbol.thread_local_wrapper_object_symbol);
}

size_t callsem_symbol_storage_bytes(
    const CallSemNode & node,
    unordered_set<const symbol_linkage::SymbolIdentity *> * seen_callsem_symbols)
{
  if(!node.extra || !node.extra->symbol) {
    return 0;
  }
  const symbol_linkage::SymbolIdentity * symbol = node.extra->symbol.get();
  if(seen_callsem_symbols && !seen_callsem_symbols->insert(symbol).second) {
    return 0;
  }
  return sizeof(symbol_linkage::SymbolIdentity) +
         symbol_identity_payload_bytes(*symbol);
}

size_t qualified_name_payload_bytes(const QualifiedName * name)
{
  if(!name) {
    return 0;
  }
  size_t bytes = sizeof(QualifiedName) +
                 string_storage_bytes(name->name) +
                 vector_storage_bytes(name->qualifiers);
  for(size_t i = 0; i < name->qualifiers.size(); ++i) {
    bytes += string_storage_bytes(name->qualifiers[i]);
  }
  return bytes;
}

size_t qualified_name_payload_bytes(const shared_ptr<QualifiedName> & name)
{
  return qualified_name_payload_bytes(name.get());
}

struct RetainedAstMemoryBucket
{
  size_t ast_nodes = 0;
  size_t argument_syntaxes = 0;
  size_t template_ids = 0;
  size_t bytes = 0;
};

class RetainedAstMemoryCensus
{
public:
  void add_inline_ast(const CppAstNode & node, const string & owner)
  {
    add_ast(node, owner, false);
  }

  void add_heap_ast(const shared_ptr<CppAstNode> & node, const string & owner)
  {
    if(node) {
      add_ast(*node, owner, true);
    }
  }

  void add_heap_ast(const unique_ptr<CppAstNode> & node, const string & owner)
  {
    if(node) {
      add_ast(*node, owner, true);
    }
  }

  void add_heap_template_id(const shared_ptr<TemplateIdSyntax> & syntax,
                            const string & owner)
  {
    if(syntax) {
      add_template_id(*syntax, owner, true);
    }
  }

  void add_inline_template_id(const TemplateIdSyntax & syntax,
                              const string & owner)
  {
    add_template_id(syntax, owner, false);
  }

  void add_argument_syntax(const TemplateArgumentSyntax & syntax,
                           const string & owner,
                           bool heap_object)
  {
    if(!seen_argument_syntaxes_.insert(&syntax).second) {
      return;
    }
    RetainedAstMemoryBucket & bucket = buckets_[owner];
    ++bucket.argument_syntaxes;
    bucket.bytes +=
        (heap_object ? sizeof(TemplateArgumentSyntax) : 0) +
        string_storage_bytes(syntax.text) +
        string_storage_bytes(syntax.source_text);
    if(syntax.template_id) {
      add_template_id(*syntax.template_id, owner, true);
    }
    add_heap_ast(syntax.type_id, owner);
    add_heap_ast(syntax.source_type_id, owner);
    add_heap_ast(syntax.expression, owner);
  }

  void add_template_argument(const TemplateArgument & argument,
                             const string & owner)
  {
    static const char * kind_names[] = {
        "type",
        "value",
        "class_template",
        "alias_template"};
    const string classified_owner =
        owner +
        (argument.dependent ? ".dependent." : ".concrete.") +
        kind_names[static_cast<size_t>(argument.kind)];
    if(argument.source_syntax) {
      add_argument_syntax(*argument.source_syntax,
                          classified_owner + ".source_syntax",
                          true);
    }
    add_heap_ast(argument.expression, classified_owner + ".expression");
  }

  void dump(ostream & out) const
  {
    vector<pair<string, RetainedAstMemoryBucket> > rows(buckets_.begin(),
                                                        buckets_.end());
    sort(rows.begin(),
         rows.end(),
         [](const pair<string, RetainedAstMemoryBucket> & lhs,
            const pair<string, RetainedAstMemoryBucket> & rhs)
         {
           if(lhs.second.bytes != rhs.second.bytes) {
             return lhs.second.bytes > rhs.second.bytes;
           }
           return lhs.first < rhs.first;
         });
    size_t total_bytes = 0;
    size_t total_ast_nodes = 0;
    size_t total_argument_syntaxes = 0;
    size_t total_template_ids = 0;
    for(size_t i = 0; i < rows.size(); ++i) {
      const RetainedAstMemoryBucket & bucket = rows[i].second;
      total_bytes += bucket.bytes;
      total_ast_nodes += bucket.ast_nodes;
      total_argument_syntaxes += bucket.argument_syntaxes;
      total_template_ids += bucket.template_ids;
      out << "semantic-memory-retained-ast"
          << " owner=" << rows[i].first
          << " ast-nodes=" << bucket.ast_nodes
          << " argument-syntaxes=" << bucket.argument_syntaxes
          << " template-ids=" << bucket.template_ids
          << " bytes=" << bucket.bytes
          << '\n';
    }
    out << "semantic-memory-retained-ast-total"
        << " owners=" << rows.size()
        << " ast-nodes=" << total_ast_nodes
        << " argument-syntaxes=" << total_argument_syntaxes
        << " template-ids=" << total_template_ids
        << " bytes=" << total_bytes
        << '\n';
  }

private:
  template<class T>
  size_t lazy_vector_bytes(const CppAstLazyVector<T> & values) const
  {
    return values.empty() ?
        0 :
        sizeof(vector<T>) + vector_storage_bytes(values.as_vector());
  }

  void note_qualified_name(const QualifiedName & name,
                           const string & owner,
                           bool heap_object)
  {
    RetainedAstMemoryBucket & bucket = buckets_[owner];
    bucket.bytes +=
        (heap_object ? sizeof(QualifiedName) : 0) +
        string_storage_bytes(name.name) +
        vector_storage_bytes(name.qualifiers);
    for(size_t i = 0; i < name.qualifiers.size(); ++i) {
      bucket.bytes += string_storage_bytes(name.qualifiers[i]);
    }
  }

  void add_template_id(const TemplateIdSyntax & syntax,
                       const string & owner,
                       bool heap_object)
  {
    if(!seen_template_ids_.insert(&syntax).second) {
      return;
    }
    RetainedAstMemoryBucket & bucket = buckets_[owner];
    ++bucket.template_ids;
    bucket.bytes +=
        (heap_object ? sizeof(TemplateIdSyntax) : 0) +
        vector_storage_bytes(syntax.qualifier_template_id_syntaxes) +
        vector_storage_bytes(syntax.arguments) +
        vector_storage_bytes(syntax.argument_syntaxes);
    note_qualified_name(syntax.name, owner, false);
    for(size_t i = 0; i < syntax.qualifier_template_id_syntaxes.size(); ++i) {
      add_template_id(syntax.qualifier_template_id_syntaxes[i], owner, false);
    }
    for(size_t i = 0; i < syntax.arguments.size(); ++i) {
      bucket.bytes += string_storage_bytes(syntax.arguments[i]);
    }
    for(size_t i = 0; i < syntax.argument_syntaxes.size(); ++i) {
      add_argument_syntax(syntax.argument_syntaxes[i], owner, false);
    }
  }

  void add_ast(const CppAstNode & node,
               const string & owner,
               bool heap_object)
  {
    if(!seen_ast_nodes_.insert(&node).second) {
      return;
    }
    RetainedAstMemoryBucket & bucket = buckets_[owner];
    ++bucket.ast_nodes;
    bucket.bytes +=
        (heap_object ? sizeof(CppAstNode) : 0) +
        string_storage_bytes(node.value) +
        vector_storage_bytes(node.children) +
        lazy_vector_bytes(node.qualifier_template_id_syntaxes) +
        lazy_vector_bytes(node.qualifier_type_syntaxes) +
        lazy_vector_bytes(node.exception_type_id_syntaxes) +
        lazy_vector_bytes(node.alignment_specifier_nodes);

    if(node.qualified_name_syntax &&
       seen_qualified_names_.insert(node.qualified_name_syntax.get()).second) {
      note_qualified_name(*node.qualified_name_syntax, owner, true);
    }
    if(node.template_id_syntax) {
      add_template_id(*node.template_id_syntax, owner, true);
    }
    for(size_t i = 0; i < node.qualifier_template_id_syntaxes.size(); ++i) {
      add_template_id(node.qualifier_template_id_syntaxes[i], owner, false);
    }
    for(size_t i = 0; i < node.qualifier_type_syntaxes.size(); ++i) {
      add_ast(node.qualifier_type_syntaxes[i], owner, false);
    }
    for(size_t i = 0; i < node.exception_type_id_syntaxes.size(); ++i) {
      add_ast(node.exception_type_id_syntaxes[i], owner, false);
    }
    for(size_t i = 0; i < node.alignment_specifier_nodes.size(); ++i) {
      add_ast(node.alignment_specifier_nodes[i], owner, false);
    }
    for(size_t i = 0; i < node.children.size(); ++i) {
      add_ast(node.children[i], owner, false);
    }

    const CppAstSparseData * sparse =
        node.sparse_data ? &*node.sparse_data : nullptr;
    if(sparse && seen_sparse_data_.insert(sparse).second) {
      bucket.bytes +=
          sizeof(CppAstSparseData) +
          string_storage_bytes(sparse->builtin_type_transform_name) +
          string_storage_bytes(
              sparse->rare_strings.gnu_ext_vector_type_argument_identifier) +
          string_storage_bytes(sparse->rare_strings.gnu_section_segment) +
          string_storage_bytes(sparse->rare_strings.gnu_section_name) +
          string_storage_bytes(sparse->rare_strings.asm_label) +
          lazy_vector_bytes(sparse->abi_tags) +
          lazy_vector_bytes(sparse->alignment_specifiers);
      for(size_t i = 0; i < sparse->abi_tags.size(); ++i) {
        bucket.bytes += string_storage_bytes(sparse->abi_tags[i]);
      }
      for(size_t i = 0; i < sparse->alignment_specifiers.size(); ++i) {
        bucket.bytes += string_storage_bytes(sparse->alignment_specifiers[i]);
      }
      if(sparse->name_lookup_snapshot &&
         seen_name_lookup_snapshots_.insert(
             sparse->name_lookup_snapshot.get()).second) {
        bucket.bytes += sizeof(CppAstNameLookupSnapshot);
      }
      add_heap_ast(sparse->conversion_type_id_syntax, owner);
      add_heap_ast(sparse->base_type_syntax, owner);
    }
  }

  map<string, RetainedAstMemoryBucket> buckets_;
  unordered_set<const CppAstNode *> seen_ast_nodes_;
  unordered_set<const TemplateArgumentSyntax *> seen_argument_syntaxes_;
  unordered_set<const TemplateIdSyntax *> seen_template_ids_;
  unordered_set<const QualifiedName *> seen_qualified_names_;
  unordered_set<const CppAstSparseData *> seen_sparse_data_;
  unordered_set<const CppAstNameLookupSnapshot *> seen_name_lookup_snapshots_;
};

thread_local RetainedAstMemoryCensus * active_retained_ast_memory_census =
    nullptr;

class ScopedRetainedAstMemoryCensus
{
public:
  explicit ScopedRetainedAstMemoryCensus(RetainedAstMemoryCensus & census)
    : previous_(active_retained_ast_memory_census)
  {
    active_retained_ast_memory_census = &census;
  }

  ~ScopedRetainedAstMemoryCensus()
  {
    active_retained_ast_memory_census = previous_;
  }

private:
  RetainedAstMemoryCensus * previous_;
};

void census_type(const TypePtr & type,
                 MemoryCensus & census,
                 unordered_set<const Type *> & seen_types);

void census_cpp_ast_node(const CppAstNode & node,
                         const string & kind,
                         MemoryCensus & census);

void census_callsem_node(const CallSemNode & node,
                         const string & kind,
                         MemoryCensus & census,
                         unordered_set<const Type *> & seen_types,
                         unordered_set<const CallSemNodeExtra *> & seen_callsem_extras,
                         unordered_set<const CallSemRareStrings *> &
                             seen_callsem_rare_strings,
                         unordered_set<const CallSemRarePayload *> &
                             seen_callsem_rare_payloads,
                         unordered_set<uint32_t> & seen_callsem_source_files,
                         unordered_set<const symbol_linkage::SymbolIdentity *> &
                             seen_callsem_symbols);

size_t template_parameter_payload_bytes(const TemplateParameterInfo & info,
                                        MemoryCensus & census,
                                        unordered_set<const Type *> & seen_types)
{
  size_t bytes = string_storage_bytes(info.name) +
                 string_storage_bytes(info.placeholder_key) +
                 string_storage_bytes(info.non_type_decl_specifier_text) +
                 vector_storage_bytes(info.alternate_names);
  for(size_t i = 0; i < info.alternate_names.size(); ++i) {
    bytes += string_storage_bytes(info.alternate_names[i]);
  }
  if(info.owned_syntax) {
    bytes += sizeof(template_model::TemplateParameterOwnedSyntax);
    if(info.owned_syntax->non_type_decl_specifier_seq) {
      if(active_retained_ast_memory_census) {
        active_retained_ast_memory_census->add_heap_ast(
            info.owned_syntax->non_type_decl_specifier_seq,
            "template_parameter.owned_syntax");
      }
      census_cpp_ast_node(*info.owned_syntax->non_type_decl_specifier_seq,
                          "template_parameter.syntax",
                          census);
    }
    if(info.owned_syntax->non_type_declarator) {
      if(active_retained_ast_memory_census) {
        active_retained_ast_memory_census->add_heap_ast(
            info.owned_syntax->non_type_declarator,
            "template_parameter.owned_syntax");
      }
      census_cpp_ast_node(*info.owned_syntax->non_type_declarator,
                          "template_parameter.syntax",
                          census);
    }
    if(info.owned_syntax->non_type_abstract_declarator) {
      if(active_retained_ast_memory_census) {
        active_retained_ast_memory_census->add_heap_ast(
            info.owned_syntax->non_type_abstract_declarator,
            "template_parameter.owned_syntax");
      }
      census_cpp_ast_node(*info.owned_syntax->non_type_abstract_declarator,
                          "template_parameter.syntax",
                          census);
    }
    if(info.owned_syntax->default_argument) {
      if(active_retained_ast_memory_census) {
        active_retained_ast_memory_census->add_heap_ast(
            info.owned_syntax->default_argument,
            "template_parameter.owned_syntax");
      }
      census_cpp_ast_node(*info.owned_syntax->default_argument,
                          "template_parameter.syntax",
                          census);
    }
  }
  census_type(info.value_type, census, seen_types);
  return bytes;
}

size_t template_argument_payload_bytes(const TemplateArgument & arg,
                                       MemoryCensus & census,
                                       unordered_set<const Type *> & seen_types,
                                       const char * retained_owner)
{
  if(active_retained_ast_memory_census) {
    active_retained_ast_memory_census->add_template_argument(
        arg, retained_owner);
  }
  census.note_detail("template_argument.base", sizeof(TemplateArgument));
  if(arg.kind != TemplateArgument::TA_TYPE) {
    census.note_detail("template_argument.non_type_kind", sizeof(arg.kind));
  }
  if(arg.template_decl) {
    census.note_detail("template_argument.nonempty.template_decl",
                       sizeof(arg.template_decl));
  }
  if(arg.template_owner_type) {
    census.note_detail("template_argument.nonempty.template_owner_type",
                       sizeof(arg.template_owner_type));
  }
  if(arg.template_entity_identity) {
    census.note_detail("template_argument.nonempty.template_entity_identity",
                       sizeof(arg.template_entity_identity));
  }
  if(arg.rare_data) {
    census.note_detail("template_argument.nonempty.rare_data",
                       sizeof(arg.rare_data) +
                           sizeof(TemplateArgument::RareData));
  }
  const TemplateArgument::RareData & rare = arg.rare();
  if(rare.function_value) {
    census.note_detail("template_argument.nonempty.function_value",
                       sizeof(rare.function_value));
  }
  if(!rare.function_internal_symbol.empty()) {
    census.note_detail("template_argument.nonempty.function_internal_symbol",
                       sizeof(rare.function_internal_symbol));
  }
  if(rare.value_binding) {
    census.note_detail("template_argument.nonempty.value_binding",
                       sizeof(rare.value_binding));
  }
  if(!rare.value_dependencies.empty()) {
    census.note_detail("template_argument.nonempty.value_dependencies",
                       vector_storage_bytes(rare.value_dependencies));
  }
  if(arg.source_syntax) {
    census.note_detail("template_argument.nonempty.source_syntax",
                       sizeof(arg.source_syntax));
  }
  if(arg.expression) {
    census.note_detail("template_argument.nonempty.expression",
                       sizeof(arg.expression));
  }
  if(arg.dependent) {
    census.note_detail("template_argument.true.dependent",
                       sizeof(arg.dependent));
  }
  if(arg.source_defaulted) {
    census.note_detail("template_argument.true.source_defaulted",
                       sizeof(arg.source_defaulted));
  }
  if(arg.partial_order_placeholder) {
    census.note_detail("template_argument.true.partial_order_placeholder",
                       sizeof(arg.partial_order_placeholder));
  }
  const string * text_identity = arg.text.storage_identity();
  size_t bytes =
      census.first_template_argument_text(text_identity) ?
          string_storage_bytes(arg.text) :
          0;
  census_type(arg.type, census, seen_types);
  return bytes;
}

size_t value_binding_payload_bytes(const ValueBinding & binding,
                                   MemoryCensus & census,
                                   unordered_set<const Type *> & seen_types)
{
  size_t bytes = string_storage_bytes(binding.name) +
                 string_storage_bytes(binding.anonymous_storage_field_name) +
                 string_storage_bytes(binding.anonymous_storage_variable_name) +
                 symbol_identity_payload_bytes(binding.symbol);
  census_type(binding.type, census, seen_types);
  return bytes;
}

void census_type(const TypePtr & type,
                 MemoryCensus & census,
                 unordered_set<const Type *> & seen_types)
{
  if(!type || !seen_types.insert(type.get()).second) {
    return;
  }

  const size_t named_display_bytes = string_storage_bytes(type->named_display);
  const size_t named_key_bytes = string_storage_bytes(type->named_key);
  const size_t named_semantic_payload_bytes =
      string_storage_bytes(type->named_semantic_payload);
  const size_t bound_text_bytes = string_storage_bytes(type->bound_text);
  static const char * type_kind_names[] = {
      "fundamental",
      "named",
      "cv",
      "atomic",
      "pointer",
      "member_pointer",
      "block_pointer",
      "lvalue_reference",
      "rvalue_reference",
      "array",
      "function"};
  census.note_detail(
      string("type.kind.") +
          type_kind_names[static_cast<size_t>(type->kind)],
      sizeof(Type));
  size_t bytes = sizeof(Type) +
                 named_display_bytes +
                 named_key_bytes +
                 named_semantic_payload_bytes +
                 bound_text_bytes +
                 vector_storage_bytes(type->named_host_abi_chunks) +
                 vector_storage_bytes(type->params);
  census.note_detail("type.string_capacity.named_display", named_display_bytes);
  census.note_detail("type.string_capacity.named_key", named_key_bytes);
  census.note_detail("type.string_capacity.named_semantic_payload",
                     named_semantic_payload_bytes);
  census.note_detail("type.string_capacity.bound_text", bound_text_bytes);
  if(type->named_rare_metadata) {
    const Type::NamedRareMetadata & syntax = *type->named_rare_metadata;
    if(active_retained_ast_memory_census) {
      active_retained_ast_memory_census->add_heap_ast(
          syntax.named_dependent_type_expression_node,
          "type.dependent_expression");
      for(size_t i = 0;
          i < syntax.named_dependent_alias_arguments.size();
          ++i) {
        active_retained_ast_memory_census->add_argument_syntax(
            syntax.named_dependent_alias_arguments[i].syntax,
            "type.dependent_alias_argument",
            false);
      }
      for(size_t i = 0;
          i < syntax.named_dependent_class_arguments.size();
          ++i) {
        active_retained_ast_memory_census->add_argument_syntax(
            syntax.named_dependent_class_arguments[i].syntax,
            "type.dependent_class_argument",
            false);
      }
      for(size_t i = 0;
          i < syntax.named_dependent_template_template_arguments.size();
          ++i) {
        active_retained_ast_memory_census->add_argument_syntax(
            syntax.named_dependent_template_template_arguments[i].syntax,
            "type.dependent_template_template_argument",
            false);
      }
      if(syntax.named_dependent_qualified_owner_template_id) {
        active_retained_ast_memory_census->add_heap_template_id(
            syntax.named_dependent_qualified_owner_template_id,
            "type.dependent_qualified_owner_template_id");
      }
      for(size_t i = 0;
          i < syntax.named_dependent_qualified_member_template_ids.size();
          ++i) {
        active_retained_ast_memory_census->add_inline_template_id(
            syntax.named_dependent_qualified_member_template_ids[i],
            "type.dependent_qualified_member_template_id");
      }
    }
    const size_t qualified_name_bytes =
        qualified_name_payload_bytes(&syntax.qualified_name);
    const size_t source_name_bytes = string_storage_bytes(syntax.source_name);
    const size_t dependent_template_name_bytes =
        string_storage_bytes(
            syntax.named_dependent_template_template_parameter_name);
    const size_t member_name_bytes =
        string_storage_bytes(syntax.named_member_name);
    size_t abi_tag_bytes = vector_storage_bytes(syntax.named_abi_tags);
    for(size_t i = 0; i < syntax.named_abi_tags.size(); ++i) {
      abi_tag_bytes += string_storage_bytes(syntax.named_abi_tags[i]);
    }
    bytes += sizeof(Type::NamedRareMetadata) +
             qualified_name_bytes +
             source_name_bytes +
             dependent_template_name_bytes +
             member_name_bytes +
             abi_tag_bytes;
    census.note_detail("type.rare.qualified_name", qualified_name_bytes);
    census.note_detail("type.rare.string_capacity.source_name",
                       source_name_bytes);
    census.note_detail("type.rare.string_capacity.dependent_template_name",
                       dependent_template_name_bytes);
    census.note_detail("type.rare.string_capacity.member_name",
                       member_name_bytes);
    census.note_detail("type.rare.abi_tags", abi_tag_bytes);
    if(syntax.named_class_template_specialization_mangle_info &&
       census.first_class_template_mangle_info(
           syntax.named_class_template_specialization_mangle_info.get())) {
      const ClassTemplateSpecializationMangleInfo & mangle =
          *syntax.named_class_template_specialization_mangle_info;
      const size_t base_bytes =
          sizeof(ClassTemplateSpecializationMangleInfo);
      const size_t name_syntax_bytes =
          qualified_name_payload_bytes(&mangle.template_name_syntax);
      const size_t scope_prefix_bytes =
          string_storage_bytes(mangle.template_scope_prefix);
      const size_t template_name_bytes =
          string_storage_bytes(mangle.template_name);
      const size_t template_parameter_storage_bytes =
          mangle.template_parameters.owned_capacity() *
          sizeof(TemplateParameterInfo);
      const size_t mangle_parameter_storage_bytes =
          vector_storage_bytes(mangle.mangle_parameters);
      const size_t mangle_argument_storage_bytes =
          vector_storage_bytes(mangle.mangle_arguments);
      const size_t argument_storage_bytes =
          census.first_class_template_mangle_arguments(
              mangle.arguments.storage_identity()) ?
              mangle.arguments.storage_capacity() *
                  sizeof(TemplateArgument) :
              0;
      const size_t argument_syntax_storage_bytes =
          vector_storage_bytes(mangle.argument_syntaxes);
      const size_t pack_size_storage_bytes =
          map_storage_bytes(mangle.pack_sizes);
      size_t mangle_bytes =
          base_bytes +
          name_syntax_bytes +
          scope_prefix_bytes +
          template_name_bytes +
          template_parameter_storage_bytes +
          mangle_parameter_storage_bytes +
          mangle_argument_storage_bytes +
          argument_storage_bytes +
          argument_syntax_storage_bytes +
          pack_size_storage_bytes;
      census.note_detail("type.mangle.base", base_bytes);
      census.note_detail("type.mangle.name_syntax", name_syntax_bytes);
      census.note_detail("type.mangle.scope_prefix", scope_prefix_bytes);
      census.note_detail("type.mangle.template_name", template_name_bytes);
      census.note_detail("type.mangle.template_parameter_storage",
                         template_parameter_storage_bytes);
      census.note_detail("type.mangle.mangle_parameter_storage",
                         mangle_parameter_storage_bytes);
      census.note_detail("type.mangle.mangle_argument_storage",
                         mangle_argument_storage_bytes);
      census.note_detail("type.mangle.argument_storage",
                         argument_storage_bytes);
      census.note_detail("type.mangle.argument_syntax_storage",
                         argument_syntax_storage_bytes);
      census.note_detail("type.mangle.pack_size_storage",
                         pack_size_storage_bytes);
      census.note_detail(
          syntax.named_class_info ?
              "type.mangle.with_class_info" :
              "type.mangle.without_class_info",
          base_bytes);
      if(mangle.template_parameters.owns_values()) {
        for(size_t i = 0; i < mangle.template_parameters.size(); ++i) {
          mangle_bytes += template_parameter_payload_bytes(
              mangle.template_parameters[i], census, seen_types);
        }
      }
      for(size_t i = 0; i < mangle.mangle_parameters.size(); ++i) {
        mangle_bytes += template_parameter_payload_bytes(
            mangle.mangle_parameters[i], census, seen_types);
      }
      for(size_t i = 0; i < mangle.mangle_arguments.size(); ++i) {
        mangle_bytes += template_argument_payload_bytes(
            mangle.mangle_arguments[i],
            census,
            seen_types,
            "type.mangle.mangle_argument");
      }
      for(size_t i = 0; i < mangle.arguments.const_values().size(); ++i) {
        mangle_bytes += template_argument_payload_bytes(
            mangle.arguments.const_values()[i],
            census,
            seen_types,
            "type.mangle.argument");
      }
      if(active_retained_ast_memory_census) {
        for(size_t i = 0; i < mangle.argument_syntaxes.size(); ++i) {
          active_retained_ast_memory_census->add_argument_syntax(
              mangle.argument_syntaxes[i],
              "type.mangle.argument_syntax",
              false);
        }
      }
      for(map<string, size_t>::const_iterator it = mangle.pack_sizes.begin();
          it != mangle.pack_sizes.end();
          ++it) {
        mangle_bytes += string_storage_bytes(it->first);
      }
      bytes += mangle_bytes;
      census.note_detail("type.rare.class_template_mangle", mangle_bytes);
    }
  }
  census.note("type", bytes);

  census_type(type->inner, census, seen_types);
  census_type(type->owner, census, seen_types);
  for(size_t i = 0; i < type->params.size(); ++i) {
    census_type(type->params[i], census, seen_types);
  }
}

void census_cpp_ast_node(const CppAstNode & node,
                         const string & kind,
                         MemoryCensus & census)
{
  if(kind == "cppast.source") {
    census.note_detail("cppast.source.base", sizeof(CppAstNode));
    if(!node.value.empty()) {
      census.note_detail("cppast.source.nonempty.value",
                         string_storage_bytes(node.value));
    }
    if(!node.children.empty()) {
      census.note_detail("cppast.source.nonempty.children",
                         vector_storage_bytes(node.children));
    }
    if(!cppast_builtin_type_transform_name(node).empty()) {
      census.note_detail("cppast.source.nonempty.builtin_type_transform_name",
                         string_storage_bytes(
                             cppast_builtin_type_transform_name(node)));
    }
    if(node.semantic_type) {
      census.note_detail("cppast.source.nonempty.semantic_type",
                         sizeof(node.semantic_type));
    }
    if(node.qualified_name_syntax) {
      census.note_detail("cppast.source.nonempty.qualified_name_syntax",
                         sizeof(node.qualified_name_syntax));
    }
    if(node.template_id_syntax) {
      census.note_detail("cppast.source.nonempty.template_id_syntax",
                         sizeof(node.template_id_syntax));
    }
    if(cppast_conversion_type_id_syntax_storage(node)) {
      census.note_detail("cppast.source.cppast_conversion_type_id_syntax_storage(nonempty)",
                         sizeof(cppast_conversion_type_id_syntax_storage(node)));
    }
    if(cppast_base_type_syntax_storage(node)) {
      census.note_detail("cppast.source.cppast_base_type_syntax_storage(nonempty)",
                         sizeof(cppast_base_type_syntax_storage(node)));
    }
    if(!node.qualifier_template_id_syntaxes.empty()) {
      census.note_detail(
          "cppast.source.nonempty.qualifier_template_id_syntaxes",
          vector_storage_bytes(node.qualifier_template_id_syntaxes));
    }
    if(!node.qualifier_type_syntaxes.empty()) {
      census.note_detail("cppast.source.nonempty.qualifier_type_syntaxes",
                         vector_storage_bytes(node.qualifier_type_syntaxes));
    }
    if(!node.exception_type_id_syntaxes.empty()) {
      census.note_detail("cppast.source.nonempty.exception_type_id_syntaxes",
                         vector_storage_bytes(node.exception_type_id_syntaxes));
    }
    if(cppast_has_rare_strings(node)) {
      census.note_detail("cppast.source.nonempty.rare_strings",
                         sizeof(CppAstRareStrings));
    }
    if(!cppast_abi_tags(node).empty()) {
      census.note_detail("cppast.source.nonempty.abi_tags",
                         vector_storage_bytes(cppast_abi_tags(node)));
    }
    if(!cppast_alignment_specifiers(node).empty()) {
      census.note_detail("cppast.source.nonempty.alignment_specifiers",
                         vector_storage_bytes(
                             cppast_alignment_specifiers(node)));
    }
    if(!node.alignment_specifier_nodes.empty()) {
      census.note_detail("cppast.source.nonempty.alignment_specifier_nodes",
                         vector_storage_bytes(
                             node.alignment_specifier_nodes));
    }
    if(cppast_name_lookup_snapshot(node)) {
      census.note_detail("cppast.source.nonempty.name_lookup_snapshot",
                         sizeof(CppAstNameLookupSnapshot));
    }
  }
  size_t bytes = sizeof(CppAstNode) +
                 string_storage_bytes(node.value) +
                 vector_storage_bytes(cppast_abi_tags(node)) +
                 vector_storage_bytes(cppast_alignment_specifiers(node)) +
                 vector_storage_bytes(node.alignment_specifier_nodes) +
                 vector_storage_bytes(node.children);
  const CppAstLazyVector<std::string> & abi_tags = cppast_abi_tags(node);
  for(size_t i = 0; i < abi_tags.size(); ++i) {
    bytes += string_storage_bytes(abi_tags[i]);
  }
  const CppAstLazyVector<std::string> & alignment_specifiers =
      cppast_alignment_specifiers(node);
  for(size_t i = 0; i < alignment_specifiers.size(); ++i) {
    bytes += string_storage_bytes(alignment_specifiers[i]);
  }
  census.note(kind, bytes);
  for(size_t i = 0; i < node.alignment_specifier_nodes.size(); ++i) {
    census_cpp_ast_node(node.alignment_specifier_nodes[i], kind, census);
  }
  for(size_t i = 0; i < node.children.size(); ++i) {
    census_cpp_ast_node(node.children[i], kind, census);
  }
}

void census_callsem_node(const CallSemNode & node,
                         const string & kind,
                         MemoryCensus & census,
                         unordered_set<const Type *> & seen_types,
                         unordered_set<const CallSemNodeExtra *> & seen_callsem_extras,
                         unordered_set<const CallSemRareStrings *> &
                             seen_callsem_rare_strings,
                         unordered_set<const CallSemRarePayload *> &
                             seen_callsem_rare_payloads,
                         unordered_set<uint32_t> & seen_callsem_source_files,
                         unordered_set<const symbol_linkage::SymbolIdentity *> &
                             seen_callsem_symbols)
{
  const size_t text_bytes = callsem_text_storage_bytes(node.text);
  const size_t resolved_name_bytes = callsem_resolved_name_storage_bytes(node);
  const size_t children_bytes = vector_storage_bytes(node.children);
  size_t extra_bytes = 0;
  size_t rare_string_bytes = 0;
  size_t rare_payload_bytes = 0;
  size_t source_file_bytes = 0;
  size_t vtt_symbol_bytes = 0;
  size_t vtt_object_symbol_bytes = 0;
  size_t runtime_bridge_symbol_bytes = 0;
  size_t local_static_guard_symbol_bytes = 0;
  size_t virtual_base_layout_bytes = 0;
  size_t virtual_base_layout_string_bytes = 0;
  if(node.extra && seen_callsem_extras.insert(node.extra.get()).second) {
    extra_bytes = sizeof(CallSemNodeExtra);
  }
  const CallSemRarePayload * rare_payload = callsem_rare_payload(node);
  if(rare_payload &&
     seen_callsem_rare_payloads.insert(rare_payload).second) {
    rare_payload_bytes = sizeof(CallSemRarePayload);
    virtual_base_layout_bytes =
        vector_storage_bytes(rare_payload->virtual_base_layout);
    for(size_t i = 0; i < rare_payload->virtual_base_layout.size(); ++i) {
      virtual_base_layout_string_bytes +=
          string_storage_bytes(rare_payload->virtual_base_layout[i].first);
    }
  }
  if(node.extra &&
     node.extra->rare_strings &&
     seen_callsem_rare_strings.insert(node.extra->rare_strings.get()).second) {
    rare_string_bytes = sizeof(CallSemRareStrings);
    vtt_symbol_bytes =
        string_storage_bytes(node.extra->rare_strings->vtt_symbol);
    vtt_object_symbol_bytes =
        string_storage_bytes(node.extra->rare_strings->vtt_object_symbol);
    runtime_bridge_symbol_bytes =
        string_storage_bytes(node.extra->rare_strings->runtime_bridge_symbol);
    local_static_guard_symbol_bytes =
        string_storage_bytes(node.extra->rare_strings->local_static_guard_symbol);
  }
  if(node.source_file_index != 0 &&
     seen_callsem_source_files.insert(node.source_file_index).second) {
    source_file_bytes = string_storage_bytes(callsem_source_file(node));
  }
  const size_t symbol_bytes =
      callsem_symbol_storage_bytes(node, &seen_callsem_symbols);
  const size_t qualified_name_bytes =
      qualified_name_payload_bytes(callsem_qualified_name_syntax(node));
  size_t bytes = sizeof(CallSemNode) +
                 extra_bytes +
                 rare_string_bytes +
                 rare_payload_bytes +
                 text_bytes +
                 resolved_name_bytes +
                 source_file_bytes +
                 children_bytes +
                 virtual_base_layout_bytes +
                 vtt_symbol_bytes +
                 vtt_object_symbol_bytes +
                 runtime_bridge_symbol_bytes +
                 local_static_guard_symbol_bytes +
                 virtual_base_layout_string_bytes +
                 symbol_bytes +
                 qualified_name_bytes;
  census.note(kind, bytes);
  census.note_detail(kind + ".inline", sizeof(CallSemNode));
  census.note_detail(kind + ".extra_inline", extra_bytes);
  census.note_detail(kind + ".rare_string_inline", rare_string_bytes);
  census.note_detail(kind + ".rare_payload_inline", rare_payload_bytes);
  census.note_detail(kind + ".string_capacity.text", text_bytes);
  census.note_detail(kind + ".string_capacity.resolved_name", resolved_name_bytes);
  census.note_detail(kind + ".string_capacity.source_file", source_file_bytes);
  census.note_detail(kind + ".children_storage", children_bytes);
  census.note_detail(kind + ".virtual_base_layout_storage", virtual_base_layout_bytes);
  census.note_detail(kind + ".string_capacity.vtt_symbol", vtt_symbol_bytes);
  census.note_detail(kind + ".string_capacity.vtt_object_symbol",
                     vtt_object_symbol_bytes);
  census.note_detail(kind + ".string_capacity.runtime_bridge_symbol",
                     runtime_bridge_symbol_bytes);
  census.note_detail(kind + ".string_capacity.local_static_guard_symbol",
                     local_static_guard_symbol_bytes);
  census.note_detail(kind + ".symbol_payload", symbol_bytes);
  census.note_detail(kind + ".qualified_name_syntax", qualified_name_bytes);

  census_type(node.semantic_type, census, seen_types);
  census_type(callsem_vtt_owner_type(node), census, seen_types);
  census_type(callsem_materialization_source_type(node), census, seen_types);
  census_type(callsem_conversion_source_type(node), census, seen_types);
  census_type(callsem_initializer_list_element_type(node), census, seen_types);
  census_type(callsem_typeid_operand_type(node), census, seen_types);
  if(callsem_lowered_condition_test(node)) {
    census_callsem_node(*callsem_lowered_condition_test(node),
                        kind + ".lowered_condition_test",
                        census,
                        seen_types,
                        seen_callsem_extras,
                        seen_callsem_rare_strings,
                        seen_callsem_rare_payloads,
                        seen_callsem_source_files,
                        seen_callsem_symbols);
  }
  for(size_t i = 0; i < node.children.size(); ++i) {
    census_callsem_node(node.children[i],
                        kind,
                        census,
                        seen_types,
                        seen_callsem_extras,
                        seen_callsem_rare_strings,
                        seen_callsem_rare_payloads,
                        seen_callsem_source_files,
                        seen_callsem_symbols);
  }
}

void census_scope(const Scope * scope,
                 MemoryCensus & census,
                 unordered_set<const Scope *> & seen_scopes,
                 unordered_set<const Type *> & seen_types)
{
  if(scope == nullptr || !seen_scopes.insert(scope).second) {
    return;
  }

  const auto note_nonempty =
      [&census](const char * kind, bool nonempty)
      {
        if(nonempty) {
          census.note_detail(kind, 0);
        }
      };
  note_nonempty("scope.nonempty.named_types", !scope->named_types.empty());
  note_nonempty("scope.nonempty.named_type_access",
                !scope->named_type_access.empty());
  note_nonempty("scope.nonempty.named_type_packs",
                !scope->named_type_packs.empty());
  note_nonempty("scope.nonempty.named_value_packs",
                !scope->named_value_packs.empty());
  note_nonempty("scope.nonempty.named_pack_sizes",
                !scope->named_pack_sizes.empty());
  note_nonempty("scope.nonempty.template_bound_type_names",
                !scope->template_bound_type_names.empty());
  note_nonempty("scope.nonempty.template_bound_type_pack_names",
                !scope->template_bound_type_pack_names.empty());
  note_nonempty("scope.nonempty.template_bound_value_names",
                !scope->template_bound_value_names.empty());
  note_nonempty("scope.nonempty.template_bound_value_pack_names",
                !scope->template_bound_value_pack_names.empty());
  note_nonempty("scope.nonempty.template_bound_template_names",
                !scope->template_bound_template_names.empty());
  note_nonempty("scope.nonempty.template_bound_template_arguments",
                !scope->template_bound_template_arguments.empty());
  note_nonempty("scope.nonempty.values", !scope->values.empty());
  note_nonempty("scope.nonempty.namespace_bindings",
                !scope->namespace_bindings.empty());
  note_nonempty("scope.nonempty.function_sets",
                !scope->function_sets.empty());
  note_nonempty("scope.nonempty.function_set_access_overrides",
                !scope->function_set_access_overrides.empty());
  note_nonempty("scope.nonempty.class_templates",
                !scope->class_templates.empty());
  note_nonempty("scope.nonempty.function_templates",
                !scope->function_templates.empty());
  note_nonempty("scope.nonempty.collected_template_declarations",
                !scope->collected_template_declarations.empty());
  note_nonempty("scope.nonempty.alias_templates",
                !scope->alias_templates.empty());
  note_nonempty("scope.nonempty.variable_templates",
                !scope->variable_templates.empty());
  note_nonempty("scope.nonempty.using_directives",
                !scope->using_directives.empty());
  note_nonempty("scope.nonempty.namespace_children",
                !scope->namespace_children.empty());
  note_nonempty("scope.nonempty.cached_direct_function_lookups",
                !scope->cached_direct_function_lookups.empty());

  size_t bytes = sizeof(Scope) +
                 string_storage_bytes(scope->name) +
                 unordered_map_storage_bytes(scope->named_types) +
                 map_storage_bytes(scope->named_type_packs.get()) +
                 map_storage_bytes(scope->named_value_packs.get()) +
                 map_storage_bytes(scope->named_pack_sizes.get()) +
                 set_storage_bytes(scope->template_bound_type_names) +
                 set_storage_bytes(scope->template_bound_type_pack_names.get()) +
                 set_storage_bytes(scope->template_bound_value_names.get()) +
                 set_storage_bytes(scope->template_bound_value_pack_names.get()) +
                 set_storage_bytes(scope->template_bound_template_names.get()) +
                 map_storage_bytes(scope->values) +
                 map_storage_bytes(scope->namespace_bindings.get()) +
                 map_storage_bytes(scope->function_sets.get()) +
                 map_storage_bytes(scope->class_templates.get()) +
                 map_storage_bytes(scope->function_templates.get()) +
                 set_storage_bytes(scope->collected_template_declarations.get()) +
                 map_storage_bytes(scope->alias_templates.get()) +
                 map_storage_bytes(scope->variable_templates.get()) +
                 vector_storage_bytes(scope->using_directives) +
                 vector_storage_bytes(scope->namespace_children);
  census.note_detail("scope.base", sizeof(Scope));
  census.note_detail("scope.storage.named_types",
                     unordered_map_storage_bytes(scope->named_types));
  census.note_detail("scope.storage.named_type_packs",
                     map_storage_bytes(scope->named_type_packs.get()));
  census.note_detail("scope.storage.named_value_packs",
                     map_storage_bytes(scope->named_value_packs.get()));
  census.note_detail("scope.storage.named_pack_sizes",
                     map_storage_bytes(scope->named_pack_sizes.get()));
  census.note_detail("scope.storage.template_bound_type_names",
                     set_storage_bytes(scope->template_bound_type_names));
  census.note_detail("scope.storage.template_bound_type_pack_names",
                     set_storage_bytes(
                         scope->template_bound_type_pack_names.get()));
  census.note_detail("scope.storage.template_bound_value_names",
                     set_storage_bytes(
                         scope->template_bound_value_names.get()));
  census.note_detail("scope.storage.template_bound_value_pack_names",
                     set_storage_bytes(
                         scope->template_bound_value_pack_names.get()));
  census.note_detail("scope.storage.template_bound_template_names",
                     set_storage_bytes(
                         scope->template_bound_template_names.get()));
  census.note_detail("scope.storage.values",
                     map_storage_bytes(scope->values));
  census.note_detail("scope.storage.function_sets",
                     map_storage_bytes(scope->function_sets.get()));
  census.note_detail("scope.storage.class_templates",
                     map_storage_bytes(scope->class_templates.get()));
  census.note_detail("scope.storage.function_templates",
                     map_storage_bytes(scope->function_templates.get()));
  census.note_detail("scope.string_capacity.name",
                     string_storage_bytes(scope->name));

  size_t named_type_key_bytes = 0;
  for(auto it = scope->named_types.begin();
      it != scope->named_types.end();
      ++it) {
    const size_t key_bytes = string_storage_bytes(it->first);
    bytes += key_bytes;
    named_type_key_bytes += key_bytes;
    census_type(it->second, census, seen_types);
  }
  census.note_detail("scope.named_types.string_capacity.keys",
                     named_type_key_bytes);
  for(map<string, vector<TypePtr> >::const_iterator it = scope->named_type_packs.begin();
      it != scope->named_type_packs.end();
      ++it) {
    bytes += string_storage_bytes(it->first);
    bytes += vector_storage_bytes(it->second);
    for(size_t i = 0; i < it->second.size(); ++i) {
      census_type(it->second[i], census, seen_types);
    }
  }
  for(map<string, vector<ValueBinding> >::const_iterator it = scope->named_value_packs.begin();
      it != scope->named_value_packs.end();
      ++it) {
    bytes += string_storage_bytes(it->first);
    bytes += vector_storage_bytes(it->second);
    for(size_t i = 0; i < it->second.size(); ++i) {
      census.note("value_binding.scope_pack",
                  sizeof(ValueBinding) +
                      value_binding_payload_bytes(it->second[i], census, seen_types));
    }
  }
  for(map<string, size_t>::const_iterator it = scope->named_pack_sizes.begin();
      it != scope->named_pack_sizes.end();
      ++it) {
    bytes += string_storage_bytes(it->first);
  }
  for(set<string>::const_iterator it = scope->template_bound_type_names.begin();
      it != scope->template_bound_type_names.end();
      ++it) {
    bytes += string_storage_bytes(*it);
  }
  for(set<string>::const_iterator it = scope->template_bound_type_pack_names.begin();
      it != scope->template_bound_type_pack_names.end();
      ++it) {
    bytes += string_storage_bytes(*it);
  }
  for(set<string>::const_iterator it = scope->template_bound_value_names.begin();
      it != scope->template_bound_value_names.end();
      ++it) {
    bytes += string_storage_bytes(*it);
  }
  for(set<string>::const_iterator it = scope->template_bound_value_pack_names.begin();
      it != scope->template_bound_value_pack_names.end();
      ++it) {
    bytes += string_storage_bytes(*it);
  }
  for(set<string>::const_iterator it = scope->template_bound_template_names.begin();
      it != scope->template_bound_template_names.end();
      ++it) {
    bytes += string_storage_bytes(*it);
  }
  for(map<string, ValueBinding>::const_iterator it = scope->values.begin();
      it != scope->values.end();
      ++it) {
    bytes += string_storage_bytes(it->first);
    census.note("value_binding.scope",
                sizeof(ValueBinding) +
                    value_binding_payload_bytes(it->second, census, seen_types));
  }
  for(auto it = scope->namespace_bindings.begin();
      it != scope->namespace_bindings.end();
      ++it) {
    bytes += string_storage_bytes(it->first);
  }
  for(map<string, vector<FunctionBinding *> >::const_iterator it = scope->function_sets.begin();
      it != scope->function_sets.end();
      ++it) {
    bytes += string_storage_bytes(it->first);
    bytes += vector_storage_bytes(it->second);
  }
  for(map<string, ClassTemplateDecl *>::const_iterator it = scope->class_templates.begin();
      it != scope->class_templates.end();
      ++it) {
    bytes += string_storage_bytes(it->first);
  }
  for(map<string, vector<FunctionTemplateDecl *> >::const_iterator
          it = scope->function_templates.begin();
      it != scope->function_templates.end();
      ++it) {
    bytes += string_storage_bytes(it->first);
    bytes += vector_storage_bytes(it->second);
  }
  for(map<string, AliasTemplateDecl *>::const_iterator it = scope->alias_templates.begin();
      it != scope->alias_templates.end();
      ++it) {
    bytes += string_storage_bytes(it->first);
  }
  for(map<string, VariableTemplateDecl *>::const_iterator it = scope->variable_templates.begin();
      it != scope->variable_templates.end();
      ++it) {
    bytes += string_storage_bytes(it->first);
  }

  census.note("scope", bytes);
  for(size_t i = 0; i < scope->namespace_children.size(); ++i) {
    census_scope(scope->namespace_children[i].get(), census, seen_scopes, seen_types);
  }
}

void census_function_binding(const FunctionBinding * binding,
                             MemoryCensus & census,
                             unordered_set<const FunctionBinding *> & seen_functions,
                             unordered_set<const Type *> & seen_types,
                             unordered_set<const CallSemNodeExtra *> & seen_callsem_extras,
                             unordered_set<const CallSemRareStrings *> &
                                 seen_callsem_rare_strings,
                             unordered_set<const CallSemRarePayload *> &
                                 seen_callsem_rare_payloads,
                             unordered_set<uint32_t> & seen_callsem_source_files,
                             unordered_set<const symbol_linkage::SymbolIdentity *> &
                                 seen_callsem_symbols)
{
  if(binding == nullptr || !seen_functions.insert(binding).second) {
    return;
  }

  size_t bytes = sizeof(FunctionBinding) +
                 string_storage_bytes(binding->name) +
                 string_storage_bytes(binding->display_name) +
                 string_storage_bytes(binding->template_instantiation_key) +
                 vector_storage_bytes(binding->params) +
                 vector_storage_bytes(binding->parameter_aliases) +
                 vector_storage_bytes(binding->default_arguments) +
                 vector_storage_bytes(binding->instantiation_arguments) +
                 map_storage_bytes(binding->instantiation_pack_sizes) +
                 symbol_identity_payload_bytes(binding->symbol);
  census.note_detail("function_binding.string_capacity.name",
                     string_storage_bytes(binding->name));
  census.note_detail("function_binding.string_capacity.display_name",
                     string_storage_bytes(binding->display_name));
  census.note_detail("function_binding.string_capacity.instantiation_key",
                     string_storage_bytes(binding->template_instantiation_key));
  for(size_t i = 0; i < binding->params.size(); ++i) {
    bytes += string_storage_bytes(binding->params[i].first);
    census_type(binding->params[i].second, census, seen_types);
  }
  for(size_t i = 0; i < binding->parameter_aliases.size(); ++i) {
    bytes += string_storage_bytes(binding->parameter_aliases[i]);
  }
  for(size_t i = 0; i < binding->instantiation_arguments.size(); ++i) {
    bytes += template_argument_payload_bytes(binding->instantiation_arguments[i],
                                             census,
                                             seen_types,
                                             "function_binding.instantiation_argument");
  }
  for(map<string, size_t>::const_iterator it = binding->instantiation_pack_sizes.begin();
      it != binding->instantiation_pack_sizes.end();
      ++it) {
    bytes += string_storage_bytes(it->first);
  }
  census.note("function_binding", bytes);

  census_type(binding->declared_type, census, seen_types);
  census_type(binding->type, census, seen_types);
  if(binding->cached_body_output) {
    census_callsem_node(*binding->cached_body_output,
                        "callsem.cached_body",
                        census,
                        seen_types,
                        seen_callsem_extras,
                        seen_callsem_rare_strings,
                        seen_callsem_rare_payloads,
                        seen_callsem_source_files,
                        seen_callsem_symbols);
  }
}

void census_class_info(const ClassInfo * info,
                       MemoryCensus & census,
                       unordered_set<const ClassInfo *> & seen_classes,
                       unordered_set<const Scope *> & seen_scopes,
                       unordered_set<const FunctionBinding *> & seen_functions,
                       unordered_set<const Type *> & seen_types,
                       unordered_set<const CallSemNodeExtra *> & seen_callsem_extras,
                       unordered_set<const CallSemRareStrings *> &
                           seen_callsem_rare_strings,
                       unordered_set<const CallSemRarePayload *> &
                           seen_callsem_rare_payloads,
                       unordered_set<uint32_t> & seen_callsem_source_files,
                       unordered_set<const symbol_linkage::SymbolIdentity *> &
                           seen_callsem_symbols)
{
  if(info == nullptr || !seen_classes.insert(info).second) {
    return;
  }

  size_t reference_named_member_index_bytes = 0;
  if(info->reference_named_member_index) {
    const ClassInfo::ReferenceNamedMemberIndex & index =
        *info->reference_named_member_index;
    reference_named_member_index_bytes =
        sizeof(ClassInfo::ReferenceNamedMemberIndex) +
        unordered_map_storage_bytes(index.by_name);
    for(auto it = index.by_name.begin(); it != index.by_name.end(); ++it) {
      reference_named_member_index_bytes +=
          string_storage_bytes(it->first) +
          vector_storage_bytes(it->second);
    }
  }

  size_t bytes = sizeof(ClassInfo) +
                 string_storage_bytes(info->name) +
                 string_storage_bytes(info->qualified_name) +
                 qualified_name_payload_bytes(
                     &info->symbol_qualified_name_syntax) +
                 string_storage_bytes(info->display_qualified_name) +
                 string_storage_bytes(info->class_kind) +
                 string_storage_bytes(info->creation_context) +
                 string_storage_bytes(info->instantiation_key) +
                 vector_storage_bytes(info->fields) +
                 map_storage_bytes(info->methods) +
                 vector_storage_bytes(info->method_declaration_order) +
                 vector_storage_bytes(info->bases) +
                 vector_storage_bytes(info->vtable_entries) +
                 vector_storage_bytes(info->vtable_entry_contracts) +
                 vector_storage_bytes(info->complete_subobjects) +
                 vector_storage_bytes(info->virtual_base_subobjects) +
                 vector_storage_bytes(info->vtables) +
                 vector_storage_bytes(info->friend_functions) +
                 vector_storage_bytes(info->friend_access_functions) +
                 vector_storage_bytes(info->friend_function_templates) +
                 vector_storage_bytes(info->friend_access_function_templates) +
                 vector_storage_bytes(info->friend_class_names) +
                 vector_storage_bytes(info->instantiation_arg_texts) +
                 vector_storage_bytes(info->instantiation_arguments) +
                 vector_storage_bytes(info->instantiation_binding_arguments) +
                 map_storage_bytes(info->instantiation_binding_pack_sizes) +
                 reference_named_member_index_bytes;
  census.note_detail("class_info.string_capacity.name",
                     string_storage_bytes(info->name));
  census.note_detail("class_info.string_capacity.qualified_name",
                     string_storage_bytes(info->qualified_name));
  census.note_detail("class_info.symbol_qualified_name_syntax",
                     qualified_name_payload_bytes(
                         &info->symbol_qualified_name_syntax));
  census.note_detail("class_info.string_capacity.display_qualified_name",
                     string_storage_bytes(info->display_qualified_name));
  census.note_detail("class_info.string_capacity.creation_context",
                     string_storage_bytes(info->creation_context));
  census.note_detail("class_info.string_capacity.instantiation_key",
                     string_storage_bytes(info->instantiation_key));
  census.note_detail("class_info.reference_named_member_index",
                     reference_named_member_index_bytes);
  size_t instantiation_arg_text_bytes = 0;
  for(size_t i = 0; i < info->fields.size(); ++i) {
    bytes += string_storage_bytes(info->fields[i].name);
    census_type(info->fields[i].type, census, seen_types);
  }
  for(map<string, vector<FunctionBinding *> >::const_iterator it = info->methods.begin();
      it != info->methods.end();
      ++it) {
    bytes += string_storage_bytes(it->first);
    bytes += vector_storage_bytes(it->second);
  }
  for(size_t i = 0; i < info->vtables.size(); ++i) {
    bytes += string_storage_bytes(info->vtables[i].key);
    bytes += vector_storage_bytes(info->vtables[i].slots);
  }
  for(size_t i = 0; i < info->friend_class_names.size(); ++i) {
    bytes += string_storage_bytes(info->friend_class_names[i]);
  }
  for(size_t i = 0; i < info->instantiation_arg_texts.size(); ++i) {
    const size_t text_bytes =
        string_storage_bytes(info->instantiation_arg_texts[i]);
    bytes += text_bytes;
    instantiation_arg_text_bytes += text_bytes;
  }
  census.note_detail("class_info.instantiation_arg_text_capacity",
                     instantiation_arg_text_bytes);
  for(size_t i = 0; i < info->instantiation_arguments.size(); ++i) {
    bytes += template_argument_payload_bytes(info->instantiation_arguments[i],
                                             census,
                                             seen_types,
                                             "class_info.instantiation_argument");
  }
  size_t binding_argument_bytes =
      vector_storage_bytes(info->instantiation_binding_arguments);
  for(size_t i = 0;
      i < info->instantiation_binding_arguments.size();
      ++i) {
    binding_argument_bytes += template_argument_payload_bytes(
        info->instantiation_binding_arguments[i],
        census,
        seen_types,
        "class_info.instantiation_binding_argument");
  }
  for(map<string, size_t>::const_iterator
          it = info->instantiation_binding_pack_sizes.begin();
      it != info->instantiation_binding_pack_sizes.end();
      ++it) {
    binding_argument_bytes += string_storage_bytes(it->first);
  }
  bytes += binding_argument_bytes -
           vector_storage_bytes(info->instantiation_binding_arguments);
  census.note_detail("class_info.instantiation_binding_arguments",
                     binding_argument_bytes);
  census.note("class_info", bytes);

  census_type(info->type, census, seen_types);
  census_type(info->initializer_list_element_type, census, seen_types);
  if(info->member_scope) {
    census_scope(info->member_scope.get(), census, seen_scopes, seen_types);
  }
  for(size_t i = 0; i < info->method_declaration_order.size(); ++i) {
    census_function_binding(info->method_declaration_order[i],
                            census,
                            seen_functions,
                            seen_types,
                            seen_callsem_extras,
                            seen_callsem_rare_strings,
                            seen_callsem_rare_payloads,
                            seen_callsem_source_files,
                            seen_callsem_symbols);
  }
  for(size_t i = 0; i < info->friend_functions.size(); ++i) {
    census_function_binding(info->friend_functions[i],
                            census,
                            seen_functions,
                            seen_types,
                            seen_callsem_extras,
                            seen_callsem_rare_strings,
                            seen_callsem_rare_payloads,
                            seen_callsem_source_files,
                            seen_callsem_symbols);
  }
  for(size_t i = 0; i < info->friend_access_functions.size(); ++i) {
    census_function_binding(info->friend_access_functions[i],
                            census,
                            seen_functions,
                            seen_types,
                            seen_callsem_extras,
                            seen_callsem_rare_strings,
                            seen_callsem_rare_payloads,
                            seen_callsem_source_files,
                            seen_callsem_symbols);
  }
}

void census_function_template(const FunctionTemplateDecl & decl,
                              MemoryCensus & census,
                              unordered_set<const Scope *> & seen_scopes,
                              unordered_set<const Type *> & seen_types)
{
  size_t bytes = sizeof(FunctionTemplateDecl) +
                 string_storage_bytes(decl.name) +
                 string_storage_bytes(decl.debug_decl_location) +
                 string_storage_bytes(decl.debug_decl_location_details) +
                 string_storage_bytes(decl.debug_scope_name) +
                 string_storage_bytes(decl.debug_signature) +
                 vector_storage_bytes(decl.parameters) +
                 vector_storage_bytes(decl.definition_owner_parameters) +
                 vector_storage_bytes(decl.params_pattern) +
                 vector_storage_bytes(decl.parameter_aliases_pattern) +
                 vector_storage_bytes(decl.default_arguments_pattern) +
                 vector_storage_bytes(decl.friend_access_classes) +
                 map_storage_bytes(decl.instantiations);
  for(size_t i = 0; i < decl.parameters.size(); ++i) {
    bytes += template_parameter_payload_bytes(decl.parameters[i], census, seen_types);
  }
  for(size_t i = 0; i < decl.definition_owner_parameters.size(); ++i) {
    bytes += template_parameter_payload_bytes(decl.definition_owner_parameters[i],
                                              census,
                                              seen_types);
  }
  for(size_t i = 0; i < decl.params_pattern.size(); ++i) {
    bytes += string_storage_bytes(decl.params_pattern[i].first);
    census_type(decl.params_pattern[i].second, census, seen_types);
  }
  for(size_t i = 0; i < decl.parameter_aliases_pattern.size(); ++i) {
    bytes += string_storage_bytes(decl.parameter_aliases_pattern[i]);
  }
  size_t instantiation_key_bytes = 0;
  for(map<string, FunctionBinding *>::const_iterator it = decl.instantiations.begin();
      it != decl.instantiations.end();
      ++it) {
    const size_t key_bytes = string_storage_bytes(it->first);
    bytes += key_bytes;
    instantiation_key_bytes += key_bytes;
  }
  census.note_detail("function_template.instantiation_key_capacity",
                     instantiation_key_bytes);
  census.note("function_template", bytes);

  if(active_retained_ast_memory_census &&
     decl.result_type_pattern.kind != CppAstKind::invalid) {
    active_retained_ast_memory_census->add_inline_ast(
        decl.result_type_pattern,
        "function_template.result_type_pattern");
  }
  census_type(decl.type_pattern, census, seen_types);
  census_scope(decl.pattern_scope, census, seen_scopes, seen_types);
}

void census_alias_template(const AliasTemplateDecl & decl,
                           MemoryCensus & census,
                           unordered_set<const Scope *> & seen_scopes,
                           unordered_set<const Type *> & seen_types)
{
  size_t bytes = sizeof(AliasTemplateDecl) +
                 string_storage_bytes(decl.name) +
                 vector_storage_bytes(decl.parameters) +
                 map_storage_bytes(decl.instantiations) +
                 map_storage_bytes(decl.reference_instantiations);
  for(size_t i = 0; i < decl.parameters.size(); ++i) {
    bytes += template_parameter_payload_bytes(decl.parameters[i], census, seen_types);
  }
  for(auto it = decl.instantiations.begin();
      it != decl.instantiations.end();
      ++it) {
    bytes += string_storage_bytes(it->first);
    census_type(it->second, census, seen_types);
  }
  for(auto it = decl.reference_instantiations.begin();
      it != decl.reference_instantiations.end();
      ++it) {
    bytes += string_storage_bytes(it->first);
    census_type(it->second, census, seen_types);
  }
  census.note("alias_template", bytes);
  census_type(decl.resolved_type_pattern, census, seen_types);
  census_scope(decl.pattern_scope, census, seen_scopes, seen_types);
}

size_t partial_class_template_specialization_payload_bytes(
    const PartialClassTemplateSpecializationDecl & decl,
    MemoryCensus & census,
    unordered_set<const Type *> & seen_types)
{
  size_t bytes = vector_storage_bytes(decl.parameters) +
                 vector_storage_bytes(decl.arg_texts) +
                 vector_storage_bytes(decl.arg_syntaxes) +
                 map_storage_bytes(decl.static_member_definitions) +
                 map_storage_bytes(decl.member_class_definitions) +
                 map_storage_bytes(decl.member_function_definitions) +
                 map_storage_bytes(decl.member_function_template_definitions);
  for(size_t i = 0; i < decl.parameters.size(); ++i) {
    bytes += template_parameter_payload_bytes(decl.parameters[i],
                                             census,
                                             seen_types);
  }
  for(size_t i = 0; i < decl.arg_texts.size(); ++i) {
    bytes += string_storage_bytes(decl.arg_texts[i]);
  }
  for(map<string, OutOfClassStaticMemberDecl>::const_iterator
          it = decl.static_member_definitions.begin();
      it != decl.static_member_definitions.end();
      ++it) {
    bytes += string_storage_bytes(it->first);
  }
  for(map<string, OutOfClassMemberClassDecl>::const_iterator
          it = decl.member_class_definitions.begin();
      it != decl.member_class_definitions.end();
      ++it) {
    bytes += string_storage_bytes(it->first);
  }
  for(map<string, vector<OutOfClassMemberFunctionDecl> >::const_iterator
          defs = decl.member_function_definitions.begin();
      defs != decl.member_function_definitions.end();
      ++defs) {
    bytes += string_storage_bytes(defs->first);
    bytes += vector_storage_bytes(defs->second);
  }
  for(map<string, vector<OutOfClassMemberFunctionTemplateDefinition> >::const_iterator
          defs = decl.member_function_template_definitions.begin();
      defs != decl.member_function_template_definitions.end();
      ++defs) {
    bytes += string_storage_bytes(defs->first);
    bytes += vector_storage_bytes(defs->second);
  }
  return bytes;
}

void census_class_template(const ClassTemplateDecl & decl,
                           MemoryCensus & census,
                           unordered_set<const Type *> & seen_types)
{
  size_t bytes = sizeof(ClassTemplateDecl) +
                 string_storage_bytes(decl.name) +
                 vector_storage_bytes(decl.parameters) +
                 map_storage_bytes(decl.instantiations) +
                 map_storage_bytes(decl.reference_instantiations) +
                 map_storage_bytes(decl.fast_reference_cache) +
                 set_storage_bytes(decl.suppress_implicit_instantiation_definitions) +
                 set_storage_bytes(
                     decl.suppress_implicit_member_function_instantiation_definitions) +
                 map_storage_bytes(decl.explicit_specializations) +
                 vector_storage_bytes(decl.partial_specializations) +
                 vector_storage_bytes(decl.deduction_guides) +
                 map_storage_bytes(decl.static_member_definitions) +
                 map_storage_bytes(decl.member_class_definitions) +
                 map_storage_bytes(decl.member_class_template_partial_specializations) +
                 map_storage_bytes(decl.member_function_definitions) +
                 map_storage_bytes(decl.member_function_template_definitions);
  for(size_t i = 0; i < decl.parameters.size(); ++i) {
    bytes += template_parameter_payload_bytes(decl.parameters[i], census, seen_types);
  }
  size_t instantiation_key_bytes = 0;
  for(auto it = decl.instantiations.begin();
      it != decl.instantiations.end();
      ++it) {
    const size_t key_bytes = string_storage_bytes(it->first);
    bytes += key_bytes;
    instantiation_key_bytes += key_bytes;
  }
  census.note_detail("class_template.instantiation_key_capacity",
                     instantiation_key_bytes);
  size_t reference_instantiation_key_bytes = 0;
  for(auto it = decl.reference_instantiations.begin();
      it != decl.reference_instantiations.end();
      ++it) {
    const size_t key_bytes = string_storage_bytes(it->first);
    bytes += key_bytes;
    reference_instantiation_key_bytes += key_bytes;
  }
  census.note_detail("class_template.reference_instantiation_key_capacity",
                     reference_instantiation_key_bytes);
  size_t fast_reference_key_bytes = 0;
  for(auto it = decl.fast_reference_cache.begin();
      it != decl.fast_reference_cache.end();
      ++it) {
    const size_t key_bytes = string_storage_bytes(it->first);
    bytes += key_bytes;
    fast_reference_key_bytes += key_bytes;
  }
  census.note_detail("class_template.fast_reference_key_capacity",
                     fast_reference_key_bytes);
  for(set<string>::const_iterator it = decl.suppress_implicit_instantiation_definitions.begin();
      it != decl.suppress_implicit_instantiation_definitions.end();
      ++it) {
    bytes += string_storage_bytes(*it);
  }
  for(set<pair<string, string> >::const_iterator it =
          decl.suppress_implicit_member_function_instantiation_definitions.begin();
      it != decl.suppress_implicit_member_function_instantiation_definitions.end();
      ++it) {
    bytes += string_storage_bytes(it->first);
    bytes += string_storage_bytes(it->second);
  }
  for(map<string, ClassTemplateSpecializationDecl>::const_iterator
          it = decl.explicit_specializations.begin();
      it != decl.explicit_specializations.end();
      ++it) {
    bytes += string_storage_bytes(it->first);
  }
  for(map<string, OutOfClassStaticMemberDecl>::const_iterator
          it = decl.static_member_definitions.begin();
      it != decl.static_member_definitions.end();
      ++it) {
    bytes += string_storage_bytes(it->first);
  }
  for(map<string, OutOfClassMemberClassDecl>::const_iterator
          it = decl.member_class_definitions.begin();
      it != decl.member_class_definitions.end();
      ++it) {
    bytes += string_storage_bytes(it->first);
  }
  for(map<string, vector<PartialClassTemplateSpecializationDecl> >::const_iterator
          it = decl.member_class_template_partial_specializations.begin();
      it != decl.member_class_template_partial_specializations.end();
      ++it) {
    bytes += string_storage_bytes(it->first);
    bytes += vector_storage_bytes(it->second);
    for(vector<PartialClassTemplateSpecializationDecl>::const_iterator
            partial = it->second.begin();
        partial != it->second.end();
        ++partial) {
      bytes += partial_class_template_specialization_payload_bytes(
          *partial,
          census,
          seen_types);
    }
  }
  for(map<string, vector<OutOfClassMemberFunctionDecl> >::const_iterator
          it = decl.member_function_definitions.begin();
      it != decl.member_function_definitions.end();
      ++it) {
    bytes += string_storage_bytes(it->first);
    bytes += vector_storage_bytes(it->second);
  }
  for(map<string, vector<OutOfClassMemberFunctionTemplateDefinition> >::const_iterator
          it = decl.member_function_template_definitions.begin();
      it != decl.member_function_template_definitions.end();
      ++it) {
    bytes += string_storage_bytes(it->first);
    bytes += vector_storage_bytes(it->second);
  }
  for(vector<PartialClassTemplateSpecializationDecl>::const_iterator
          it = decl.partial_specializations.begin();
      it != decl.partial_specializations.end();
      ++it) {
    bytes += partial_class_template_specialization_payload_bytes(*it,
                                                                 census,
                                                                 seen_types);
  }
  census.note("class_template", bytes);
}

void census_variable_template(const VariableTemplateDecl & decl,
                              MemoryCensus & census,
                              unordered_set<const Scope *> & seen_scopes,
                              unordered_set<const Type *> & seen_types)
{
  size_t bytes = sizeof(VariableTemplateDecl) +
                 string_storage_bytes(decl.name) +
                 vector_storage_bytes(decl.parameters) +
                 map_storage_bytes(decl.instantiations) +
                 map_storage_bytes(decl.explicit_specializations) +
                 vector_storage_bytes(decl.partial_specializations);
  for(size_t i = 0; i < decl.parameters.size(); ++i) {
    bytes += template_parameter_payload_bytes(decl.parameters[i], census, seen_types);
  }
  for(map<string, ValueBinding>::const_iterator it = decl.instantiations.begin();
      it != decl.instantiations.end();
      ++it) {
    bytes += string_storage_bytes(it->first);
    census.note("value_binding.variable_template",
                sizeof(ValueBinding) +
                    value_binding_payload_bytes(it->second, census, seen_types));
  }
  for(map<string, VariableTemplateSpecializationDecl>::const_iterator
          it = decl.explicit_specializations.begin();
      it != decl.explicit_specializations.end();
      ++it) {
    bytes += string_storage_bytes(it->first);
  }
  census.note("variable_template", bytes);
  census_type(decl.type_pattern, census, seen_types);
  census_scope(decl.pattern_scope, census, seen_scopes, seen_types);
}

void census_semantic_cache(const semantic_cache::SemanticCache & cache,
                           MemoryCensus & census,
                           unordered_set<const Type *> & seen_types)
{
  size_t bytes = unordered_map_storage_bytes(cache.captured_local_scope_cache);
  census.note("cache.captured_local_scope", bytes, cache.captured_local_scope_cache.size());

  bytes = unordered_set_storage_bytes(cache.interned_text_pool);
  for(unordered_set<string>::const_iterator it = cache.interned_text_pool.begin();
      it != cache.interned_text_pool.end();
      ++it) {
    bytes += string_storage_bytes(*it);
  }
  census.note("cache.interned_text_pool", bytes, cache.interned_text_pool.size());

  census.note("cache.global_text_atoms",
              callsemantic_internal::interned_text_atom_storage_bytes(),
              callsemantic_internal::interned_text_atom_count());

  bytes = unordered_map_storage_bytes(cache.identifier_token_cache);
  for(unordered_map<semantic_cache::InternedTextPtr, IdentifierTokenSet>::const_iterator
          it = cache.identifier_token_cache.begin();
      it != cache.identifier_token_cache.end();
      ++it) {
    bytes += it->second.dynamic_storage_bytes();
    bytes += vector_storage_bytes(it->second.owned_names);
    for(size_t i = 0; i < it->second.owned_names.size(); ++i) {
      bytes += sizeof(string) +
               string_storage_bytes(*it->second.owned_names[i]);
    }
  }
  census.note("cache.identifier_tokens", bytes, cache.identifier_token_cache.size());

  bytes = unordered_map_storage_bytes(cache.template_placeholder_mentions_cache);
  for(unordered_map<semantic_cache::ScopeTextKey,
                    semantic_cache::TextMentionCacheState,
                    semantic_cache::ScopeTextKeyHash>::const_iterator
          it = cache.template_placeholder_mentions_cache.begin();
      it != cache.template_placeholder_mentions_cache.end();
      ++it) {}
  census.note("cache.template_placeholder_mentions",
              bytes,
              cache.template_placeholder_mentions_cache.size());

  bytes = unordered_map_storage_bytes(cache.non_namespace_binding_mentions_cache);
  for(unordered_map<semantic_cache::ScopeTextKey,
                    bool,
                    semantic_cache::ScopeTextKeyHash>::const_iterator
          it = cache.non_namespace_binding_mentions_cache.begin();
      it != cache.non_namespace_binding_mentions_cache.end();
      ++it) {}
  census.note("cache.non_namespace_binding_mentions",
              bytes,
              cache.non_namespace_binding_mentions_cache.size());

  bytes = unordered_map_storage_bytes(cache.dependent_non_namespace_binding_mentions_cache);
  for(unordered_map<semantic_cache::ScopeTextKey,
                    bool,
                    semantic_cache::ScopeTextKeyHash>::const_iterator
          it = cache.dependent_non_namespace_binding_mentions_cache.begin();
      it != cache.dependent_non_namespace_binding_mentions_cache.end();
      ++it) {}
  census.note("cache.dependent_non_namespace_binding_mentions",
              bytes,
              cache.dependent_non_namespace_binding_mentions_cache.size());

  bytes = unordered_map_storage_bytes(cache.qualified_type_lookup_cache);
  for(unordered_map<semantic_cache::QualifiedTypeLookupKey,
                    TypePtr,
                    semantic_cache::QualifiedTypeLookupKeyHash>::const_iterator
          it = cache.qualified_type_lookup_cache.begin();
      it != cache.qualified_type_lookup_cache.end();
      ++it) {
    census_type(it->second, census, seen_types);
  }
  census.note("cache.qualified_type_lookup", bytes, cache.qualified_type_lookup_cache.size());
}

uint64_t hash_mix(uint64_t seed, uint64_t value)
{
  seed ^= value + 0x9e3779b97f4a7c15ULL + (seed << 6) + (seed >> 2);
  return seed;
}

uint64_t hash_string_value(const string & value)
{
  uint64_t hash = 1469598103934665603ULL;
  for(size_t i = 0; i < value.size(); ++i) {
    hash ^= static_cast<unsigned char>(value[i]);
    hash *= 1099511628211ULL;
  }
  return hash;
}

template<class T>
uint64_t hash_integral_value(T value)
{
  return static_cast<uint64_t>(value);
}

uint64_t hash_type_ptr_value(const TypePtr & type)
{
  return static_cast<uint64_t>(
      reinterpret_cast<uintptr_t>(type.get()));
}

bool qualified_name_equal(const shared_ptr<QualifiedName> & lhs,
                          const shared_ptr<QualifiedName> & rhs)
{
  if(!lhs || !rhs) {
    return !lhs && !rhs;
  }
  return lhs->rooted == rhs->rooted &&
         lhs->qualifiers == rhs->qualifiers &&
         lhs->name == rhs->name;
}

uint64_t hash_qualified_name(const shared_ptr<QualifiedName> & qualified)
{
  uint64_t hash = 0x9368e53c2f6af274ULL;
  hash = hash_mix(hash, qualified ? 1 : 0);
  if(!qualified) {
    return hash;
  }
  hash = hash_mix(hash, qualified->rooted ? 1 : 0);
  hash = hash_mix(hash, qualified->qualifiers.size());
  for(size_t i = 0; i < qualified->qualifiers.size(); ++i) {
    hash = hash_mix(hash, hash_string_value(qualified->qualifiers[i]));
  }
  hash = hash_mix(hash, hash_string_value(qualified->name));
  return hash;
}

bool symbol_identity_equal(const symbol_linkage::SymbolIdentity & lhs,
                           const symbol_linkage::SymbolIdentity & rhs)
{
  return lhs.internal_symbol == rhs.internal_symbol &&
         lhs.object_symbol == rhs.object_symbol &&
         lhs.thread_local_wrapper_object_symbol ==
             rhs.thread_local_wrapper_object_symbol &&
         lhs.keep_internal_alias == rhs.keep_internal_alias &&
         lhs.prefer_local_object_binding == rhs.prefer_local_object_binding &&
         lhs.linkage == rhs.linkage;
}

uint64_t hash_symbol_identity(const symbol_linkage::SymbolIdentity & symbol)
{
  uint64_t hash = 0x2f98b7e9c587a123ULL;
  hash = hash_mix(hash, hash_string_value(symbol.internal_symbol));
  hash = hash_mix(hash, hash_string_value(symbol.object_symbol));
  hash = hash_mix(hash, hash_string_value(symbol.thread_local_wrapper_object_symbol));
  hash = hash_mix(hash, symbol.keep_internal_alias ? 1 : 0);
  hash = hash_mix(hash, symbol.prefer_local_object_binding ? 1 : 0);
  hash = hash_mix(hash, static_cast<uint64_t>(symbol.linkage));
  return hash;
}

bool callsem_flags_equal(const CallSemNode & lhs, const CallSemNode & rhs)
{
  return lhs.implicit_return_move_eligible == rhs.implicit_return_move_eligible &&
         lhs.has_uint_value == rhs.has_uint_value &&
         lhs.has_int_value == rhs.has_int_value &&
         lhs.has_result_adjust == rhs.has_result_adjust &&
         lhs.has_virtual_result_adjust == rhs.has_virtual_result_adjust &&
         lhs.is_bit_field == rhs.is_bit_field &&
         lhs.is_reference_storage == rhs.is_reference_storage &&
         lhs.is_reference_storage_target == rhs.is_reference_storage_target &&
         lhs.is_base_subobject == rhs.is_base_subobject &&
         lhs.is_public_access == rhs.is_public_access &&
         lhs.is_virtual_base_subobject == rhs.is_virtual_base_subobject &&
         lhs.has_token == rhs.has_token &&
         lhs.is_virtual_dispatch == rhs.is_virtual_dispatch &&
         lhs.is_virtual_member_function == rhs.is_virtual_member_function &&
         lhs.is_constructor == rhs.is_constructor &&
         lhs.is_delegating_constructor == rhs.is_delegating_constructor &&
         lhs.is_destructor == rhs.is_destructor &&
         lhs.is_const_method == rhs.is_const_method &&
         lhs.is_volatile_method == rhs.is_volatile_method &&
         lhs.has_function_ref_qualifier == rhs.has_function_ref_qualifier &&
         lhs.has_virtual_dispatch_view_offset ==
             rhs.has_virtual_dispatch_view_offset &&
         lhs.is_primary_vtable == rhs.is_primary_vtable &&
         lhs.uses_extended_vtable_layout == rhs.uses_extended_vtable_layout &&
         lhs.is_extern_declaration == rhs.is_extern_declaration &&
         lhs.is_static_storage == rhs.is_static_storage &&
         lhs.is_c_linkage == rhs.is_c_linkage &&
         lhs.is_thread_local == rhs.is_thread_local &&
         lhs.is_inline_namespace == rhs.is_inline_namespace &&
         lhs.is_declval_callee == rhs.is_declval_callee &&
         lhs.is_explicit_nothrow == rhs.is_explicit_nothrow &&
         lhs.is_semantically_nothrow == rhs.is_semantically_nothrow &&
         lhs.is_force_inline == rhs.is_force_inline &&
         lhs.is_object_output_root == rhs.is_object_output_root &&
         lhs.has_dynamic_exception_spec == rhs.has_dynamic_exception_spec &&
         lhs.needs_noexcept_terminate == rhs.needs_noexcept_terminate &&
         lhs.trivial_lifecycle == rhs.trivial_lifecycle &&
         lhs.value_initializes_result == rhs.value_initializes_result &&
         lhs.object_trivial_lifecycle == rhs.object_trivial_lifecycle &&
         lhs.has_trivial_storage_copy_prefix == rhs.has_trivial_storage_copy_prefix &&
         lhs.has_special_member_entry_point_kind == rhs.has_special_member_entry_point_kind &&
         lhs.uses_vtt_parameter == rhs.uses_vtt_parameter &&
         lhs.has_vtt_slice_offset == rhs.has_vtt_slice_offset &&
         lhs.has_vtt_entry_index == rhs.has_vtt_entry_index &&
         lhs.is_destructor_body_scope == rhs.is_destructor_body_scope;
}

uint64_t hash_callsem_flags(const CallSemNode & node)
{
  uint64_t hash = 0x56d1bc39b38f984dULL;
  hash = hash_mix(hash, node.implicit_return_move_eligible ? 1 : 0);
  hash = hash_mix(hash, node.has_uint_value ? 1 : 0);
  hash = hash_mix(hash, node.has_int_value ? 1 : 0);
  hash = hash_mix(hash, node.has_result_adjust ? 1 : 0);
  hash = hash_mix(hash, node.has_virtual_result_adjust ? 1 : 0);
  hash = hash_mix(hash, node.is_bit_field ? 1 : 0);
  hash = hash_mix(hash, node.is_reference_storage ? 1 : 0);
  hash = hash_mix(hash, node.is_reference_storage_target ? 1 : 0);
  hash = hash_mix(hash, node.is_base_subobject ? 1 : 0);
  hash = hash_mix(hash, node.is_public_access ? 1 : 0);
  hash = hash_mix(hash, node.is_virtual_base_subobject ? 1 : 0);
  hash = hash_mix(hash, node.has_token ? 1 : 0);
  hash = hash_mix(hash, node.is_virtual_dispatch ? 1 : 0);
  hash = hash_mix(hash, node.is_virtual_member_function ? 1 : 0);
  hash = hash_mix(hash, node.is_constructor ? 1 : 0);
  hash = hash_mix(hash, node.is_delegating_constructor ? 1 : 0);
  hash = hash_mix(hash, node.is_destructor ? 1 : 0);
  hash = hash_mix(hash, node.is_const_method ? 1 : 0);
  hash = hash_mix(hash, node.is_volatile_method ? 1 : 0);
  hash = hash_mix(hash, node.has_function_ref_qualifier ? 1 : 0);
  hash = hash_mix(hash, node.has_virtual_dispatch_view_offset ? 1 : 0);
  hash = hash_mix(hash, node.is_primary_vtable ? 1 : 0);
  hash = hash_mix(hash, node.uses_extended_vtable_layout ? 1 : 0);
  hash = hash_mix(hash, node.is_extern_declaration ? 1 : 0);
  hash = hash_mix(hash, node.is_static_storage ? 1 : 0);
  hash = hash_mix(hash, node.is_c_linkage ? 1 : 0);
  hash = hash_mix(hash, node.is_thread_local ? 1 : 0);
  hash = hash_mix(hash, node.is_inline_namespace ? 1 : 0);
  hash = hash_mix(hash, node.is_declval_callee ? 1 : 0);
  hash = hash_mix(hash, node.is_explicit_nothrow ? 1 : 0);
  hash = hash_mix(hash, node.is_semantically_nothrow ? 1 : 0);
  hash = hash_mix(hash, node.is_force_inline ? 1 : 0);
  hash = hash_mix(hash, node.is_object_output_root ? 1 : 0);
  hash = hash_mix(hash, node.has_dynamic_exception_spec ? 1 : 0);
  hash = hash_mix(hash, node.needs_noexcept_terminate ? 1 : 0);
  hash = hash_mix(hash, node.trivial_lifecycle ? 1 : 0);
  hash = hash_mix(hash, node.value_initializes_result ? 1 : 0);
  hash = hash_mix(hash, node.object_trivial_lifecycle ? 1 : 0);
  hash = hash_mix(hash, node.has_trivial_storage_copy_prefix ? 1 : 0);
  hash = hash_mix(hash, node.has_special_member_entry_point_kind ? 1 : 0);
  hash = hash_mix(hash, node.uses_vtt_parameter ? 1 : 0);
  hash = hash_mix(hash, node.has_vtt_slice_offset ? 1 : 0);
  hash = hash_mix(hash, node.has_vtt_entry_index ? 1 : 0);
  hash = hash_mix(hash, node.is_destructor_body_scope ? 1 : 0);
  return hash;
}

bool callsem_shallow_exact_equal(const CallSemNode & lhs, const CallSemNode & rhs)
{
  return lhs.kind == rhs.kind &&
         lhs.value_category == rhs.value_category &&
         lhs.token_type == rhs.token_type &&
         callsem_special_member_entry_point_kind(lhs) ==
             callsem_special_member_entry_point_kind(rhs) &&
         callsem_function_ref_qualifier(lhs) ==
             callsem_function_ref_qualifier(rhs) &&
         lhs.text == rhs.text &&
         callsem_resolved_name(lhs) == callsem_resolved_name(rhs) &&
         qualified_name_equal(callsem_qualified_name_syntax(lhs),
                              callsem_qualified_name_syntax(rhs)) &&
         lhs.semantic_type.get() == rhs.semantic_type.get() &&
         callsem_vtt_owner_type(lhs).get() == callsem_vtt_owner_type(rhs).get() &&
         callsem_materialization_source_type(lhs).get() ==
             callsem_materialization_source_type(rhs).get() &&
         callsem_conversion_source_type(lhs).get() ==
             callsem_conversion_source_type(rhs).get() &&
         callsem_initializer_list_element_type(lhs).get() ==
             callsem_initializer_list_element_type(rhs).get() &&
         callsem_typeid_operand_type(lhs).get() ==
             callsem_typeid_operand_type(rhs).get() &&
         callsem_virtual_base_layout(lhs) == callsem_virtual_base_layout(rhs) &&
         symbol_identity_equal(callsem_symbol(lhs), callsem_symbol(rhs)) &&
         callsem_uint_value(lhs) == callsem_uint_value(rhs) &&
         callsem_int_value(lhs) == callsem_int_value(rhs) &&
         callsem_result_adjust(lhs) == callsem_result_adjust(rhs) &&
         callsem_result_vcall_offset(lhs) ==
             callsem_result_vcall_offset(rhs) &&
         callsem_result_virtual_base_index(lhs) ==
             callsem_result_virtual_base_index(rhs) &&
         callsem_host_vcall_offset_count(lhs) ==
             callsem_host_vcall_offset_count(rhs) &&
         callsem_virtual_dispatch_view_offset(lhs) ==
             callsem_virtual_dispatch_view_offset(rhs) &&
         callsem_bit_field_width(lhs) == callsem_bit_field_width(rhs) &&
         callsem_bit_field_offset(lhs) == callsem_bit_field_offset(rhs) &&
         callsem_bit_field_storage_size(lhs) ==
             callsem_bit_field_storage_size(rhs) &&
         callsem_trivial_storage_copy_prefix_bytes(lhs) ==
             callsem_trivial_storage_copy_prefix_bytes(rhs) &&
         callsem_vtt_slice_offset(lhs) == callsem_vtt_slice_offset(rhs) &&
         callsem_vtt_entry_index(lhs) == callsem_vtt_entry_index(rhs) &&
         callsem_source_file(lhs) == callsem_source_file(rhs) &&
         callsem_source_line(lhs) == callsem_source_line(rhs) &&
         callsem_source_column(lhs) == callsem_source_column(rhs) &&
         callsem_vtt_symbol(lhs) == callsem_vtt_symbol(rhs) &&
         callsem_vtt_object_symbol(lhs) == callsem_vtt_object_symbol(rhs) &&
         callsem_runtime_bridge_symbol(lhs) == callsem_runtime_bridge_symbol(rhs) &&
         callsem_local_static_guard_symbol(lhs) ==
             callsem_local_static_guard_symbol(rhs) &&
         callsem_abi_tags(lhs) == callsem_abi_tags(rhs) &&
         callsem_object_aliases(lhs) == callsem_object_aliases(rhs) &&
         callsem_flags_equal(lhs, rhs);
}

uint64_t hash_callsem_shallow_exact(const CallSemNode & node)
{
  uint64_t hash = 0x4dd3f9a35f11c713ULL;
  hash = hash_mix(hash, static_cast<uint64_t>(node.kind));
  hash = hash_mix(hash, static_cast<uint64_t>(node.value_category));
  hash = hash_mix(hash, static_cast<uint64_t>(node.token_type));
  hash = hash_mix(
      hash,
      static_cast<uint64_t>(callsem_special_member_entry_point_kind(node)));
  hash = hash_mix(
      hash,
      static_cast<uint64_t>(callsem_function_ref_qualifier(node)));
  hash = hash_mix(hash, hash_string_value(node.text));
  hash = hash_mix(hash, hash_string_value(callsem_resolved_name(node)));
  hash = hash_mix(hash, hash_qualified_name(callsem_qualified_name_syntax(node)));
  hash = hash_mix(hash, hash_type_ptr_value(node.semantic_type));
  hash = hash_mix(hash, hash_type_ptr_value(callsem_vtt_owner_type(node)));
  hash = hash_mix(hash, hash_type_ptr_value(callsem_materialization_source_type(node)));
  hash = hash_mix(hash, hash_type_ptr_value(callsem_conversion_source_type(node)));
  hash = hash_mix(hash, hash_type_ptr_value(callsem_initializer_list_element_type(node)));
  hash = hash_mix(hash, hash_type_ptr_value(callsem_typeid_operand_type(node)));
  const CallSemVirtualBaseLayout & virtual_base_layout =
      callsem_virtual_base_layout(node);
  hash = hash_mix(hash, virtual_base_layout.size());
  for(size_t i = 0; i < virtual_base_layout.size(); ++i) {
    hash = hash_mix(hash, hash_string_value(virtual_base_layout[i].first));
    hash = hash_mix(hash, virtual_base_layout[i].second);
  }
  hash = hash_mix(hash, hash_symbol_identity(callsem_symbol(node)));
  hash = hash_mix(hash, callsem_uint_value(node));
  hash = hash_mix(hash, static_cast<uint64_t>(callsem_int_value(node)));
  hash = hash_mix(hash, static_cast<uint64_t>(callsem_result_adjust(node)));
  hash = hash_mix(hash, static_cast<uint64_t>(callsem_result_vcall_offset(node)));
  hash = hash_mix(hash, callsem_result_virtual_base_index(node));
  hash = hash_mix(hash, callsem_host_vcall_offset_count(node));
  hash = hash_mix(
      hash,
      static_cast<uint64_t>(callsem_virtual_dispatch_view_offset(node)));
  hash = hash_mix(hash, callsem_bit_field_width(node));
  hash = hash_mix(hash, callsem_bit_field_offset(node));
  hash = hash_mix(hash, callsem_bit_field_storage_size(node));
  hash = hash_mix(hash, callsem_trivial_storage_copy_prefix_bytes(node));
  hash = hash_mix(hash, callsem_vtt_slice_offset(node));
  hash = hash_mix(hash, callsem_vtt_entry_index(node));
  hash = hash_mix(hash, hash_string_value(callsem_source_file(node)));
  hash = hash_mix(hash, callsem_source_line(node));
  hash = hash_mix(hash, callsem_source_column(node));
  hash = hash_mix(hash, hash_string_value(callsem_vtt_symbol(node)));
  hash = hash_mix(hash, hash_string_value(callsem_vtt_object_symbol(node)));
  hash = hash_mix(hash, hash_string_value(callsem_runtime_bridge_symbol(node)));
  hash = hash_mix(hash, hash_string_value(callsem_local_static_guard_symbol(node)));
  const std::vector<std::string> & abi_tags = callsem_abi_tags(node);
  hash = hash_mix(hash, abi_tags.size());
  for(size_t i = 0; i < abi_tags.size(); ++i) {
    hash = hash_mix(hash, hash_string_value(abi_tags[i]));
  }
  const std::vector<std::string> & object_aliases = callsem_object_aliases(node);
  hash = hash_mix(hash, object_aliases.size());
  for(size_t i = 0; i < object_aliases.size(); ++i) {
    hash = hash_mix(hash, hash_string_value(object_aliases[i]));
  }
  hash = hash_mix(hash, hash_callsem_flags(node));
  return hash;
}

bool callsem_shallow_shape_equal(const CallSemNode & lhs, const CallSemNode & rhs)
{
  return lhs.kind == rhs.kind &&
         lhs.has_token == rhs.has_token &&
         lhs.token_type == rhs.token_type &&
         lhs.text == rhs.text;
}

uint64_t hash_callsem_shallow_shape(const CallSemNode & node)
{
  uint64_t hash = 0x97dce2bd3a37ab21ULL;
  hash = hash_mix(hash, static_cast<uint64_t>(node.kind));
  hash = hash_mix(hash, node.has_token ? 1 : 0);
  hash = hash_mix(hash, static_cast<uint64_t>(node.token_type));
  hash = hash_mix(hash, hash_string_value(node.text));
  return hash;
}

bool callsem_exact_equal(const CallSemNode & lhs, const CallSemNode & rhs);
bool callsem_shape_equal(const CallSemNode & lhs, const CallSemNode & rhs);

bool callsem_exact_equal(const CallSemNode & lhs, const CallSemNode & rhs)
{
  if(!callsem_shallow_exact_equal(lhs, rhs) ||
     lhs.children.size() != rhs.children.size()) {
    return false;
  }
  const shared_ptr<CallSemNode> & lhs_lowered = callsem_lowered_condition_test(lhs);
  const shared_ptr<CallSemNode> & rhs_lowered = callsem_lowered_condition_test(rhs);
  if(static_cast<bool>(lhs_lowered) != static_cast<bool>(rhs_lowered)) {
    return false;
  }
  if(lhs_lowered && !callsem_exact_equal(*lhs_lowered, *rhs_lowered)) {
    return false;
  }
  for(size_t i = 0; i < lhs.children.size(); ++i) {
    if(!callsem_exact_equal(lhs.children[i], rhs.children[i])) {
      return false;
    }
  }
  return true;
}

bool callsem_shape_equal(const CallSemNode & lhs, const CallSemNode & rhs)
{
  if(!callsem_shallow_shape_equal(lhs, rhs) ||
     lhs.children.size() != rhs.children.size()) {
    return false;
  }
  const shared_ptr<CallSemNode> & lhs_lowered = callsem_lowered_condition_test(lhs);
  const shared_ptr<CallSemNode> & rhs_lowered = callsem_lowered_condition_test(rhs);
  if(static_cast<bool>(lhs_lowered) != static_cast<bool>(rhs_lowered)) {
    return false;
  }
  if(lhs_lowered && !callsem_shape_equal(*lhs_lowered, *rhs_lowered)) {
    return false;
  }
  for(size_t i = 0; i < lhs.children.size(); ++i) {
    if(!callsem_shape_equal(lhs.children[i], rhs.children[i])) {
      return false;
    }
  }
  return true;
}

size_t callsem_duplicate_shallow_bytes(const CallSemNode & node)
{
  size_t bytes = sizeof(CallSemNode) +
                 callsem_text_storage_bytes(node.text) +
                 callsem_resolved_name_storage_bytes(node) +
                 vector_storage_bytes(node.children) +
                 callsem_symbol_storage_bytes(node, nullptr) +
                 qualified_name_payload_bytes(callsem_qualified_name_syntax(node));
  if(node.extra) {
    bytes += sizeof(CallSemNodeExtra);
    if(node.extra->rare_strings) {
      bytes += sizeof(CallSemRareStrings) +
               string_storage_bytes(node.extra->rare_strings->vtt_symbol) +
               string_storage_bytes(node.extra->rare_strings->vtt_object_symbol) +
               string_storage_bytes(node.extra->rare_strings->runtime_bridge_symbol) +
               string_storage_bytes(
                   node.extra->rare_strings->local_static_guard_symbol);
    }
  }
  const CallSemRarePayload * rare_payload = callsem_rare_payload(node);
  if(rare_payload) {
    bytes += sizeof(CallSemRarePayload) +
             vector_storage_bytes(rare_payload->virtual_base_layout);
    for(size_t i = 0; i < rare_payload->virtual_base_layout.size(); ++i) {
      bytes += string_storage_bytes(rare_payload->virtual_base_layout[i].first);
    }
  }
  return bytes;
}

string quote_callsem_dup_field(const string & value)
{
  ostringstream out;
  out << '"';
  for(size_t i = 0; i < value.size(); ++i) {
    const char ch = value[i];
    if(ch == '"' || ch == '\\') {
      out << '\\' << ch;
    } else if(ch == '\n') {
      out << "\\n";
    } else if(ch == '\r') {
      out << "\\r";
    } else if(ch == '\t') {
      out << "\\t";
    } else {
      out << ch;
    }
  }
  out << '"';
  return out.str();
}

size_t callsem_duplicate_top_limit()
{
  const char * text = getenv("CPPGM_CALLSEM_DUP_HASH_TOP");
  if(!text || !*text) {
    return 20;
  }
  char * end = nullptr;
  const unsigned long value = strtoul(text, &end, 10);
  if(end == text || value == 0) {
    return 20;
  }
  return static_cast<size_t>(value);
}

struct CallSemDuplicateNodeMetrics
{
  uint64_t exact_hash = 0;
  uint64_t shape_hash = 0;
  size_t subtree_nodes = 0;
  size_t subtree_bytes = 0;
};

struct CallSemDuplicateExactGroup
{
  const CallSemNode * representative = nullptr;
  uint64_t hash = 0;
  size_t count = 0;
  size_t output_count = 0;
  size_t cached_count = 0;
  size_t total_subtree_nodes = 0;
  size_t total_subtree_bytes = 0;
  size_t max_subtree_nodes = 0;
  size_t max_subtree_bytes = 0;
};

struct CallSemDuplicateShapeGroup
{
  const CallSemNode * representative = nullptr;
  uint64_t hash = 0;
  size_t count = 0;
  size_t output_count = 0;
  size_t cached_count = 0;
  size_t total_subtree_nodes = 0;
  size_t total_subtree_bytes = 0;
  size_t max_subtree_nodes = 0;
  size_t max_subtree_bytes = 0;
  unordered_set<uint64_t> exact_hashes;
  vector<const CallSemNode *> exact_samples;
};

class CallSemDuplicateHashCensus
{
public:
  void add_root(const CallSemNode & node, bool cached_body)
  {
    ++root_count_;
    visit(node, cached_body);
  }

  void dump(ostream & out) const
  {
    vector<const CallSemDuplicateExactGroup *> exact_groups;
    for(unordered_map<uint64_t, vector<CallSemDuplicateExactGroup> >::const_iterator it =
            exact_groups_.begin();
        it != exact_groups_.end();
        ++it) {
      for(size_t i = 0; i < it->second.size(); ++i) {
        exact_groups.push_back(&it->second[i]);
      }
    }
    vector<const CallSemDuplicateShapeGroup *> shape_groups;
    for(unordered_map<uint64_t, vector<CallSemDuplicateShapeGroup> >::const_iterator it =
            shape_groups_.begin();
        it != shape_groups_.end();
        ++it) {
      for(size_t i = 0; i < it->second.size(); ++i) {
        shape_groups.push_back(&it->second[i]);
      }
    }

    dump_exact_summary(out, exact_groups);
    dump_shape_summary(out, shape_groups);
    dump_top_exact_groups(out, exact_groups);
    dump_top_shape_groups(out, shape_groups);
  }

private:
  CallSemDuplicateNodeMetrics visit(const CallSemNode & node, bool cached_body)
  {
    ++node_occurrences_;
    CallSemDuplicateNodeMetrics metrics;
    metrics.subtree_nodes = 1;
    metrics.subtree_bytes = callsem_duplicate_shallow_bytes(node);
    metrics.exact_hash = hash_callsem_shallow_exact(node);
    metrics.shape_hash = hash_callsem_shallow_shape(node);

    const shared_ptr<CallSemNode> & lowered = callsem_lowered_condition_test(node);
    if(lowered) {
      const CallSemDuplicateNodeMetrics child = visit(*lowered, cached_body);
      metrics.subtree_nodes += child.subtree_nodes;
      metrics.subtree_bytes += child.subtree_bytes;
      metrics.exact_hash = hash_mix(metrics.exact_hash, 0x719ab891d54f5b43ULL);
      metrics.exact_hash = hash_mix(metrics.exact_hash, child.exact_hash);
      metrics.shape_hash = hash_mix(metrics.shape_hash, 0x9f92f715e53bb1a2ULL);
      metrics.shape_hash = hash_mix(metrics.shape_hash, child.shape_hash);
    }

    metrics.exact_hash = hash_mix(metrics.exact_hash, node.children.size());
    metrics.shape_hash = hash_mix(metrics.shape_hash, node.children.size());
    for(size_t i = 0; i < node.children.size(); ++i) {
      const CallSemDuplicateNodeMetrics child = visit(node.children[i], cached_body);
      metrics.subtree_nodes += child.subtree_nodes;
      metrics.subtree_bytes += child.subtree_bytes;
      metrics.exact_hash = hash_mix(metrics.exact_hash, child.exact_hash);
      metrics.shape_hash = hash_mix(metrics.shape_hash, child.shape_hash);
    }

    add_exact_group(node, cached_body, metrics);
    add_shape_group(node, cached_body, metrics);
    return metrics;
  }

  void add_exact_group(const CallSemNode & node,
                       bool cached_body,
                       const CallSemDuplicateNodeMetrics & metrics)
  {
    vector<CallSemDuplicateExactGroup> & bucket = exact_groups_[metrics.exact_hash];
    for(size_t i = 0; i < bucket.size(); ++i) {
      if(callsem_exact_equal(*bucket[i].representative, node)) {
        note_exact_group(bucket[i], cached_body, metrics);
        return;
      }
    }
    CallSemDuplicateExactGroup group;
    group.representative = &node;
    group.hash = metrics.exact_hash;
    note_exact_group(group, cached_body, metrics);
    bucket.push_back(group);
  }

  void add_shape_group(const CallSemNode & node,
                       bool cached_body,
                       const CallSemDuplicateNodeMetrics & metrics)
  {
    vector<CallSemDuplicateShapeGroup> & bucket = shape_groups_[metrics.shape_hash];
    for(size_t i = 0; i < bucket.size(); ++i) {
      if(callsem_shape_equal(*bucket[i].representative, node)) {
        note_shape_group(bucket[i], node, cached_body, metrics);
        return;
      }
    }
    CallSemDuplicateShapeGroup group;
    group.representative = &node;
    group.hash = metrics.shape_hash;
    note_shape_group(group, node, cached_body, metrics);
    bucket.push_back(group);
  }

  static void note_exact_group(CallSemDuplicateExactGroup & group,
                               bool cached_body,
                               const CallSemDuplicateNodeMetrics & metrics)
  {
    ++group.count;
    if(cached_body) {
      ++group.cached_count;
    } else {
      ++group.output_count;
    }
    group.total_subtree_nodes += metrics.subtree_nodes;
    group.total_subtree_bytes += metrics.subtree_bytes;
    group.max_subtree_nodes = max(group.max_subtree_nodes, metrics.subtree_nodes);
    group.max_subtree_bytes = max(group.max_subtree_bytes, metrics.subtree_bytes);
  }

  static void note_shape_group(CallSemDuplicateShapeGroup & group,
                               const CallSemNode & node,
                               bool cached_body,
                               const CallSemDuplicateNodeMetrics & metrics)
  {
    ++group.count;
    if(cached_body) {
      ++group.cached_count;
    } else {
      ++group.output_count;
    }
    group.total_subtree_nodes += metrics.subtree_nodes;
    group.total_subtree_bytes += metrics.subtree_bytes;
    group.max_subtree_nodes = max(group.max_subtree_nodes, metrics.subtree_nodes);
    group.max_subtree_bytes = max(group.max_subtree_bytes, metrics.subtree_bytes);
    if(group.exact_hashes.insert(metrics.exact_hash).second) {
      group.exact_samples.push_back(&node);
    }
  }

  static size_t duplicate_group_bytes(const CallSemDuplicateExactGroup & group)
  {
    return group.total_subtree_bytes > group.max_subtree_bytes ?
        group.total_subtree_bytes - group.max_subtree_bytes :
        0;
  }

  static size_t duplicate_group_nodes(const CallSemDuplicateExactGroup & group)
  {
    return group.total_subtree_nodes > group.max_subtree_nodes ?
        group.total_subtree_nodes - group.max_subtree_nodes :
        0;
  }

  static size_t candidate_group_bytes(const CallSemDuplicateShapeGroup & group)
  {
    return group.total_subtree_bytes > group.max_subtree_bytes ?
        group.total_subtree_bytes - group.max_subtree_bytes :
        0;
  }

  static size_t candidate_group_nodes(const CallSemDuplicateShapeGroup & group)
  {
    return group.total_subtree_nodes > group.max_subtree_nodes ?
        group.total_subtree_nodes - group.max_subtree_nodes :
        0;
  }

  static bool exact_group_duplicate_less(const CallSemDuplicateExactGroup * lhs,
                                         const CallSemDuplicateExactGroup * rhs)
  {
    const size_t lhs_bytes = duplicate_group_bytes(*lhs);
    const size_t rhs_bytes = duplicate_group_bytes(*rhs);
    if(lhs_bytes != rhs_bytes) {
      return lhs_bytes > rhs_bytes;
    }
    if(lhs->count != rhs->count) {
      return lhs->count > rhs->count;
    }
    return lhs->hash < rhs->hash;
  }

  static bool shape_group_candidate_less(const CallSemDuplicateShapeGroup * lhs,
                                         const CallSemDuplicateShapeGroup * rhs)
  {
    const size_t lhs_bytes = candidate_group_bytes(*lhs);
    const size_t rhs_bytes = candidate_group_bytes(*rhs);
    if(lhs_bytes != rhs_bytes) {
      return lhs_bytes > rhs_bytes;
    }
    if(lhs->exact_hashes.size() != rhs->exact_hashes.size()) {
      return lhs->exact_hashes.size() > rhs->exact_hashes.size();
    }
    if(lhs->count != rhs->count) {
      return lhs->count > rhs->count;
    }
    return lhs->hash < rhs->hash;
  }

  void dump_exact_summary(
      ostream & out,
      const vector<const CallSemDuplicateExactGroup *> & groups) const
  {
    size_t duplicate_groups = 0;
    size_t duplicate_subtrees = 0;
    size_t duplicate_nodes = 0;
    size_t duplicate_bytes = 0;
    size_t mixed_groups = 0;
    size_t mixed_bytes = 0;
    size_t output_only_groups = 0;
    size_t output_only_bytes = 0;
    size_t cached_only_groups = 0;
    size_t cached_only_bytes = 0;
    for(size_t i = 0; i < groups.size(); ++i) {
      if(groups[i]->count <= 1) {
        continue;
      }
      ++duplicate_groups;
      duplicate_subtrees += groups[i]->count - 1;
      duplicate_nodes += duplicate_group_nodes(*groups[i]);
      const size_t group_bytes = duplicate_group_bytes(*groups[i]);
      duplicate_bytes += group_bytes;
      if(groups[i]->output_count != 0 && groups[i]->cached_count != 0) {
        ++mixed_groups;
        mixed_bytes += group_bytes;
      } else if(groups[i]->output_count != 0) {
        ++output_only_groups;
        output_only_bytes += group_bytes;
      } else {
        ++cached_only_groups;
        cached_only_bytes += group_bytes;
      }
    }
    out << "callsem-dup-summary"
        << " profile=exact"
        << " roots=" << root_count_
        << " node_occurrences=" << node_occurrences_
        << " groups=" << groups.size()
        << " duplicate_groups=" << duplicate_groups
        << " duplicate_subtrees=" << duplicate_subtrees
        << " duplicate_nodes_overlap=" << duplicate_nodes
        << " duplicate_subtree_bytes_overlap=" << duplicate_bytes
        << " mixed_groups=" << mixed_groups
        << " mixed_bytes_overlap=" << mixed_bytes
        << " output_only_groups=" << output_only_groups
        << " output_only_bytes_overlap=" << output_only_bytes
        << " cached_only_groups=" << cached_only_groups
        << " cached_only_bytes_overlap=" << cached_only_bytes
        << '\n';
  }

  void dump_shape_summary(
      ostream & out,
      const vector<const CallSemDuplicateShapeGroup *> & groups) const
  {
    size_t duplicate_groups = 0;
    size_t candidate_groups = 0;
    size_t candidate_subtrees = 0;
    size_t candidate_nodes = 0;
    size_t candidate_bytes = 0;
    size_t candidate_exact_variants = 0;
    size_t mixed_candidate_groups = 0;
    size_t mixed_candidate_bytes = 0;
    size_t output_only_candidate_groups = 0;
    size_t output_only_candidate_bytes = 0;
    size_t cached_only_candidate_groups = 0;
    size_t cached_only_candidate_bytes = 0;
    for(size_t i = 0; i < groups.size(); ++i) {
      if(groups[i]->count > 1) {
        ++duplicate_groups;
      }
      if(groups[i]->count <= 1 || groups[i]->exact_hashes.size() <= 1) {
        continue;
      }
      ++candidate_groups;
      candidate_subtrees += groups[i]->count - 1;
      candidate_nodes += candidate_group_nodes(*groups[i]);
      const size_t group_bytes = candidate_group_bytes(*groups[i]);
      candidate_bytes += group_bytes;
      candidate_exact_variants += groups[i]->exact_hashes.size();
      if(groups[i]->output_count != 0 && groups[i]->cached_count != 0) {
        ++mixed_candidate_groups;
        mixed_candidate_bytes += group_bytes;
      } else if(groups[i]->output_count != 0) {
        ++output_only_candidate_groups;
        output_only_candidate_bytes += group_bytes;
      } else {
        ++cached_only_candidate_groups;
        cached_only_candidate_bytes += group_bytes;
      }
    }
    out << "callsem-dup-summary"
        << " profile=shape"
        << " roots=" << root_count_
        << " node_occurrences=" << node_occurrences_
        << " groups=" << groups.size()
        << " duplicate_groups=" << duplicate_groups
        << " nonexact_candidate_groups=" << candidate_groups
        << " nonexact_candidate_subtrees=" << candidate_subtrees
        << " nonexact_candidate_nodes_overlap=" << candidate_nodes
        << " nonexact_candidate_subtree_bytes_overlap=" << candidate_bytes
        << " nonexact_candidate_exact_variants=" << candidate_exact_variants
        << " mixed_candidate_groups=" << mixed_candidate_groups
        << " mixed_candidate_bytes_overlap=" << mixed_candidate_bytes
        << " output_only_candidate_groups=" << output_only_candidate_groups
        << " output_only_candidate_bytes_overlap=" << output_only_candidate_bytes
        << " cached_only_candidate_groups=" << cached_only_candidate_groups
        << " cached_only_candidate_bytes_overlap=" << cached_only_candidate_bytes
        << '\n';
  }

  void dump_top_exact_groups(
      ostream & out,
      vector<const CallSemDuplicateExactGroup *> groups) const
  {
    groups.erase(remove_if(groups.begin(),
                           groups.end(),
                           [](const CallSemDuplicateExactGroup * group)
                           { return group->count <= 1; }),
                 groups.end());
    sort(groups.begin(), groups.end(), exact_group_duplicate_less);
    const size_t limit = min(callsem_duplicate_top_limit(), groups.size());
    for(size_t i = 0; i < limit; ++i) {
      const CallSemDuplicateExactGroup & group = *groups[i];
      dump_group_prefix(out,
                        "callsem-dup-group",
                        "exact",
                        i + 1,
                        group.hash,
                        group.count,
                        group.output_count,
                        group.cached_count,
                        group.max_subtree_nodes,
                        group.max_subtree_bytes,
                        duplicate_group_nodes(group),
                        duplicate_group_bytes(group),
                        *group.representative);
      out << '\n';
    }
  }

  void dump_top_shape_groups(
      ostream & out,
      vector<const CallSemDuplicateShapeGroup *> groups) const
  {
    groups.erase(remove_if(groups.begin(),
                           groups.end(),
                           [](const CallSemDuplicateShapeGroup * group)
                           {
                             return group->count <= 1 ||
                                    group->exact_hashes.size() <= 1;
                           }),
                 groups.end());
    sort(groups.begin(), groups.end(), shape_group_candidate_less);
    const size_t limit = min(callsem_duplicate_top_limit(), groups.size());
    for(size_t i = 0; i < limit; ++i) {
      const CallSemDuplicateShapeGroup & group = *groups[i];
      dump_group_prefix(out,
                        "callsem-dup-group",
                        "shape",
                        i + 1,
                        group.hash,
                        group.count,
                        group.output_count,
                        group.cached_count,
                        group.max_subtree_nodes,
                        group.max_subtree_bytes,
                        candidate_group_nodes(group),
                        candidate_group_bytes(group),
                        *group.representative);
      out << " exact_variants=" << group.exact_hashes.size()
          << " variation=" << quote_callsem_dup_field(variation_mask(group))
          << '\n';
    }
  }

  static void dump_group_prefix(ostream & out,
                                const char * line_kind,
                                const char * profile,
                                size_t rank,
                                uint64_t hash,
                                size_t count,
                                size_t output_count,
                                size_t cached_count,
                                size_t representative_nodes,
                                size_t representative_bytes,
                                size_t duplicate_nodes,
                                size_t duplicate_bytes,
                                const CallSemNode & representative)
  {
    out << line_kind
        << " profile=" << profile
        << " rank=" << rank
        << " hash=" << hash
        << " count=" << count
        << " output=" << output_count
        << " cached=" << cached_count
        << " representative_nodes=" << representative_nodes
        << " representative_bytes=" << representative_bytes
        << " duplicate_nodes_overlap=" << duplicate_nodes
        << " duplicate_subtree_bytes_overlap=" << duplicate_bytes
        << " kind=" << callsem_kind_text(representative.kind)
        << " text=" << quote_callsem_dup_field(representative.text);
  }

  static string variation_mask(const CallSemDuplicateShapeGroup & group)
  {
    if(group.exact_samples.size() <= 1) {
      return "none";
    }
    const CallSemNode & first = *group.exact_samples[0];
    set<string> variations;
    for(size_t i = 1; i < group.exact_samples.size(); ++i) {
      const CallSemNode & other = *group.exact_samples[i];
      note_variations(first, other, variations);
    }
    if(variations.empty()) {
      variations.insert("descendant");
    }
    string result;
    for(set<string>::const_iterator it = variations.begin();
        it != variations.end();
        ++it) {
      if(!result.empty()) {
        result += ",";
      }
      result += *it;
    }
    return result;
  }

  static void note_variations(const CallSemNode & lhs,
                              const CallSemNode & rhs,
                              set<string> & variations)
  {
    if(lhs.value_category != rhs.value_category ||
       callsem_uint_value(lhs) != callsem_uint_value(rhs) ||
       callsem_int_value(lhs) != callsem_int_value(rhs) ||
       callsem_result_adjust(lhs) != callsem_result_adjust(rhs) ||
       callsem_result_vcall_offset(lhs) != callsem_result_vcall_offset(rhs) ||
       callsem_result_virtual_base_index(lhs) !=
           callsem_result_virtual_base_index(rhs) ||
       callsem_host_vcall_offset_count(lhs) !=
           callsem_host_vcall_offset_count(rhs) ||
       callsem_virtual_dispatch_view_offset(lhs) !=
           callsem_virtual_dispatch_view_offset(rhs) ||
       callsem_bit_field_width(lhs) != callsem_bit_field_width(rhs) ||
       callsem_bit_field_offset(lhs) != callsem_bit_field_offset(rhs) ||
       callsem_bit_field_storage_size(lhs) !=
           callsem_bit_field_storage_size(rhs) ||
       callsem_trivial_storage_copy_prefix_bytes(lhs) !=
           callsem_trivial_storage_copy_prefix_bytes(rhs) ||
       callsem_vtt_slice_offset(lhs) != callsem_vtt_slice_offset(rhs) ||
       callsem_vtt_entry_index(lhs) != callsem_vtt_entry_index(rhs) ||
       callsem_special_member_entry_point_kind(lhs) !=
           callsem_special_member_entry_point_kind(rhs) ||
       callsem_function_ref_qualifier(lhs) !=
           callsem_function_ref_qualifier(rhs)) {
      variations.insert("value");
    }
    if(!callsem_flags_equal(lhs, rhs)) {
      variations.insert("flags");
    }
    if(callsem_resolved_name(lhs) != callsem_resolved_name(rhs)) {
      variations.insert("resolved_name");
    }
    if(!qualified_name_equal(callsem_qualified_name_syntax(lhs),
                            callsem_qualified_name_syntax(rhs))) {
      variations.insert("qualified_name");
    }
    if(lhs.semantic_type.get() != rhs.semantic_type.get() ||
       callsem_vtt_owner_type(lhs).get() != callsem_vtt_owner_type(rhs).get() ||
       callsem_materialization_source_type(lhs).get() !=
           callsem_materialization_source_type(rhs).get() ||
       callsem_conversion_source_type(lhs).get() !=
           callsem_conversion_source_type(rhs).get() ||
       callsem_initializer_list_element_type(lhs).get() !=
           callsem_initializer_list_element_type(rhs).get() ||
       callsem_typeid_operand_type(lhs).get() !=
           callsem_typeid_operand_type(rhs).get()) {
      variations.insert("type");
    }
    if(!symbol_identity_equal(callsem_symbol(lhs), callsem_symbol(rhs))) {
      variations.insert("symbol");
    }
    if(callsem_source_file(lhs) != callsem_source_file(rhs) ||
       callsem_source_line(lhs) != callsem_source_line(rhs) ||
       callsem_source_column(lhs) != callsem_source_column(rhs)) {
      variations.insert("source");
    }
    if(callsem_vtt_symbol(lhs) != callsem_vtt_symbol(rhs) ||
       callsem_vtt_object_symbol(lhs) != callsem_vtt_object_symbol(rhs) ||
       callsem_runtime_bridge_symbol(lhs) != callsem_runtime_bridge_symbol(rhs) ||
       callsem_local_static_guard_symbol(lhs) !=
           callsem_local_static_guard_symbol(rhs) ||
       callsem_abi_tags(lhs) != callsem_abi_tags(rhs) ||
       callsem_object_aliases(lhs) != callsem_object_aliases(rhs)) {
      variations.insert("sidecar_string");
    }
    if(callsem_virtual_base_layout(lhs) != callsem_virtual_base_layout(rhs)) {
      variations.insert("virtual_base_layout");
    }
  }

  size_t root_count_ = 0;
  size_t node_occurrences_ = 0;
  unordered_map<uint64_t, vector<CallSemDuplicateExactGroup> > exact_groups_;
  unordered_map<uint64_t, vector<CallSemDuplicateShapeGroup> > shape_groups_;
};

void note_exact_qualified_name_strings(ExactStringRetentionCensus & census,
                                       ExactStringCategory category,
                                       const QualifiedName & name)
{
  census.note(category, name.name);
  for(size_t i = 0; i < name.qualifiers.size(); ++i) {
    census.note(category, name.qualifiers[i]);
  }
}

void note_exact_template_argument_strings(
    ExactStringRetentionCensus & census,
    const vector<TemplateArgument> & arguments)
{
  for(size_t i = 0; i < arguments.size(); ++i) {
    census.note(ESC_TEMPLATE_ARGUMENT, arguments[i].text);
    census.note(ESC_TEMPLATE_ARGUMENT,
                arguments[i].rare().function_internal_symbol);
    if(arguments[i].template_entity_identity) {
      census.note(ESC_TEMPLATE_ARGUMENT,
                  arguments[i].template_entity_identity->scope_prefix);
      census.note(ESC_TEMPLATE_ARGUMENT,
                  arguments[i].template_entity_identity->name);
      note_exact_qualified_name_strings(
          census,
          ESC_TEMPLATE_ARGUMENT,
          arguments[i].template_entity_identity->name_syntax);
    }
  }
}

void dump_exact_string_retention_census(
    ostream & out,
    const MemoryCensusInput & input,
    const unordered_set<const Scope *> & seen_scopes,
    const unordered_set<const FunctionBinding *> & seen_functions,
    const unordered_set<const ClassInfo *> & seen_classes,
    const unordered_set<const Type *> & seen_types)
{
  ExactStringRetentionCensus census;
  for(unordered_set<const Type *>::const_iterator it = seen_types.begin();
      it != seen_types.end();
      ++it) {
    const Type & type = **it;
    census.note(ESC_TYPE_DISPLAY, type.named_display);
    census.note(ESC_TYPE_KEY, type.named_key);
    if(!type.named_rare_metadata) {
      continue;
    }
    const Type::NamedRareMetadata & rare = *type.named_rare_metadata;
    note_exact_qualified_name_strings(
        census, ESC_TYPE_QUALIFIED_NAME, rare.qualified_name);
    for(size_t i = 0; i < rare.named_abi_tags.size(); ++i) {
      census.note(ESC_TYPE_MANGLE, rare.named_abi_tags[i]);
    }
    if(!rare.named_class_template_specialization_mangle_info) {
      continue;
    }
    const ClassTemplateSpecializationMangleInfo & mangle =
        *rare.named_class_template_specialization_mangle_info;
    note_exact_qualified_name_strings(
        census, ESC_TYPE_MANGLE, mangle.template_name_syntax);
    census.note(ESC_TYPE_MANGLE, mangle.template_scope_prefix);
    census.note(ESC_TYPE_MANGLE, mangle.template_name);
    note_exact_template_argument_strings(census, mangle.mangle_arguments);
    note_exact_template_argument_strings(
        census, mangle.arguments.const_values());
  }

  for(unordered_set<const ClassInfo *>::const_iterator it = seen_classes.begin();
      it != seen_classes.end();
      ++it) {
    const ClassInfo & info = **it;
    census.note(ESC_CLASS_QUALIFIED_NAME, info.qualified_name);
    census.note(ESC_CLASS_INSTANTIATION_KEY, info.instantiation_key);
    for(size_t i = 0; i < info.instantiation_arg_texts.size(); ++i) {
      census.note(ESC_CLASS_ARGUMENT_TEXT, info.instantiation_arg_texts[i]);
    }
    note_exact_template_argument_strings(census, info.instantiation_arguments);
    note_exact_template_argument_strings(
        census, class_instantiation_binding_arguments(info));
  }

  for(unordered_set<const Scope *>::const_iterator it = seen_scopes.begin();
      it != seen_scopes.end();
      ++it) {
    const Scope & scope = **it;
    census.note(ESC_SCOPE_NAME, scope.name);
    for(Scope::NamedTypeMap::const_iterator named = scope.named_types.begin();
        named != scope.named_types.end();
        ++named) {
      census.note(ESC_SCOPE_NAMED_TYPE_KEY, named->first);
    }
  }

  for(unordered_set<const FunctionBinding *>::const_iterator
          it = seen_functions.begin();
      it != seen_functions.end();
      ++it) {
    census.note(ESC_FUNCTION_NAME, (*it)->name);
    census.note(ESC_FUNCTION_INSTANTIATION_KEY,
                (*it)->template_instantiation_key);
    note_exact_template_argument_strings(census, (*it)->instantiation_arguments);
  }

  for(size_t i = 0; i < input.class_templates.size(); ++i) {
    const ClassTemplateDecl & decl = *input.class_templates[i];
    for(map<string, ClassInfo *>::const_iterator it = decl.instantiations.begin();
        it != decl.instantiations.end();
        ++it) {
      census.note(ESC_CLASS_TEMPLATE_INSTANTIATION_KEY, it->first);
    }
    for(map<string, ClassInfo *>::const_iterator
            it = decl.reference_instantiations.begin();
        it != decl.reference_instantiations.end();
        ++it) {
      census.note(ESC_CLASS_TEMPLATE_REFERENCE_KEY, it->first);
    }
  }
  census.dump(out);
}

}  // namespace

void dump_memory_census(ostream & out, const MemoryCensusInput & input)
{
  MemoryCensus census;
  RetainedAstMemoryCensus retained_ast_census;
  ScopedRetainedAstMemoryCensus retained_ast_scope(retained_ast_census);
  unordered_set<const Scope *> seen_scopes;
  unordered_set<const FunctionBinding *> seen_functions;
  unordered_set<const ClassInfo *> seen_classes;
  unordered_set<const Type *> seen_types;
  unordered_set<const CallSemNodeExtra *> seen_callsem_extras;
  unordered_set<const CallSemRareStrings *> seen_callsem_rare_strings;
  unordered_set<const CallSemRarePayload *> seen_callsem_rare_payloads;
  unordered_set<uint32_t> seen_callsem_source_files;
  unordered_set<const symbol_linkage::SymbolIdentity *> seen_callsem_symbols;

  census.note("analyzer.owner_vectors",
              vector_storage_bytes(input.functions) +
                  vector_storage_bytes(input.classes) +
                  vector_storage_bytes(input.function_templates) +
                  vector_storage_bytes(input.class_templates) +
                  vector_storage_bytes(input.alias_templates) +
                  vector_storage_bytes(input.variable_templates) +
                  vector_storage_bytes(input.template_scopes) +
                  vector_storage_bytes(input.captured_local_scopes) +
                  vector_storage_bytes(input.durable_type_scopes) +
                  vector_storage_bytes(input.synthetic_ast_nodes) +
                  vector_storage_bytes(input.instantiated_functions) +
                  vector_storage_bytes(input.instantiated_classes) +
                  vector_storage_bytes(input.required_function_definitions) +
                  vector_storage_bytes(input.late_required_class_methods) +
                  vector_storage_bytes(input.late_required_class_static_functions) +
                  vector_storage_bytes(input.synthetic_functions) +
                  vector_storage_bytes(input.deferred_constexpr_functions));

  census_scope(input.root, census, seen_scopes, seen_types);
  for(size_t i = 0; i < input.template_scopes.size(); ++i) {
    census_scope(input.template_scopes[i].get(), census, seen_scopes, seen_types);
  }
  for(size_t i = 0; i < input.captured_local_scopes.size(); ++i) {
    census_scope(input.captured_local_scopes[i].get(), census, seen_scopes, seen_types);
  }
  for(size_t i = 0; i < input.durable_type_scopes.size(); ++i) {
    census_scope(input.durable_type_scopes[i].get(), census, seen_scopes, seen_types);
  }

  for(size_t i = 0; i < input.functions.size(); ++i) {
    census_function_binding(input.functions[i].get(),
                            census,
                            seen_functions,
                            seen_types,
                            seen_callsem_extras,
                            seen_callsem_rare_strings,
                            seen_callsem_rare_payloads,
                            seen_callsem_source_files,
                            seen_callsem_symbols);
  }
  for(size_t i = 0; i < input.classes.size(); ++i) {
    census_class_info(input.classes[i].get(),
                      census,
                      seen_classes,
                      seen_scopes,
                      seen_functions,
                      seen_types,
                      seen_callsem_extras,
                      seen_callsem_rare_strings,
                      seen_callsem_rare_payloads,
                      seen_callsem_source_files,
                      seen_callsem_symbols);
  }
  for(size_t i = 0; i < input.function_templates.size(); ++i) {
    census_function_template(*input.function_templates[i], census, seen_scopes, seen_types);
  }
  for(size_t i = 0; i < input.alias_templates.size(); ++i) {
    census_alias_template(*input.alias_templates[i], census, seen_scopes, seen_types);
  }
  for(size_t i = 0; i < input.class_templates.size(); ++i) {
    census_class_template(*input.class_templates[i], census, seen_types);
  }
  for(size_t i = 0; i < input.variable_templates.size(); ++i) {
    census_variable_template(*input.variable_templates[i], census, seen_scopes, seen_types);
  }
  for(size_t i = 0; i < input.synthetic_ast_nodes.size(); ++i) {
    if(input.synthetic_ast_nodes[i]) {
      census_cpp_ast_node(*input.synthetic_ast_nodes[i], "cppast.synthetic", census);
    }
  }

  census_semantic_cache(input.cache_state, census, seen_types);
  census_callsem_node(input.translation_unit,
                      "callsem.output",
                      census,
                      seen_types,
                      seen_callsem_extras,
                      seen_callsem_rare_strings,
                      seen_callsem_rare_payloads,
                      seen_callsem_source_files,
                      seen_callsem_symbols);
  census.dump(out);
  retained_ast_census.dump(out);
  dump_exact_string_retention_census(out,
                                     input,
                                     seen_scopes,
                                     seen_functions,
                                     seen_classes,
                                     seen_types);
}

void dump_source_ast_memory_census(ostream & out,
                                   const CppAstNode & source_ast)
{
  MemoryCensus census;
  census_cpp_ast_node(source_ast, "cppast.source", census);
  census.dump(out);
}

void dump_callsem_duplicate_hash_census(ostream & out, const MemoryCensusInput & input)
{
  CallSemDuplicateHashCensus census;
  census.add_root(input.translation_unit, false);
  for(size_t i = 0; i < input.functions.size(); ++i) {
    if(input.functions[i] && input.functions[i]->cached_body_output) {
      census.add_root(*input.functions[i]->cached_body_output, true);
    }
  }
  census.dump(out);
}

namespace {

struct CallSemProvenanceIndexedNode
{
  const CallSemNode * node = nullptr;
  CallSemDuplicateNodeMetrics metrics;
};

struct CallSemProvenanceCachedRoot
{
  const FunctionBinding * binding = nullptr;
  CallSemDuplicateNodeMetrics metrics;
  size_t exact_output_matches = 0;
  size_t shape_output_matches = 0;
  const CallSemNode * first_shape_match = nullptr;

  bool exact_output_match() const
  {
    return exact_output_matches != 0;
  }

  bool shape_only_output_match() const
  {
    return exact_output_matches == 0 && shape_output_matches != 0;
  }
};

CallSemDuplicateNodeMetrics compute_callsem_provenance_metrics(
    const CallSemNode & node,
    unordered_map<uint64_t, vector<CallSemProvenanceIndexedNode> > * exact_index,
    unordered_map<uint64_t, vector<CallSemProvenanceIndexedNode> > * shape_index)
{
  CallSemDuplicateNodeMetrics metrics;
  metrics.subtree_nodes = 1;
  metrics.subtree_bytes = callsem_duplicate_shallow_bytes(node);
  metrics.exact_hash = hash_callsem_shallow_exact(node);
  metrics.shape_hash = hash_callsem_shallow_shape(node);

  const shared_ptr<CallSemNode> & lowered = callsem_lowered_condition_test(node);
  if(lowered) {
    const CallSemDuplicateNodeMetrics child =
        compute_callsem_provenance_metrics(*lowered, exact_index, shape_index);
    metrics.subtree_nodes += child.subtree_nodes;
    metrics.subtree_bytes += child.subtree_bytes;
    metrics.exact_hash = hash_mix(metrics.exact_hash, 0x719ab891d54f5b43ULL);
    metrics.exact_hash = hash_mix(metrics.exact_hash, child.exact_hash);
    metrics.shape_hash = hash_mix(metrics.shape_hash, 0x9f92f715e53bb1a2ULL);
    metrics.shape_hash = hash_mix(metrics.shape_hash, child.shape_hash);
  }

  metrics.exact_hash = hash_mix(metrics.exact_hash, node.children.size());
  metrics.shape_hash = hash_mix(metrics.shape_hash, node.children.size());
  for(size_t i = 0; i < node.children.size(); ++i) {
    const CallSemDuplicateNodeMetrics child =
        compute_callsem_provenance_metrics(node.children[i], exact_index, shape_index);
    metrics.subtree_nodes += child.subtree_nodes;
    metrics.subtree_bytes += child.subtree_bytes;
    metrics.exact_hash = hash_mix(metrics.exact_hash, child.exact_hash);
    metrics.shape_hash = hash_mix(metrics.shape_hash, child.shape_hash);
  }

  if(exact_index) {
    CallSemProvenanceIndexedNode indexed;
    indexed.node = &node;
    indexed.metrics = metrics;
    (*exact_index)[metrics.exact_hash].push_back(indexed);
  }
  if(shape_index) {
    CallSemProvenanceIndexedNode indexed;
    indexed.node = &node;
    indexed.metrics = metrics;
    (*shape_index)[metrics.shape_hash].push_back(indexed);
  }
  return metrics;
}

string callsem_provenance_variation_mask(const CallSemNode & lhs,
                                         const CallSemNode & rhs)
{
  set<string> variations;
  if(lhs.value_category != rhs.value_category ||
     callsem_uint_value(lhs) != callsem_uint_value(rhs) ||
     callsem_int_value(lhs) != callsem_int_value(rhs) ||
     callsem_result_adjust(lhs) != callsem_result_adjust(rhs) ||
     callsem_result_vcall_offset(lhs) != callsem_result_vcall_offset(rhs) ||
     callsem_result_virtual_base_index(lhs) !=
         callsem_result_virtual_base_index(rhs) ||
     callsem_host_vcall_offset_count(lhs) !=
         callsem_host_vcall_offset_count(rhs) ||
     callsem_bit_field_width(lhs) != callsem_bit_field_width(rhs) ||
     callsem_bit_field_offset(lhs) != callsem_bit_field_offset(rhs) ||
     callsem_bit_field_storage_size(lhs) != callsem_bit_field_storage_size(rhs) ||
     callsem_trivial_storage_copy_prefix_bytes(lhs) !=
         callsem_trivial_storage_copy_prefix_bytes(rhs) ||
     callsem_vtt_slice_offset(lhs) != callsem_vtt_slice_offset(rhs) ||
     callsem_vtt_entry_index(lhs) != callsem_vtt_entry_index(rhs) ||
     callsem_special_member_entry_point_kind(lhs) !=
         callsem_special_member_entry_point_kind(rhs) ||
     callsem_function_ref_qualifier(lhs) !=
         callsem_function_ref_qualifier(rhs)) {
    variations.insert("value");
  }
  if(!callsem_flags_equal(lhs, rhs)) {
    variations.insert("flags");
  }
  if(callsem_resolved_name(lhs) != callsem_resolved_name(rhs)) {
    variations.insert("resolved_name");
  }
  if(!qualified_name_equal(callsem_qualified_name_syntax(lhs),
                          callsem_qualified_name_syntax(rhs))) {
    variations.insert("qualified_name");
  }
  if(lhs.semantic_type.get() != rhs.semantic_type.get() ||
     callsem_vtt_owner_type(lhs).get() != callsem_vtt_owner_type(rhs).get() ||
     callsem_materialization_source_type(lhs).get() !=
         callsem_materialization_source_type(rhs).get() ||
     callsem_conversion_source_type(lhs).get() !=
         callsem_conversion_source_type(rhs).get() ||
     callsem_typeid_operand_type(lhs).get() !=
         callsem_typeid_operand_type(rhs).get()) {
    variations.insert("type");
  }
  if(!symbol_identity_equal(callsem_symbol(lhs), callsem_symbol(rhs))) {
    variations.insert("symbol");
  }
  if(callsem_source_file(lhs) != callsem_source_file(rhs) ||
     callsem_source_line(lhs) != callsem_source_line(rhs) ||
     callsem_source_column(lhs) != callsem_source_column(rhs)) {
    variations.insert("source");
  }
  if(callsem_vtt_symbol(lhs) != callsem_vtt_symbol(rhs) ||
     callsem_vtt_object_symbol(lhs) != callsem_vtt_object_symbol(rhs) ||
     callsem_runtime_bridge_symbol(lhs) != callsem_runtime_bridge_symbol(rhs) ||
     callsem_local_static_guard_symbol(lhs) !=
         callsem_local_static_guard_symbol(rhs) ||
     callsem_abi_tags(lhs) != callsem_abi_tags(rhs) ||
     callsem_object_aliases(lhs) != callsem_object_aliases(rhs)) {
    variations.insert("sidecar_string");
  }
  if(callsem_virtual_base_layout(lhs) != callsem_virtual_base_layout(rhs)) {
    variations.insert("virtual_base_layout");
  }
  if(variations.empty()) {
    variations.insert("descendant");
  }

  string result;
  for(set<string>::const_iterator it = variations.begin();
      it != variations.end();
      ++it) {
    if(!result.empty()) {
      result += ",";
    }
    result += *it;
  }
  return result;
}

bool callsem_provenance_cached_root_less(const CallSemProvenanceCachedRoot & lhs,
                                         const CallSemProvenanceCachedRoot & rhs)
{
  if(lhs.metrics.subtree_bytes != rhs.metrics.subtree_bytes) {
    return lhs.metrics.subtree_bytes > rhs.metrics.subtree_bytes;
  }
  const string lhs_symbol =
      lhs.binding ? lhs.binding->symbol.internal_symbol : string();
  const string rhs_symbol =
      rhs.binding ? rhs.binding->symbol.internal_symbol : string();
  return lhs_symbol < rhs_symbol;
}

string callsem_provenance_binding_symbol(const FunctionBinding & binding)
{
  if(!binding.symbol.internal_symbol.empty()) {
    return binding.symbol.internal_symbol;
  }
  return binding.symbol.object_symbol;
}

void dump_callsem_provenance_top_roots(
    ostream & out,
    vector<CallSemProvenanceCachedRoot> roots,
    const string & status,
    size_t limit)
{
  sort(roots.begin(), roots.end(), callsem_provenance_cached_root_less);
  const size_t actual_limit = min(limit, roots.size());
  for(size_t i = 0; i < actual_limit; ++i) {
    const CallSemProvenanceCachedRoot & root = roots[i];
    const FunctionBinding * binding = root.binding;
    out << "callsem-provenance-cached-body"
        << " rank=" << (i + 1)
        << " status=" << status
        << " nodes=" << root.metrics.subtree_nodes
        << " bytes=" << root.metrics.subtree_bytes
        << " exact_output_matches=" << root.exact_output_matches
        << " shape_output_matches=" << root.shape_output_matches
        << " function=" << quote_callsem_dup_field(binding ? binding->name : string())
        << " symbol="
        << quote_callsem_dup_field(binding ?
               callsem_provenance_binding_symbol(*binding) :
               string());
    if(binding && binding->cached_body_output && root.first_shape_match) {
      out << " variation="
          << quote_callsem_dup_field(callsem_provenance_variation_mask(
                 *binding->cached_body_output,
                 *root.first_shape_match));
    }
    out << '\n';
  }
}

}  // namespace

void dump_callsem_provenance_census(ostream & out, const MemoryCensusInput & input)
{
  unordered_map<uint64_t, vector<CallSemProvenanceIndexedNode> > exact_output_index;
  unordered_map<uint64_t, vector<CallSemProvenanceIndexedNode> > shape_output_index;
  const CallSemDuplicateNodeMetrics output_metrics =
      compute_callsem_provenance_metrics(input.translation_unit,
                                         &exact_output_index,
                                         &shape_output_index);

  size_t cached_roots = 0;
  size_t cached_nodes = 0;
  size_t cached_bytes = 0;
  size_t exact_output_roots = 0;
  size_t exact_output_nodes = 0;
  size_t exact_output_bytes = 0;
  size_t shape_only_roots = 0;
  size_t shape_only_nodes = 0;
  size_t shape_only_bytes = 0;
  size_t no_output_roots = 0;
  size_t no_output_nodes = 0;
  size_t no_output_bytes = 0;
  vector<CallSemProvenanceCachedRoot> exact_roots;
  vector<CallSemProvenanceCachedRoot> shape_only_roots_detail;
  vector<CallSemProvenanceCachedRoot> no_output_roots_detail;

  for(size_t i = 0; i < input.functions.size(); ++i) {
    if(!input.functions[i] || !input.functions[i]->cached_body_output) {
      continue;
    }
    const FunctionBinding & binding = *input.functions[i];
    const CallSemNode & cached_root = *binding.cached_body_output;
    CallSemProvenanceCachedRoot root;
    root.binding = &binding;
    root.metrics =
        compute_callsem_provenance_metrics(cached_root, nullptr, nullptr);

    unordered_map<uint64_t, vector<CallSemProvenanceIndexedNode> >::const_iterator exact_it =
        exact_output_index.find(root.metrics.exact_hash);
    if(exact_it != exact_output_index.end()) {
      for(size_t j = 0; j < exact_it->second.size(); ++j) {
        if(callsem_exact_equal(cached_root, *exact_it->second[j].node)) {
          ++root.exact_output_matches;
        }
      }
    }

    unordered_map<uint64_t, vector<CallSemProvenanceIndexedNode> >::const_iterator shape_it =
        shape_output_index.find(root.metrics.shape_hash);
    if(shape_it != shape_output_index.end()) {
      for(size_t j = 0; j < shape_it->second.size(); ++j) {
        if(callsem_shape_equal(cached_root, *shape_it->second[j].node)) {
          ++root.shape_output_matches;
          if(!root.first_shape_match) {
            root.first_shape_match = shape_it->second[j].node;
          }
        }
      }
    }

    ++cached_roots;
    cached_nodes += root.metrics.subtree_nodes;
    cached_bytes += root.metrics.subtree_bytes;
    if(root.exact_output_match()) {
      ++exact_output_roots;
      exact_output_nodes += root.metrics.subtree_nodes;
      exact_output_bytes += root.metrics.subtree_bytes;
      exact_roots.push_back(root);
    } else if(root.shape_only_output_match()) {
      ++shape_only_roots;
      shape_only_nodes += root.metrics.subtree_nodes;
      shape_only_bytes += root.metrics.subtree_bytes;
      shape_only_roots_detail.push_back(root);
    } else {
      ++no_output_roots;
      no_output_nodes += root.metrics.subtree_nodes;
      no_output_bytes += root.metrics.subtree_bytes;
      no_output_roots_detail.push_back(root);
    }
  }

  out << "callsem-provenance-summary"
      << " output_nodes=" << output_metrics.subtree_nodes
      << " output_bytes=" << output_metrics.subtree_bytes
      << " cached_roots=" << cached_roots
      << " cached_nodes=" << cached_nodes
      << " cached_bytes=" << cached_bytes
      << " exact_output_roots=" << exact_output_roots
      << " exact_output_nodes=" << exact_output_nodes
      << " exact_output_bytes=" << exact_output_bytes
      << " shape_only_output_roots=" << shape_only_roots
      << " shape_only_output_nodes=" << shape_only_nodes
      << " shape_only_output_bytes=" << shape_only_bytes
      << " no_output_roots=" << no_output_roots
      << " no_output_nodes=" << no_output_nodes
      << " no_output_bytes=" << no_output_bytes
      << '\n';

  const size_t top_limit = callsem_duplicate_top_limit();
  dump_callsem_provenance_top_roots(out,
                                    exact_roots,
                                    "exact-output-match",
                                    top_limit);
  dump_callsem_provenance_top_roots(out,
                                    shape_only_roots_detail,
                                    "shape-only-output-match",
                                    top_limit);
  dump_callsem_provenance_top_roots(out,
                                    no_output_roots_detail,
                                    "no-output-match",
                                    top_limit);
}

namespace {

struct CallSemRetainedStats
{
  size_t nodes = 0;
  size_t leaf_nodes = 0;
  size_t parent_nodes = 0;
  size_t typed_nodes = 0;
  size_t extra_nodes = 0;
  size_t symbol_nodes = 0;
  size_t qualified_name_nodes = 0;
  size_t bytes = 0;
  size_t inline_bytes = 0;
  size_t children_storage = 0;
  size_t extra_inline = 0;
  size_t rare_string_inline = 0;
  size_t rare_payload_inline = 0;
  size_t text_storage = 0;
  size_t resolved_name_storage = 0;
  size_t source_file_storage = 0;
  size_t sidecar_string_storage = 0;
  size_t virtual_base_layout_storage = 0;
  size_t symbol_payload = 0;
  size_t qualified_name_syntax = 0;
};

struct CallSemRetainedKindKey
{
  string source;
  CallSemKind kind = CallSemKind::invalid;

  bool operator<(const CallSemRetainedKindKey & rhs) const
  {
    if(source != rhs.source) {
      return source < rhs.source;
    }
    return kind < rhs.kind;
  }
};

struct CallSemRetainedKindEntry
{
  CallSemRetainedKindKey key;
  CallSemRetainedStats stats;
};

void add_callsem_retained_stats(CallSemRetainedStats & total,
                                const CallSemRetainedStats & item)
{
  total.nodes += item.nodes;
  total.leaf_nodes += item.leaf_nodes;
  total.parent_nodes += item.parent_nodes;
  total.typed_nodes += item.typed_nodes;
  total.extra_nodes += item.extra_nodes;
  total.symbol_nodes += item.symbol_nodes;
  total.qualified_name_nodes += item.qualified_name_nodes;
  total.bytes += item.bytes;
  total.inline_bytes += item.inline_bytes;
  total.children_storage += item.children_storage;
  total.extra_inline += item.extra_inline;
  total.rare_string_inline += item.rare_string_inline;
  total.rare_payload_inline += item.rare_payload_inline;
  total.text_storage += item.text_storage;
  total.resolved_name_storage += item.resolved_name_storage;
  total.source_file_storage += item.source_file_storage;
  total.sidecar_string_storage += item.sidecar_string_storage;
  total.virtual_base_layout_storage += item.virtual_base_layout_storage;
  total.symbol_payload += item.symbol_payload;
  total.qualified_name_syntax += item.qualified_name_syntax;
}

CallSemRetainedStats callsem_retained_shallow_stats(
    const CallSemNode & node,
    unordered_set<const CallSemNodeExtra *> & seen_callsem_extras,
    unordered_set<const CallSemRareStrings *> & seen_callsem_rare_strings,
    unordered_set<const CallSemRarePayload *> & seen_callsem_rare_payloads,
    unordered_set<uint32_t> & seen_callsem_source_files,
    unordered_set<const symbol_linkage::SymbolIdentity *> & seen_callsem_symbols)
{
  CallSemRetainedStats stats;
  stats.nodes = 1;
  stats.leaf_nodes = node.children.empty() ? 1 : 0;
  stats.parent_nodes = node.children.empty() ? 0 : 1;
  stats.typed_nodes = node.semantic_type ? 1 : 0;
  stats.extra_nodes = node.extra ? 1 : 0;
  stats.symbol_nodes = !callsem_symbol_is_empty(callsem_symbol(node)) ? 1 : 0;
  stats.qualified_name_nodes = callsem_qualified_name_syntax(node) ? 1 : 0;
  stats.inline_bytes = sizeof(CallSemNode);
  stats.children_storage = vector_storage_bytes(node.children);
  stats.text_storage = callsem_text_storage_bytes(node.text);
  stats.resolved_name_storage = callsem_resolved_name_storage_bytes(node);
  stats.symbol_payload =
      callsem_symbol_storage_bytes(node, &seen_callsem_symbols);
  stats.qualified_name_syntax =
      qualified_name_payload_bytes(callsem_qualified_name_syntax(node));
  if(node.extra && seen_callsem_extras.insert(node.extra.get()).second) {
    stats.extra_inline = sizeof(CallSemNodeExtra);
  }
  const CallSemRarePayload * rare_payload = callsem_rare_payload(node);
  if(rare_payload &&
     seen_callsem_rare_payloads.insert(rare_payload).second) {
    stats.rare_payload_inline = sizeof(CallSemRarePayload);
    stats.virtual_base_layout_storage =
        vector_storage_bytes(rare_payload->virtual_base_layout);
    for(size_t i = 0; i < rare_payload->virtual_base_layout.size(); ++i) {
      stats.virtual_base_layout_storage +=
          string_storage_bytes(rare_payload->virtual_base_layout[i].first);
    }
  }
  if(node.extra &&
     node.extra->rare_strings &&
     seen_callsem_rare_strings.insert(node.extra->rare_strings.get()).second) {
    stats.rare_string_inline = sizeof(CallSemRareStrings);
    stats.sidecar_string_storage =
        string_storage_bytes(node.extra->rare_strings->vtt_symbol) +
        string_storage_bytes(node.extra->rare_strings->vtt_object_symbol) +
        string_storage_bytes(node.extra->rare_strings->runtime_bridge_symbol) +
        string_storage_bytes(
            node.extra->rare_strings->local_static_guard_symbol);
  }
  if(node.source_file_index != 0 &&
     seen_callsem_source_files.insert(node.source_file_index).second) {
    stats.source_file_storage = string_storage_bytes(callsem_source_file(node));
  }
  stats.bytes = stats.inline_bytes +
                stats.children_storage +
                stats.extra_inline +
                stats.rare_string_inline +
                stats.rare_payload_inline +
                stats.text_storage +
                stats.resolved_name_storage +
                stats.source_file_storage +
                stats.sidecar_string_storage +
                stats.virtual_base_layout_storage +
                stats.symbol_payload +
                stats.qualified_name_syntax;
  return stats;
}

class CallSemRetainedKindCensus
{
public:
  void add_root(const CallSemNode & node, const string & source)
  {
    ++roots_[source];
    visit(node, source);
  }

  void dump(ostream & out) const
  {
    for(map<string, CallSemRetainedStats>::const_iterator it = summaries_.begin();
        it != summaries_.end();
        ++it) {
      const size_t roots = roots_.count(it->first) ? roots_.find(it->first)->second : 0;
      dump_stats(out, "callsem-retained-summary", it->first, "", 0, roots, it->second);
    }

    vector<CallSemRetainedKindEntry> entries;
    for(map<CallSemRetainedKindKey, CallSemRetainedStats>::const_iterator it =
            by_kind_.begin();
        it != by_kind_.end();
        ++it) {
      CallSemRetainedKindEntry entry;
      entry.key = it->first;
      entry.stats = it->second;
      entries.push_back(entry);
    }
    sort(entries.begin(),
         entries.end(),
         [](const CallSemRetainedKindEntry & lhs,
            const CallSemRetainedKindEntry & rhs)
         {
           if(lhs.key.source != rhs.key.source) {
             return lhs.key.source < rhs.key.source;
           }
           if(lhs.stats.bytes != rhs.stats.bytes) {
             return lhs.stats.bytes > rhs.stats.bytes;
           }
           return lhs.key.kind < rhs.key.kind;
         });

    string previous_source;
    size_t rank = 0;
    for(size_t i = 0; i < entries.size(); ++i) {
      if(entries[i].key.source != previous_source) {
        previous_source = entries[i].key.source;
        rank = 0;
      }
      ++rank;
      dump_stats(out,
                 "callsem-retained-kind",
                 entries[i].key.source,
                 callsem_kind_text(entries[i].key.kind),
                 rank,
                 0,
                 entries[i].stats);
    }
  }

private:
  void visit(const CallSemNode & node, const string & source)
  {
    const CallSemRetainedStats stats =
        callsem_retained_shallow_stats(node,
                                       seen_callsem_extras_,
                                       seen_callsem_rare_strings_,
                                       seen_callsem_rare_payloads_,
                                       seen_callsem_source_files_,
                                       seen_callsem_symbols_);
    add_callsem_retained_stats(summaries_[source], stats);

    CallSemRetainedKindKey key;
    key.source = source;
    key.kind = node.kind;
    add_callsem_retained_stats(by_kind_[key], stats);

    const shared_ptr<CallSemNode> & lowered = callsem_lowered_condition_test(node);
    if(lowered) {
      visit(*lowered, source);
    }
    for(size_t i = 0; i < node.children.size(); ++i) {
      visit(node.children[i], source);
    }
  }

  static void dump_stats(ostream & out,
                         const string & prefix,
                         const string & source,
                         const string & kind,
                         size_t rank,
                         size_t roots,
                         const CallSemRetainedStats & stats)
  {
    out << prefix;
    if(rank != 0) {
      out << " rank=" << rank;
    }
    out << " source=" << source;
    if(!kind.empty()) {
      out << " kind=" << kind;
    }
    if(roots != 0) {
      out << " roots=" << roots;
    }
    out << " nodes=" << stats.nodes
        << " leaves=" << stats.leaf_nodes
        << " parents=" << stats.parent_nodes
        << " typed=" << stats.typed_nodes
        << " extra_nodes=" << stats.extra_nodes
        << " symbol_nodes=" << stats.symbol_nodes
        << " qualified_name_nodes=" << stats.qualified_name_nodes
        << " bytes=" << stats.bytes
        << " inline=" << stats.inline_bytes
        << " children_storage=" << stats.children_storage
        << " extra_inline=" << stats.extra_inline
        << " rare_string_inline=" << stats.rare_string_inline
        << " rare_payload_inline=" << stats.rare_payload_inline
        << " text_storage=" << stats.text_storage
        << " resolved_name_storage=" << stats.resolved_name_storage
        << " source_file_storage=" << stats.source_file_storage
        << " sidecar_string_storage=" << stats.sidecar_string_storage
        << " virtual_base_layout_storage=" << stats.virtual_base_layout_storage
        << " symbol_payload=" << stats.symbol_payload
        << " qualified_name_syntax=" << stats.qualified_name_syntax
        << '\n';
  }

  map<string, size_t> roots_;
  map<string, CallSemRetainedStats> summaries_;
  map<CallSemRetainedKindKey, CallSemRetainedStats> by_kind_;
  unordered_set<const CallSemNodeExtra *> seen_callsem_extras_;
  unordered_set<const CallSemRareStrings *> seen_callsem_rare_strings_;
  unordered_set<const CallSemRarePayload *> seen_callsem_rare_payloads_;
  unordered_set<uint32_t> seen_callsem_source_files_;
  unordered_set<const symbol_linkage::SymbolIdentity *> seen_callsem_symbols_;
};

}  // namespace

void dump_callsem_retained_kind_census(ostream & out,
                                       const MemoryCensusInput & input)
{
  CallSemRetainedKindCensus census;
  census.add_root(input.translation_unit, "output");
  for(size_t i = 0; i < input.functions.size(); ++i) {
    if(input.functions[i] && input.functions[i]->cached_body_output) {
      census.add_root(*input.functions[i]->cached_body_output, "cached");
    }
  }
  census.dump(out);
}

}  // namespace callsemantic

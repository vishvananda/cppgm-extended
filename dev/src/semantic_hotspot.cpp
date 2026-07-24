#include "semantic_hotspot.h"

#include <algorithm>
#include <cstdlib>
#include <cstdint>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include "cpp_decl_bridge.h"
#include "cppast_ast.h"
#include "semantic_trace.h"

namespace semantic_hotspot {

namespace {

enum NodeDomain
{
  ND_EXPRESSION,
  ND_CONSTANT_EVAL
};

struct NodeKey
{
  NodeKey() : node(nullptr), domain(ND_EXPRESSION) {}
  NodeKey(const CppAstNode * node, NodeDomain domain)
      : node(node), domain(domain)
  {}

  const CppAstNode * node;
  NodeDomain domain;

  bool operator==(const NodeKey & other) const
  {
    return node == other.node && domain == other.domain;
  }
};

struct NodeKeyHash
{
  std::size_t operator()(const NodeKey & key) const
  {
    return (reinterpret_cast<std::uintptr_t>(key.node) >> 4) ^
           (static_cast<std::size_t>(key.domain) << 1);
  }
};

struct NodeRecord
{
  std::size_t count = 0;
  std::size_t text_len = 0;
  std::string domain;
  std::string kind;
  std::string location;
  std::string frame;
  std::string preview;
};

struct FragmentKey
{
  std::string kind;
  std::string text;

  bool operator==(const FragmentKey & other) const
  {
    return kind == other.kind &&
           text == other.text;
  }
};

struct FragmentKeyHash
{
  std::size_t operator()(const FragmentKey & key) const
  {
    std::size_t out = std::hash<std::string>()(key.kind);
    out ^= std::hash<std::string>()(key.text) + 0x9e3779b9 + (out << 6) + (out >> 2);
    return out;
  }
};

struct FragmentRecord
{
  std::size_t count = 0;
  std::size_t total_chars = 0;
  std::unordered_map<std::string, std::size_t> frames;
};

struct QueryKey
{
  std::string kind;
  std::string text;

  bool operator==(const QueryKey & other) const
  {
    return kind == other.kind && text == other.text;
  }
};

struct QueryKeyHash
{
  std::size_t operator()(const QueryKey & key) const
  {
    std::size_t out = std::hash<std::string>()(key.kind);
    out ^= std::hash<std::string>()(key.text) + 0x9e3779b9 + (out << 6) + (out >> 2);
    return out;
  }
};

struct QueryRecord
{
  std::size_t count = 0;
  std::size_t total_chars = 0;
  std::unordered_map<std::string, std::size_t> frames;
};

std::string preview_text(const std::string & text)
{
  static const std::size_t kMaxPreview = 120;
  if(text.size() <= kMaxPreview) {
    return text;
  }
  return text.substr(0, kMaxPreview - 3) + "...";
}

const char * node_domain_text(NodeDomain domain)
{
  switch(domain) {
  case ND_EXPRESSION:
    return "expr";
  case ND_CONSTANT_EVAL:
    return "constexpr";
  }
  return "unknown";
}

bool env_enabled()
{
  static const bool enabled = []()
  {
    const char * value = std::getenv("CPPGM_SEMANTIC_HOTSPOT");
    return value && *value && std::string(value) != "0";
  }();
  return enabled;
}

bool collect_enabled()
{
  return env_enabled();
}

const std::string & fragment_trace_filter()
{
  static const std::string filter = []() -> std::string
  {
    const char * value = std::getenv("CPPGM_SEMANTIC_HOTSPOT_TRACE_FRAGMENT");
    return value ? std::string(value) : std::string();
  }();
  return filter;
}

const std::string & node_trace_filter()
{
  static const std::string filter = []() -> std::string
  {
    const char * value = std::getenv("CPPGM_SEMANTIC_HOTSPOT_TRACE_NODE");
    return value ? std::string(value) : std::string();
  }();
  return filter;
}

const std::string & query_trace_filter()
{
  static const std::string filter = []() -> std::string
  {
    const char * value = std::getenv("CPPGM_SEMANTIC_HOTSPOT_TRACE_QUERY");
    return value ? std::string(value) : std::string();
  }();
  return filter;
}

const std::string & query_dump_filter()
{
  static const std::string filter = []() -> std::string
  {
    const char * value = std::getenv("CPPGM_SEMANTIC_HOTSPOT_DUMP_QUERY");
    return value ? std::string(value) : std::string();
  }();
  return filter;
}

const std::string & fragment_dump_filter()
{
  static const std::string filter = []() -> std::string
  {
    const char * value = std::getenv("CPPGM_SEMANTIC_HOTSPOT_DUMP_FRAGMENT");
    return value ? std::string(value) : std::string();
  }();
  return filter;
}

std::size_t fragment_trace_limit()
{
  static const std::size_t limit = []() -> std::size_t
  {
    const char * value = std::getenv("CPPGM_SEMANTIC_HOTSPOT_TRACE_LIMIT");
    if(value == nullptr || *value == '\0') {
      return 8;
    }
    const long parsed = std::strtol(value, nullptr, 10);
    return parsed > 0 ? static_cast<std::size_t>(parsed) : 8;
  }();
  return limit;
}

std::size_t dump_limit()
{
  static const std::size_t limit = []() -> std::size_t
  {
    const char * value = std::getenv("CPPGM_SEMANTIC_HOTSPOT_DUMP_LIMIT");
    if(value == nullptr || *value == '\0') {
      return 32;
    }
    const long parsed = std::strtol(value, nullptr, 10);
    return parsed > 0 ? static_cast<std::size_t>(parsed) : 32;
  }();
  return limit;
}

bool matches_filter(const std::string & filter,
                    const std::string & kind,
                    const std::string & text)
{
  if(filter == "*") {
    return true;
  }
  return !filter.empty() &&
         (kind.find(filter) != std::string::npos ||
          text.find(filter) != std::string::npos);
}

template <class FrameMap>
std::string top_frame_summary(const FrameMap & frames,
                              std::size_t limit)
{
  if(frames.empty() || limit == 0) {
    return std::string();
  }

  std::vector<typename FrameMap::const_iterator> rows;
  rows.reserve(frames.size());
  for(typename FrameMap::const_iterator it = frames.begin();
      it != frames.end();
      ++it) {
    rows.push_back(it);
  }
  std::sort(rows.begin(),
            rows.end(),
            [](typename FrameMap::const_iterator a,
               typename FrameMap::const_iterator b)
            {
              if(a->second != b->second) {
                return a->second > b->second;
              }
              return a->first < b->first;
            });

  const std::size_t count = std::min<std::size_t>(limit, rows.size());
  std::ostringstream out;
  for(std::size_t i = 0; i < count; ++i) {
    if(i != 0) {
      out << " | ";
    }
    out << rows[i]->second << "@" << rows[i]->first;
  }
  return out.str();
}

struct Profiler
{
  std::unordered_map<NodeKey, NodeRecord, NodeKeyHash> nodes;
  std::unordered_map<FragmentKey, FragmentRecord, FragmentKeyHash> fragments;
  std::unordered_map<QueryKey, QueryRecord, QueryKeyHash> queries;
  std::size_t emitted_node_traces = 0;
  std::size_t emitted_fragment_traces = 0;
  std::size_t emitted_query_traces = 0;
  std::size_t total_node_visits = 0;
  std::size_t total_query_requests = 0;
  std::size_t total_fragment_requests = 0;
  std::size_t total_fragment_request_chars = 0;

  ~Profiler()
  {
    if(!env_enabled()) {
      return;
    }

    std::vector<const std::pair<const NodeKey, NodeRecord> *> node_rows;
    node_rows.reserve(nodes.size());
    for(const auto & entry : nodes) {
      if(entry.second.count >= 4) {
        node_rows.push_back(&entry);
      }
    }
    std::sort(node_rows.begin(),
              node_rows.end(),
              [](const std::pair<const NodeKey, NodeRecord> * lhs,
                 const std::pair<const NodeKey, NodeRecord> * rhs)
              {
                const std::size_t lhs_work = lhs->second.count * lhs->second.text_len;
                const std::size_t rhs_work = rhs->second.count * rhs->second.text_len;
                if(lhs_work != rhs_work) {
                  return lhs_work > rhs_work;
                }
                return lhs->second.count > rhs->second.count;
              });

    std::vector<const std::pair<const FragmentKey, FragmentRecord> *> fragment_rows;
    fragment_rows.reserve(fragments.size());
    for(const auto & entry : fragments) {
      if(entry.second.count >= 4 || entry.second.total_chars >= 1024) {
        fragment_rows.push_back(&entry);
      }
    }
    std::sort(fragment_rows.begin(),
              fragment_rows.end(),
              [](const std::pair<const FragmentKey, FragmentRecord> * lhs,
                 const std::pair<const FragmentKey, FragmentRecord> * rhs)
              {
                if(lhs->second.total_chars != rhs->second.total_chars) {
                  return lhs->second.total_chars > rhs->second.total_chars;
                }
                return lhs->second.count > rhs->second.count;
              });

    std::vector<const std::pair<const QueryKey, QueryRecord> *> query_rows;
    query_rows.reserve(queries.size());
    for(const auto & entry : queries) {
      if(entry.second.count >= 4 || entry.second.total_chars >= 256) {
        query_rows.push_back(&entry);
      }
    }
    std::sort(query_rows.begin(),
              query_rows.end(),
              [](const std::pair<const QueryKey, QueryRecord> * lhs,
                 const std::pair<const QueryKey, QueryRecord> * rhs)
              {
                if(lhs->second.count != rhs->second.count) {
                  return lhs->second.count > rhs->second.count;
                }
                return lhs->second.total_chars > rhs->second.total_chars;
              });

    std::cerr << "SEMANTIC_HOTSPOT summary"
              << " node_visits=" << total_node_visits
              << " unique_nodes=" << nodes.size()
              << " query_requests=" << total_query_requests
              << " fragment_requests=" << total_fragment_requests
              << " fragment_request_chars=" << total_fragment_request_chars
              << " unique_fragments=" << fragments.size()
              << '\n';

    if(!node_rows.empty()) {
      std::cerr << "SEMANTIC_HOTSPOT repeated_nodes\n";
      const std::size_t limit = std::min<std::size_t>(12, node_rows.size());
      for(std::size_t i = 0; i < limit; ++i) {
        const NodeRecord & row = node_rows[i]->second;
        std::cerr << "  count=" << row.count
                  << " work=" << (row.count * row.text_len)
                  << " domain=" << row.domain
                  << " kind=" << row.kind
                  << " loc=" << (row.location.empty() ? "<none>" : row.location)
                  << " frame=" << (row.frame.empty() ? "<none>" : row.frame)
                  << " text=" << row.preview
                  << '\n';
      }
    }

    if(!fragment_rows.empty()) {
      std::cerr << "SEMANTIC_HOTSPOT fragment_requests\n";
      const std::size_t limit = std::min<std::size_t>(8, fragment_rows.size());
      for(std::size_t i = 0; i < limit; ++i) {
        const FragmentKey & key = fragment_rows[i]->first;
        const FragmentRecord & row = fragment_rows[i]->second;
        std::cerr << "  count=" << row.count
                  << " chars=" << row.total_chars
                  << " kind=" << key.kind
                  << " len=" << key.text.size()
                  << " text=" << preview_text(key.text)
                  << '\n';

        std::vector<std::pair<std::string, std::size_t> > frames(row.frames.begin(),
                                                                 row.frames.end());
        std::sort(frames.begin(),
                  frames.end(),
                  [](const std::pair<std::string, std::size_t> & lhs,
                     const std::pair<std::string, std::size_t> & rhs)
                  {
                    if(lhs.second != rhs.second) {
                      return lhs.second > rhs.second;
                    }
                    return lhs.first < rhs.first;
                  });
        const std::size_t frame_limit = std::min<std::size_t>(4, frames.size());
        for(std::size_t j = 0; j < frame_limit; ++j) {
          std::cerr << "    frame_count=" << frames[j].second
                    << " frame=" << (frames[j].first.empty() ? "<none>" : frames[j].first)
                    << '\n';
        }
      }
    }

    if(!query_rows.empty()) {
      std::cerr << "SEMANTIC_HOTSPOT semantic_queries\n";
      const std::size_t limit = std::min<std::size_t>(12, query_rows.size());
      for(std::size_t i = 0; i < limit; ++i) {
        const QueryKey & key = query_rows[i]->first;
        const QueryRecord & row = query_rows[i]->second;
        std::cerr << "  count=" << row.count
                  << " chars=" << row.total_chars
                  << " kind=" << key.kind
                  << " len=" << key.text.size()
                  << " text=" << preview_text(key.text)
                  << '\n';

        std::vector<std::pair<std::string, std::size_t> > frames(row.frames.begin(),
                                                                 row.frames.end());
        std::sort(frames.begin(),
                  frames.end(),
                  [](const std::pair<std::string, std::size_t> & lhs,
                     const std::pair<std::string, std::size_t> & rhs)
                  {
                    if(lhs.second != rhs.second) {
                      return lhs.second > rhs.second;
                    }
                    return lhs.first < rhs.first;
                  });
        const std::size_t frame_limit = std::min<std::size_t>(4, frames.size());
        for(std::size_t j = 0; j < frame_limit; ++j) {
          std::cerr << "    frame_count=" << frames[j].second
                    << " frame=" << (frames[j].first.empty() ? "<none>" : frames[j].first)
                    << '\n';
        }
      }
    }

    const std::string & active_query_dump_filter = query_dump_filter();
    if(!active_query_dump_filter.empty()) {
      std::vector<const std::pair<const QueryKey, QueryRecord> *> filtered_rows;
      filtered_rows.reserve(query_rows.size());
      for(std::size_t i = 0; i < query_rows.size(); ++i) {
        if(matches_filter(active_query_dump_filter,
                          query_rows[i]->first.kind,
                          query_rows[i]->first.text)) {
          filtered_rows.push_back(query_rows[i]);
        }
      }
      if(!filtered_rows.empty()) {
        std::cerr << "SEMANTIC_HOTSPOT query_dump"
                  << " filter=" << active_query_dump_filter
                  << " matches=" << filtered_rows.size()
                  << '\n';
        const std::size_t limit = std::min<std::size_t>(dump_limit(), filtered_rows.size());
        for(std::size_t i = 0; i < limit; ++i) {
          const QueryKey & key = filtered_rows[i]->first;
          const QueryRecord & row = filtered_rows[i]->second;
          std::cerr << "  count=" << row.count
                    << " chars=" << row.total_chars
                    << " kind=" << key.kind
                    << " len=" << key.text.size()
                    << " text=" << key.text;
          const std::string frames_text = top_frame_summary(row.frames, 3);
          if(!frames_text.empty()) {
            std::cerr << " frames=" << frames_text;
          }
          std::cerr
                    << '\n';
        }
      }
    }

    const std::string & active_fragment_dump_filter = fragment_dump_filter();
    if(!active_fragment_dump_filter.empty()) {
      std::vector<const std::pair<const FragmentKey, FragmentRecord> *> filtered_rows;
      filtered_rows.reserve(fragment_rows.size());
      for(std::size_t i = 0; i < fragment_rows.size(); ++i) {
        if(matches_filter(active_fragment_dump_filter,
                          fragment_rows[i]->first.kind,
                          fragment_rows[i]->first.text)) {
          filtered_rows.push_back(fragment_rows[i]);
        }
      }
      if(!filtered_rows.empty()) {
        std::cerr << "SEMANTIC_HOTSPOT fragment_dump"
                  << " filter=" << active_fragment_dump_filter
                  << " matches=" << filtered_rows.size()
                  << '\n';
        const std::size_t limit = std::min<std::size_t>(dump_limit(), filtered_rows.size());
        for(std::size_t i = 0; i < limit; ++i) {
          const FragmentKey & key = filtered_rows[i]->first;
          const FragmentRecord & row = filtered_rows[i]->second;
          std::cerr << "  count=" << row.count
                    << " chars=" << row.total_chars
                    << " kind=" << key.kind
                    << " len=" << key.text.size()
                    << " text=" << key.text;
          const std::string frames_text = top_frame_summary(row.frames, 3);
          if(!frames_text.empty()) {
            std::cerr << " frames=" << frames_text;
          }
          std::cerr
                    << '\n';
        }
      }
    }
  }
};

Profiler & profiler();

void note_fragment(const char * kind,
                   const std::string & text)
{
  if(!collect_enabled()) {
    return;
  }

  FragmentKey key;
  key.kind = kind ? kind : "fragment";
  key.text = text;
  FragmentRecord & record = profiler().fragments[key];
  ++record.count;
  record.total_chars += text.size();
  ++record.frames[DiagnosticContext::current_frame()];

  const std::string & trace_filter = fragment_trace_filter();
  if(!trace_filter.empty() &&
     text.find(trace_filter) != std::string::npos &&
     profiler().emitted_fragment_traces < fragment_trace_limit()) {
    ++profiler().emitted_fragment_traces;
    std::cerr << "SEMANTIC_HOTSPOT fragment_trace"
              << " count=" << record.count
              << " kind=" << key.kind
              << " text=" << text
              << '\n'
              << DiagnosticContext::format_stack()
              << '\n';
  }

  ++profiler().total_fragment_requests;
  profiler().total_fragment_request_chars += text.size();
}

Profiler & profiler()
{
  static Profiler instance;
  return instance;
}

void note_node_visit(NodeDomain domain,
                     const CppAstNode & node,
                     const std::string & location)
{
  if(!collect_enabled()) {
    return;
  }

  NodeRecord & record = profiler().nodes[NodeKey{&node, domain}];
  if(record.count == 0) {
    const std::string text = cpp_decl::node_text(node);
    record.text_len = text.size();
    record.domain = node_domain_text(domain);
    record.kind = cppast_kind_text(node.kind);
    record.location = location;
    record.frame = DiagnosticContext::current_frame();
    record.preview = preview_text(text);
  }
  ++record.count;
  const std::string & trace_filter = node_trace_filter();
  if(!trace_filter.empty() &&
     record.count > 1 &&
     (record.location.find(trace_filter) != std::string::npos ||
      record.preview.find(trace_filter) != std::string::npos ||
      record.kind.find(trace_filter) != std::string::npos) &&
     profiler().emitted_node_traces < fragment_trace_limit()) {
    ++profiler().emitted_node_traces;
    std::cerr << "SEMANTIC_HOTSPOT node_trace"
              << " count=" << record.count
              << " domain=" << record.domain
              << " kind=" << record.kind
              << " loc=" << (record.location.empty() ? "<none>" : record.location)
              << " text=" << record.preview
              << '\n'
              << DiagnosticContext::format_stack()
              << '\n';
  }
  ++profiler().total_node_visits;
}

}  // namespace

bool enabled()
{
  return collect_enabled();
}

void note_expression_analysis(const CppAstNode & node,
                              const std::string & location)
{
  note_node_visit(ND_EXPRESSION, node, location);
}

void note_constant_evaluation(const CppAstNode & node,
                              const std::string & location)
{
  note_node_visit(ND_CONSTANT_EVAL, node, location);
}

void note_semantic_query(const char * kind,
                         const std::string & text)
{
  if(!collect_enabled()) {
    return;
  }

  QueryKey key;
  key.kind = kind ? kind : "query";
  key.text = text;
  QueryRecord & record = profiler().queries[key];
  ++record.count;
  record.total_chars += text.size();
  const std::string current_frame = DiagnosticContext::current_frame();
  ++record.frames[current_frame];
  ++profiler().total_query_requests;

  const std::string & trace_filter = query_trace_filter();
  if(!trace_filter.empty() &&
     (text.find(trace_filter) != std::string::npos ||
      current_frame.find(trace_filter) != std::string::npos) &&
     profiler().emitted_query_traces < fragment_trace_limit()) {
    ++profiler().emitted_query_traces;
    std::cerr << "SEMANTIC_HOTSPOT query_trace"
              << " count=" << record.count
              << " kind=" << key.kind
              << " text=" << text
              << '\n'
              << DiagnosticContext::format_stack()
              << '\n';
  }
}

void note_fragment_request(const char * kind,
                           const std::string & text)
{
  note_fragment(kind, text);
}

}  // namespace semantic_hotspot

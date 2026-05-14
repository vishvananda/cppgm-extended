#include "parser_trace.h"

#include <cstdlib>
#include <deque>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace parser_trace {

namespace {

const char * const kSuppressedUseLocationMarker = "\x1d";

struct Event
{
  std::string category;
  std::string location;
  std::string message;
};

thread_local std::deque<Event> events_;

struct Config
{
  std::vector<std::string> categories;
  std::string file_filter;
  std::string symbol_filter;
  std::size_t limit = 256;
  bool live = false;
  bool on_error = false;
  bool any_categories = false;
};

thread_local std::vector<std::string> use_locations_;
thread_local std::vector<const std::string *> order_use_locations_;

bool category_matches(const std::string & configured,
                      const char * category)
{
  if(configured == "all" || configured == category) {
    return true;
  }

  const std::size_t dot = configured.find(".*");
  if(dot != std::string::npos &&
     std::string(category).compare(0, dot, configured, 0, dot) == 0) {
    return true;
  }

  return false;
}

bool truthy_value(const char * value)
{
  return value && *value && std::string(value) != "0";
}

void parse_categories(const std::string & text,
                      std::vector<std::string> & out)
{
  std::string configured;
  for(std::size_t i = 0; i <= text.size(); ++i) {
    const char ch = i == text.size() ? '\0' : text[i];
    if(ch == ',' || ch == '\0') {
      if(!configured.empty()) {
        out.push_back(configured);
      }
      configured.clear();
      continue;
    }
    if(ch != ' ' && ch != '\t') {
      configured += ch;
    }
  }
}

Config load_config()
{
  Config cfg;

  const char * categories = std::getenv("CPPGM_TRACE");
  if(categories && *categories) {
    cfg.any_categories = true;
    parse_categories(categories, cfg.categories);
  }

  const char * file_filter = std::getenv("CPPGM_TRACE_FILE");
  if(file_filter && *file_filter) {
    cfg.file_filter = file_filter;
  }

  const char * symbol_filter = std::getenv("CPPGM_TRACE_SYMBOL");
  if(symbol_filter && *symbol_filter) {
    cfg.symbol_filter = symbol_filter;
  }

  const char * limit_value = std::getenv("CPPGM_TRACE_LIMIT");
  if(limit_value && *limit_value) {
    const long parsed = std::strtol(limit_value, nullptr, 10);
    if(parsed > 0) {
      cfg.limit = static_cast<std::size_t>(parsed);
    }
  }

  cfg.live = truthy_value(std::getenv("CPPGM_TRACE_LIVE"));
  cfg.on_error = truthy_value(std::getenv("CPPGM_TRACE_ON_ERROR"));
  return cfg;
}

const Config & config()
{
  static const Config cfg = load_config();
  return cfg;
}

std::string location_for(const IRecogTokenSequence & tokens, std::size_t pos)
{
  const SourceLocationTable * table = tokens.source_locations();
  if(!table) {
    return std::string();
  }
  const RecogToken & token = tokens.peek(pos);
  if(token.location_id == 0) {
    return std::string();
  }
  const std::string described = table->describe(token.location_id);
  return described == "<unknown>" ? std::string() : described;
}

bool category_enabled(const Config & cfg, const char * category)
{
  if(!cfg.any_categories) {
    return false;
  }

  for(std::size_t i = 0; i < cfg.categories.size(); ++i) {
    if(category_matches(cfg.categories[i], category)) {
      return true;
    }
  }
  return false;
}

bool passes_filters(const Config & cfg,
                    const char * category,
                    const std::string & location,
                    const std::string & message)
{
  if(!category_enabled(cfg, category)) {
    return false;
  }

  if(!cfg.file_filter.empty() &&
     !location.empty() &&
     location.find(cfg.file_filter) == std::string::npos) {
    return false;
  }

  if(!cfg.symbol_filter.empty() &&
     message.find(cfg.symbol_filter) == std::string::npos) {
    return false;
  }

  return true;
}

}  // namespace

bool enabled(const char * category)
{
  return category_enabled(config(), category);
}

void note(const char * category,
          const IRecogTokenSequence & tokens,
          std::size_t pos,
          const std::string & message)
{
  const Config & cfg = config();
  if(!category_enabled(cfg, category)) {
    return;
  }
  const std::string location = location_for(tokens, pos);
  if(!passes_filters(cfg, category, location, message)) {
    return;
  }
  note(category, location, message);
}

void note(const char * category,
          const std::string & location,
          const std::string & message)
{
  const Config & cfg = config();
  const bool trace_enabled = passes_filters(cfg, category, location, message);
  if(!trace_enabled) {
    return;
  }

  events_.push_back(Event{category, location, message});
  if(events_.size() > cfg.limit) {
    events_.pop_front();
  }

  if(cfg.live) {
    std::cerr << '[' << category << ']';
    if(!location.empty()) {
      std::cerr << ' ' << location;
    }
    std::cerr << ' ' << message << '\n';
  }
}

void push_use_location(const std::string & location)
{
  use_locations_.push_back(location);
}

void pop_use_location()
{
  if(!use_locations_.empty()) {
    use_locations_.pop_back();
  }
}

bool use_location_suppressed()
{
  for(std::size_t i = use_locations_.size(); i > 0; --i) {
    if(use_locations_[i - 1] == kSuppressedUseLocationMarker) {
      return true;
    }
  }
  return false;
}

std::string current_use_location()
{
  if(use_location_suppressed()) {
    return std::string();
  }
  for(std::size_t i = use_locations_.size(); i > 0; --i) {
    if(!use_locations_[i - 1].empty()) {
      return use_locations_[i - 1];
    }
  }
  return std::string();
}

std::string current_order_use_location()
{
  for(std::size_t i = 0; i < order_use_locations_.size(); ++i) {
    const std::string * location = order_use_locations_[i];
    if(location && !location->empty()) {
      return *location;
    }
  }
  return std::string();
}

ScopedOrderUseLocation::ScopedOrderUseLocation(std::string location)
  : location_(std::move(location)),
    active_(!location_.empty())
{
  if(active_) {
    order_use_locations_.push_back(&location_);
  }
}

ScopedOrderUseLocation::~ScopedOrderUseLocation()
{
  if(active_ && !order_use_locations_.empty()) {
    order_use_locations_.pop_back();
  }
}

std::string dump()
{
  std::ostringstream out;
  for(std::size_t i = 0; i < events_.size(); ++i) {
    if(i != 0) {
      out << '\n';
    }
    out << '[' << events_[i].category << ']';
    if(!events_[i].location.empty()) {
      out << ' ' << events_[i].location;
    }
    out << ' ' << events_[i].message;
  }
  return out.str();
}

void append_to_error(std::string & error)
{
  if(!config().on_error) {
    return;
  }
  const std::string traced = dump();
  if(traced.empty()) {
    return;
  }
  error += "\nParser trace:\n";
  error += traced;
}

}  // namespace parser_trace

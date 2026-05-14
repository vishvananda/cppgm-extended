#include "file_timing.h"

#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace file_timing {

namespace {

struct Entry
{
  std::string category;
  std::string file;
  long long total_us = 0;
  std::size_t count = 0;
  long long next_live_report_us = 0;
};

struct Config
{
  bool enabled = false;
  bool live = false;
  std::string filter;
  std::size_t limit = 20;
  long long live_step_us = 500000;
};

std::vector<Entry> entries_;

long long monotonic_time_us()
{
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return static_cast<long long>(ts.tv_sec) * 1000000LL +
         static_cast<long long>(ts.tv_nsec / 1000);
}

const long long process_start_us_ = monotonic_time_us();
long long startup_last_mark_us_ = process_start_us_;

bool truthy(const char * value)
{
  return value && *value && std::string(value) != "0";
}

bool startup_config_enabled()
{
  return truthy(std::getenv("CPPGM_STARTUP_PROFILE")) ||
         truthy(std::getenv("CPPGM_STARTUP_TIMING"));
}

void write_startup_line(const char * label, long long now_us, long long delta_us)
{
  std::ostringstream line;
  line.setf(std::ios::fixed);
  line.precision(3);
  line << "[startup.timing] " << label
       << " ms=" << (static_cast<double>(now_us - process_start_us_) / 1000.0)
       << " delta_ms=" << (static_cast<double>(delta_us) / 1000.0);
  std::cerr << line.str() << '\n';
}

std::string file_from_location(const std::string & location)
{
  std::string cleaned = location;
  if(cleaned.compare(0, 4, " at ") == 0) {
    cleaned.erase(0, 4);
  }

  if(cleaned.empty() || cleaned[0] == '<') {
    return std::string();
  }

  std::size_t last = cleaned.rfind(':');
  if(last == std::string::npos) {
    return cleaned;
  }
  std::size_t second_last = cleaned.rfind(':', last - 1);
  if(second_last == std::string::npos) {
    return cleaned.substr(0, last);
  }
  return cleaned.substr(0, second_last);
}

Config load_config()
{
  Config cfg;
  cfg.enabled = truthy(std::getenv("CPPGM_FILE_TIMING"));
  cfg.live = truthy(std::getenv("CPPGM_FILE_TIMING_LIVE"));

  const char * filter = std::getenv("CPPGM_FILE_TIMING_FILTER");
  if(filter && *filter) {
    cfg.filter = filter;
  }

  const char * limit = std::getenv("CPPGM_FILE_TIMING_LIMIT");
  if(limit && *limit) {
    const long parsed = std::strtol(limit, nullptr, 10);
    if(parsed > 0) {
      cfg.limit = static_cast<std::size_t>(parsed);
    }
  }

  const char * step_ms = std::getenv("CPPGM_FILE_TIMING_STEP_MS");
  if(step_ms && *step_ms) {
    const long parsed = std::strtol(step_ms, nullptr, 10);
    if(parsed > 0) {
      cfg.live_step_us = static_cast<long long>(parsed) * 1000LL;
    }
  }

  return cfg;
}

const Config & config()
{
  static const Config cfg = load_config();
  return cfg;
}

bool passes_filter(const std::string & file)
{
  const Config & cfg = config();
  return cfg.filter.empty() || file.find(cfg.filter) != std::string::npos;
}

void dump_summary()
{
  const Config & cfg = config();
  if(!cfg.enabled || entries_.empty()) {
    return;
  }

  std::vector<Entry> sorted = entries_;
  std::sort(sorted.begin(), sorted.end(),
            [](const Entry & lhs, const Entry & rhs)
            {
              if(lhs.total_us != rhs.total_us) {
                return lhs.total_us > rhs.total_us;
              }
              if(lhs.category != rhs.category) {
                return lhs.category < rhs.category;
              }
              return lhs.file < rhs.file;
            });

  std::cerr << "[file.timing.summary] top " << cfg.limit << '\n';
  for(std::size_t i = 0; i < sorted.size() && i < cfg.limit; ++i) {
    std::ostringstream line;
    line.setf(std::ios::fixed);
    line.precision(3);
    line << "[file.timing.summary] " << sorted[i].category
         << " total_ms=" << (static_cast<double>(sorted[i].total_us) / 1000.0)
         << " count=" << sorted[i].count
         << " file=" << sorted[i].file;
    std::cerr << line.str() << '\n';
  }
}

struct DumpRegistrar
{
  DumpRegistrar()
  {
    if(config().enabled) {
      std::atexit(dump_summary);
    }
  }
} dump_registrar_;

void record_elapsed(const char * category,
                    const std::string & location,
                    long long elapsed_us)
{
  const Config & cfg = config();
  if(!cfg.enabled || elapsed_us <= 0) {
    return;
  }

  const std::string file = file_from_location(location);
  if(file.empty() || !passes_filter(file)) {
    return;
  }

  Entry * entry = nullptr;
  for(std::size_t i = 0; i < entries_.size(); ++i) {
    if(entries_[i].category == category && entries_[i].file == file) {
      entry = &entries_[i];
      break;
    }
  }
  if(!entry) {
    Entry created;
    created.category = category;
    created.file = file;
    entries_.push_back(created);
    entries_.back().next_live_report_us = cfg.live_step_us;
    entry = &entries_.back();
  }

  entry->total_us += elapsed_us;
  ++entry->count;

  if(cfg.live && entry->total_us >= entry->next_live_report_us) {
    std::ostringstream line;
    line.setf(std::ios::fixed);
    line.precision(3);
    line << "[file.timing] " << entry->category
         << " total_ms=" << (static_cast<double>(entry->total_us) / 1000.0)
         << " count=" << entry->count
         << " file=" << entry->file;
    std::cerr << line.str() << '\n';
    entry->next_live_report_us += cfg.live_step_us;
  }
}

}  // namespace

bool enabled()
{
  return config().enabled;
}

bool startup_enabled()
{
  static const bool enabled = startup_config_enabled();
  return enabled;
}

void startup_mark(const char * label)
{
  if(!startup_enabled()) {
    return;
  }
  const long long now_us = monotonic_time_us();
  write_startup_line(label, now_us, now_us - startup_last_mark_us_);
  startup_last_mark_us_ = now_us;
}

ScopedTimer::ScopedTimer(const char * category, const std::string & location) :
  category_(category),
  location_(location),
  active_(config().enabled),
  start_us_(0)
{
  if(active_) {
    start_us_ = monotonic_time_us();
  }
}

ScopedTimer::~ScopedTimer()
{
  if(active_) {
    record_elapsed(category_, location_, monotonic_time_us() - start_us_);
  }
}

}  // namespace file_timing

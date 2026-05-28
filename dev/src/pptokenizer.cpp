#ifdef CPPGM_PROFILE_PEEK_ENABLED
#include <array>
#include <cstdio>
#include <cstdlib>
#endif
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <algorithm>

using namespace std;

#include "encoding.h"
#include "pptokenizer.h"

enum PeekSite
{
  PS_OTHER = 0,
  PS_TRANSLATE_UTF8,
  PS_TRANSLATE_TRIGRAPH,
  PS_CONSUME_PP_NUMBER,
  PS_MATCH_HEADER_NAME,
  PS_MATCH_OP_OR_PUNC,
  PS_MATCH_WHITESPACE,
  PS_GET_REGULAR_STRING,
  PS_GET_STRINGLIKE,
  PS_RAW_STRING,
  PS_COUNT
};

#ifdef CPPGM_PROFILE_PEEK_ENABLED
struct PeekStats
{
  PeekStats()
    : enabled(getenv("CPPGM_PROFILE_PEEK") != NULL),
      total_calls(0),
      miss_calls(0),
      fill_steps(0),
      max_n(0),
      large_n_calls(0),
      current_site(PS_OTHER)
  {
    n_hist.fill(0);
    site_calls.fill(0);
  }

  ~PeekStats()
  {
    if(!enabled) {
      return;
    }

    static const char * const kSiteNames[PS_COUNT] =
    {
      "other",
      "translate_utf8",
      "translate_trigraph",
      "consume_pp_number",
      "match_header_name",
      "match_op_or_punc",
      "match_whitespace",
      "get_regular_string",
      "get_stringlike",
      "raw_string"
    };

    fprintf(stderr, "peek.total_calls %llu\n",
            static_cast<unsigned long long>(total_calls));
    fprintf(stderr, "peek.miss_calls %llu\n",
            static_cast<unsigned long long>(miss_calls));
    fprintf(stderr, "peek.fill_steps %llu\n",
            static_cast<unsigned long long>(fill_steps));
    fprintf(stderr, "peek.max_n %llu\n",
            static_cast<unsigned long long>(max_n));
    fprintf(stderr, "peek.large_n_calls %llu\n",
            static_cast<unsigned long long>(large_n_calls));
    for(size_t i = 0; i < n_hist.size(); ++i) {
      fprintf(stderr, "peek.n[%zu] %llu\n", i,
              static_cast<unsigned long long>(n_hist[i]));
    }
    for(size_t i = 0; i < site_calls.size(); ++i) {
      fprintf(stderr, "peek.site[%s] %llu\n",
              kSiteNames[i],
              static_cast<unsigned long long>(site_calls[i]));
    }
  }

  bool enabled;
  std::uint64_t total_calls;
  std::uint64_t miss_calls;
  std::uint64_t fill_steps;
  std::uint64_t max_n;
  std::uint64_t large_n_calls;
  std::array<std::uint64_t, 8> n_hist;
  std::array<std::uint64_t, PS_COUNT> site_calls;
  PeekSite current_site;
};

PeekStats & peek_stats()
{
  static PeekStats stats;
  return stats;
}

struct PeekScope
{
  explicit PeekScope(PeekSite site)
    : stats(peek_stats()),
      previous(PS_OTHER)
  {
    if(stats.enabled) {
      previous = stats.current_site;
      stats.current_site = site;
    }
  }

  ~PeekScope()
  {
    if(stats.enabled) {
      stats.current_site = previous;
    }
  }

  PeekStats & stats;
  PeekSite previous;
};
#else
struct PeekScope
{
  explicit PeekScope(PeekSite) {}
};
#endif

// EndOfFile: synthetic "character" to represent the end of source file
constexpr int EndOfFile = -1;

// given hex digit character c, return its value
// See C++ standard 2.11 Identifiers and Appendix/Annex E.1
const vector<pair<int, int>> IdentifierNonDigitRangesSorted =
{
  {'A','Z'},
  {'_','_'},
  {'a','z'},
  {0xA8,0xA8},
  {0xAA,0xAA},
  {0xAD,0xAD},
  {0xAF,0xAF},
  {0xB2,0xB5},
  {0xB7,0xBA},
  {0xBC,0xBE},
  {0xC0,0xD6},
  {0xD8,0xF6},
  {0xF8,0xFF},
  {0x100,0x167F},
  {0x1681,0x180D},
  {0x180F,0x1FFF},
  {0x200B,0x200D},
  {0x202A,0x202E},
  {0x203F,0x2040},
  {0x2054,0x2054},
  {0x2060,0x206F},
  {0x2070,0x218F},
  {0x2460,0x24FF},
  {0x2776,0x2793},
  {0x2C00,0x2DFF},
  {0x2E80,0x2FFF},
  {0x3004,0x3007},
  {0x3021,0x302F},
  {0x3031,0x303F},
  {0x3040,0xD7FF},
  {0xF900,0xFD3D},
  {0xFD40,0xFDCF},
  {0xFDF0,0xFE44},
  {0xFE47,0xFFFD},
  {0x10000,0x1FFFD},
  {0x20000,0x2FFFD},
  {0x30000,0x3FFFD},
  {0x40000,0x4FFFD},
  {0x50000,0x5FFFD},
  {0x60000,0x6FFFD},
  {0x70000,0x7FFFD},
  {0x80000,0x8FFFD},
  {0x90000,0x9FFFD},
  {0xA0000,0xAFFFD},
  {0xB0000,0xBFFFD},
  {0xC0000,0xCFFFD},
  {0xD0000,0xDFFFD},
  {0xE0000,0xEFFFD}
};

// See C++ standard 2.11 Identifiers and Appendix/Annex E.2
const vector<pair<int, int>> IdentifierDisallowedInitiallyRangesSorted =
{
  {0x300,0x36F},
  {0x1DC0,0x1DFF},
  {0x20D0,0x20FF},
  {0xFE20,0xFE2F}
};

inline bool in_sorted_ranges(const vector<pair<int, int>> & ranges, int value)
{
  auto it = lower_bound(ranges.begin(), ranges.end(), value,
                        [](const pair<int, int> & range, int needle) {
                          return range.second < needle;
                        });
  return it != ranges.end() && value >= it->first;
}

inline bool is_ascii_identifier_initial(int value)
{
  return value == '_' ||
         (value >= 'A' && value <= 'Z') ||
         (value >= 'a' && value <= 'z');
}

inline bool is_ascii_identifier_continue(int value)
{
  return is_ascii_identifier_initial(value) ||
         (value >= '0' && value <= '9');
}

inline bool is_identifier_nondigit(int value)
{
  if(is_ascii_identifier_initial(value)) {
    return true;
  }
  if(value < IdentifierNonDigitRangesSorted.front().first ||
     value > IdentifierNonDigitRangesSorted.back().second) {
    return false;
  }
  return in_sorted_ranges(IdentifierNonDigitRangesSorted, value);
}

inline bool is_identifier_initial(int value)
{
  if(is_ascii_identifier_initial(value)) {
    return true;
  }
  if(value < IdentifierNonDigitRangesSorted.front().first ||
     value > IdentifierNonDigitRangesSorted.back().second) {
    return false;
  }
  return is_identifier_nondigit(value) &&
         !in_sorted_ranges(IdentifierDisallowedInitiallyRangesSorted, value);
}

inline bool is_identifier_like_operator(const string & str)
{
  switch(str.size()) {
  case 2:
    return str == "or";
  case 3:
    return str == "and" || str == "new" || str == "not" || str == "xor";
  case 5:
    return str == "bitor" || str == "compl" || str == "or_eq";
  case 6:
    return str == "and_eq" || str == "bitand" || str == "delete" ||
           str == "not_eq" || str == "xor_eq";
  default:
    return false;
  }
}

inline bool is_one_character_operator(int value)
{
  switch(value) {
  case '{':
  case '}':
  case '[':
  case ']':
  case '#':
  case '(':
  case ')':
  case ';':
  case ':':
  case '?':
  case '.':
  case '+':
  case '-':
  case '*':
  case '/':
  case '%':
  case '^':
  case '&':
  case '|':
  case '~':
  case '!':
  case '=':
  case '<':
  case '>':
  case ',':
    return true;
  default:
    return false;
  }
}

inline bool is_always_single_character_operator(int value)
{
  switch(value) {
  case '{':
  case '}':
  case '[':
  case ']':
  case '(':
  case ')':
  case ';':
  case '?':
  case '~':
  case ',':
    return true;
  default:
    return false;
  }
}

inline bool is_two_character_operator(int value, int next)
{
  switch(value) {
  case '#':
    return next == '#';
  case '<':
    return next == ':' || next == '%' || next == '<' || next == '=';
  case ':':
    return next == '>' || next == ':';
  case '%':
    return next == '>' || next == ':' || next == '=';
  case '.':
    return next == '*';
  case '+':
    return next == '=' || next == '+';
  case '-':
    return next == '=' || next == '-' || next == '>';
  case '*':
  case '/':
  case '^':
    return next == '=';
  case '&':
    return next == '=' || next == '&';
  case '|':
    return next == '=' || next == '|';
  case '=':
  case '!':
    return next == '=';
  case '>':
    return next == '>' || next == '=';
  default:
    return false;
  }
}

inline bool is_escape_character(int value)
{
  switch(value) {
  case '\'':
  case '"':
  case '?':
  case '\\':
  case 'a':
  case 'b':
  case 'f':
  case 'n':
  case 'r':
  case 't':
  case 'v':
    return true;
  default:
    return false;
  }
}

inline bool is_hex_digit(int value)
{
  return (value >= '0' && value <= '9') ||
         (value >= 'A' && value <= 'F') ||
         (value >= 'a' && value <= 'f');
}

Normalizer::Normalizer(streambuf * buf) :
  CodePointIterator(1, 1),
  buf(buf)
{
  auto next = buf->sgetc();
  value = next == char_traits<char>::eof() ? EndOfFile :
          static_cast<unsigned char>(next);
}

int Normalizer::operator*()
{
  return value;
}

inline CodePointIterator& Normalizer::operator++()
{
  if (value != EndOfFile && buf != nullptr) {
    if (value == '\n') {
      ch = 1;
      ++ln;
    } else {
      ++ch;
    }
    auto next = buf->sbumpc();
    (void)next;
    next = buf->sgetc();
    if (next == char_traits<char>::eof()) {
      if (value != '\n') {
        value = '\n';
      } else {
        value = EndOfFile;
      }
    } else {
      value = static_cast<unsigned char>(next);
    }
  }
  return *this;
}

BufferedIterator::BufferedIterator(CodePointIterator & source) :
  CodePointIterator(source.ln, source.ch),
  source(source)
{}

BufferedIterator::Buffer::Buffer() :
  inline_data(),
  overflow_data(),
  data(inline_data.data()),
  capacity(inline_data.size()),
  mask(capacity - 1),
  start(0),
  count(0)
{}

bool BufferedIterator::Buffer::empty() const
{
  return count == 0;
}

int BufferedIterator::Buffer::front() const
{
  return data[start];
}

size_t BufferedIterator::Buffer::size() const
{
  return count;
}

size_t BufferedIterator::Buffer::index(size_t n) const
{
  return (start + n) & mask;
}

size_t BufferedIterator::Buffer::storage_capacity() const
{
  return capacity;
}

int BufferedIterator::Buffer::operator[](size_t n) const
{
  return data[index(n)];
}

void BufferedIterator::Buffer::push_back(int value)
{
  if (count == storage_capacity())
    ensure_capacity(count + 1);
  data[index(count)] = value;
  ++count;
}

void BufferedIterator::Buffer::push_front(int value)
{
  if (count == storage_capacity())
    ensure_capacity(count + 1);
  start = (start - 1) & mask;
  data[start] = value;
  ++count;
}

void BufferedIterator::Buffer::set_front(int value)
{
  data[start] = value;
}

void BufferedIterator::Buffer::pop_front()
{
  start = (start + 1) & mask;
  --count;
}

void BufferedIterator::Buffer::ensure_capacity(size_t required)
{
  if (required <= storage_capacity())
    return;

  vector<int> expanded(storage_capacity() * 2);
  size_t tail = storage_capacity() - start;
  size_t first = min(count, tail);
  copy_n(data + start, first, expanded.begin());
  copy_n(data, count - first,
         expanded.begin() + static_cast<ptrdiff_t>(first));
  overflow_data.swap(expanded);
  data = overflow_data.data();
  capacity = overflow_data.size();
  mask = capacity - 1;
  start = 0;
}

int BufferedIterator::operator*()
{
  if (buffer.empty()) {
    buffer.push_back(*source);
    ln = source.ln;
    ch = source.ch;
    ++source;
  }
  return buffer.front();
}

CodePointIterator& BufferedIterator::operator++()
{
  if (!buffer.empty())
    buffer.pop_front();
  else
    ++source;
  return *this;
}

inline int BufferedIterator::peek(int n)
{
  bool missed = false;
#ifdef CPPGM_PROFILE_PEEK_ENABLED
  PeekStats & stats = peek_stats();
  if(stats.enabled) {
    ++stats.total_calls;
    ++stats.site_calls[stats.current_site];
    if(n >= 0 && static_cast<size_t>(n) < stats.n_hist.size()) {
      ++stats.n_hist[static_cast<size_t>(n)];
    } else {
      ++stats.large_n_calls;
    }
    if(static_cast<std::uint64_t>(n) > stats.max_n) {
      stats.max_n = static_cast<std::uint64_t>(n);
    }
  }
#else
  (void)missed;
#endif

  auto fill_one = [&]() {
#ifdef CPPGM_PROFILE_PEEK_ENABLED
    if(stats.enabled) {
      if(!missed) {
        ++stats.miss_calls;
        missed = true;
      }
      ++stats.fill_steps;
    }
#endif
    buffer.push_back(*source);
    ln = source.ln;
    ch = source.ch;
    ++source;
  };

  size_t buffered = buffer.size();
  if(n <= 0) {
    if(buffered == 0) {
      fill_one();
    }
    return buffer.front();
  }
  if(n == 1) {
    if(buffered == 0) {
      fill_one();
      fill_one();
    } else if(buffered == 1) {
      fill_one();
    }
    return buffer[1];
  }
  if(n == 2) {
    if(buffered == 0) {
      fill_one();
      fill_one();
      fill_one();
    } else if(buffered == 1) {
      fill_one();
      fill_one();
    } else if(buffered == 2) {
      fill_one();
    }
    return buffer[2];
  }
  while(buffer.size() <= static_cast<size_t>(n)) {
    fill_one();
  }
  return buffer[n];
}

inline int BufferedIterator::next()
{
  size_t buffered = buffer.size();
  if (buffered > 1) {
    buffer.pop_front();
    return buffer.front();
  }
  if (buffered == 1) {
    buffer.set_front(*source);
    ln = source.ln;
    ch = source.ch;
    ++source;
    return buffer.front();
  } else {
    ++source;
  }
  buffer.push_back(*source);
  ln = source.ln;
  ch = source.ch;
  ++source;
  return buffer.front();
}

inline int BufferedIterator::pop()
{
  if (buffer.empty()) {
    buffer.push_back(*source);
    ln = source.ln;
    ch = source.ch;
    ++source;
  }
  int value = buffer.front();
  buffer.pop_front();
  return value;
}

inline u32string BufferedIterator::extract32(int n)
{
  u32string data;
  data.reserve(n);
  for(int i = 0; i < n; ++i) {
    int value = pop();
    if (value == EndOfFile)
      throw runtime_error("Extracting past the end of input");
    data.push_back(static_cast<char32_t>(value));
  }
  return data;
}

inline string BufferedIterator::extract(int n)
{
    string data;
    data.reserve(n);
    for(int i = 0; i < n; ++i) {
      int value = pop();
      if (value == EndOfFile)
        throw runtime_error("Extracting past the end of input");
      if(value >= 0 && value < 0x80) {
        data.push_back(static_cast<char>(value));
      } else {
        append_utf8_bytes(value, data);
      }
    }
    return data;
}

inline void append_code_point(int value, string & data)
{
  if(value >= 0 && value < 0x80) {
    data.push_back(static_cast<char>(value));
  } else {
    append_utf8_bytes(value, data);
  }
}

UTF8Translator::UTF8Translator(CodePointIterator & source) :
  BufferedIterator(source),
  allow_initial_bom(true)
{}

inline int windows_1252_c1_source_compat(int value)
{
  switch(value) {
  case 0x80: return 0x20AC;
  case 0x82: return 0x201A;
  case 0x83: return 0x0192;
  case 0x84: return 0x201E;
  case 0x85: return 0x2026;
  case 0x86: return 0x2020;
  case 0x87: return 0x2021;
  case 0x88: return 0x02C6;
  case 0x89: return 0x2030;
  case 0x8A: return 0x0160;
  case 0x8B: return 0x2039;
  case 0x8C: return 0x0152;
  case 0x8E: return 0x017D;
  case 0x91: return 0x2018;
  case 0x92: return 0x2019;
  case 0x93: return 0x201C;
  case 0x94: return 0x201D;
  case 0x95: return 0x2022;
  case 0x96: return 0x2013;
  case 0x97: return 0x2014;
  case 0x98: return 0x02DC;
  case 0x99: return 0x2122;
  case 0x9A: return 0x0161;
  case 0x9B: return 0x203A;
  case 0x9C: return 0x0153;
  case 0x9E: return 0x017E;
  case 0x9F: return 0x0178;
  default: return 0;
  }
}

int UTF8Translator::operator*()
{
  if (buffer.empty())
  {
    int value = *source;
    if (value >= 0 && value < 0x80) {
      ln = source.ln;
      ch = source.ch;
      return value;
    } else {
      translate_utf8();
    }
  }
  return buffer.front();
}

inline void UTF8Translator::translate_utf8()
{
  PeekScope scope(PS_TRANSLATE_UTF8);
  for(;;) {
    int value = *source;
    ln = source.ln;
    ch = source.ch;
    if (value < 0x80 || value == EndOfFile) {
      buffer.push_back(value);
      ++source;
      allow_initial_bom = false;
      return;
    }

    ++source;
    if (value <= 0xFF) {
      auto tail = utf8_tail_length(value);
      if (!tail) {
        int compat_value = windows_1252_c1_source_compat(value);
        if (!compat_value)
          throw logic_error("Invalid utf-8 character");
        value = compat_value;
      } else {
        value &= (0x3f >> tail);

        for (int i = 0; i < tail; ++i) {
          int next = *source;
          if (next == EndOfFile || (next & 0xc0) != 0x80)
            throw logic_error("Invalid utf-8 character");
          value = ((value << 6) + (next & 0x3f));
          ++source;
        }
      }
    }

    if (allow_initial_bom) {
      allow_initial_bom = false;
      if (ln == 1 && ch == 1 && value == 0xFEFF) {
        continue;
      }
    }
    buffer.push_back(value);
    return;
  }
}

inline void FullTranslator::translate_trigraph_ucn_splice()
{
  PeekScope scope(PS_TRANSLATE_TRIGRAPH);
  int extra = 0;
  int value = peek();
  if (value == '?') {
    int next = peek(1);
    if (next == '?') {
      int third = peek(2);
      // Translate is only called if the buffer is empty we translate greedily
      for (; third == '?'; ++extra) {
        third = peek(extra + 3);
      }
      value = 0;
      switch (third) {
      case '=': value = '#'; break;
      case '/': value = '\\'; break;
      case '\'': value = '^'; break;
      case '(': value = '['; break;
      case ')': value = ']'; break;
      case '!': value = '|'; break;
      case '<': value = '{'; break;
      case '>': value = '}'; break;
      case '-': value = '~'; break;
      }
      if (value) {
        for (int i = 0; i < extra; ++i) {
          ++*this;
        }
        ++*this;
        ++*this;
        ++*this;
        buffer.push_front(value);
      }
    }
  }
  while (value == '\\') {
    int next = peek(1);
    if (next == 'u' || next == 'U') {
      int num = next == 'u' ? 4 : 8;
      bool valid_ucn = true;
      for(int i = 0; i < num; ++i) {
        int digit = peek(i + 2);
        if (digit == EndOfFile)
          throw logic_error("Unterminated unicode escape");
        if (!is_hex_digit(digit)) {
          valid_ucn = false;
          break;
        }
      }
      if(!valid_ucn)
        break;
      ++*this;
      ++*this;
      value = 0;
      for(int i = 0; i < num; ++i) {
        next = pop();
        value = (value << 4) + hex_to_value(next);
      }
      buffer.push_front(value);
      break;
    } else if (next == '\n') {
      ++*this;
      ++*this;
    } else {
      break;
    }
    value = peek();
  }
  if (value) {
    for (int i = 0; i < extra; ++i) {
      buffer.push_front('?');
    }
  }
}

FullTranslator::FullTranslator(CodePointIterator & source) :
  UTF8Translator(source)
{}

int FullTranslator::operator*()
{
  if (buffer.empty())
  {
    int value = *source;
    if (value >= 0 && value < 0x80) {
      ln = source.ln;
      ch = source.ch;
      if (value == '?' || value == '\\') {
        buffer.push_back(value);
        ++source;
        translate_trigraph_ucn_splice();
      } else {
        return value;
      }
    } else {
      translate_utf8();
    }
  }
  return buffer.front();
}

inline bool consume_identifier_known_initial(BufferedIterator & it,
                                             string & out,
                                             int value)
{
  char ascii[32];
  size_t ascii_len = 0;
  auto flush_ascii = [&]() {
    if (ascii_len == 0)
      return;
    out.append(ascii, ascii_len);
      ascii_len = 0;
  };

  out.clear();
  out.reserve(16);
  for(;;) {
    if (value >= 0 && value < 0x80) {
      ascii[ascii_len++] = static_cast<char>(value);
      if (ascii_len == sizeof(ascii)) {
        flush_ascii();
      }
    } else {
      flush_ascii();
      append_utf8_bytes(value, out);
    }
    value = it.next();
    if(is_ascii_identifier_continue(value)) {
      continue;
    }
    if(value >= 0x80 && is_identifier_nondigit(value))
      continue;
    flush_ascii();
    return true;
  }
}

bool consume_identifier(BufferedIterator & it, string & out)
{
  int value = *it;
  if(!is_identifier_initial(value)) {
    return false;
  }
  return consume_identifier_known_initial(it, out, value);
}

bool consume_pp_number(BufferedIterator & it, string & out, bool & is_float)
{
  PeekScope scope(PS_CONSUME_PP_NUMBER);
  int value = *it;
  bool is_hex = false;
  is_float = false;
  out.clear();
  out.reserve(16);
  if (value == '.')
  {
    if (it.peek(1) < '0' || it.peek(1) > '9')
      return false;
    is_float = true;
    append_code_point(value, out);
    ++it;
    value = *it;
  } else if (value < '0' || value > '9') {
    return false;
  }

  for(;;) {
    if (value == '.') {
      is_float = true;
      append_code_point(value, out);
      ++it;
      value = *it;
      continue;
    }
    if (!is_float && (value == 'x' || value == 'X')) {
      is_hex = true;
      append_code_point(value, out);
      ++it;
      value = *it;
      continue;
    }
    if((!is_hex && (value == 'e' || value == 'E')) ||
       (is_hex && (value == 'p' || value == 'P'))) {
      int next = it.peek(1);
      if (next == '+' || next == '-') {
        is_float = true;
        append_code_point(value, out);
        ++it;
        append_code_point(*it, out);
        ++it;
        value = *it;
        continue;
      } else if (next >= '0' && next <= '9') {
        is_float = true;
        append_code_point(value, out);
        ++it;
        value = *it;
        continue;
      }
    }
    if (value >= '0' && value <= '9') {
      append_code_point(value, out);
      ++it;
      value = *it;
      continue;
    }
    if(is_ascii_identifier_initial(value)) {
      append_code_point(value, out);
      ++it;
      value = *it;
      continue;
    }
    if(value >= 0x80 && is_identifier_nondigit(value)) {
      append_code_point(value, out);
      ++it;
      value = *it;
      continue;
    }
    return true;
  }
}

int match_header_name(BufferedIterator & it)
{
  PeekScope scope(PS_MATCH_HEADER_NAME);
  int value = *it;
  if (value == '"' || value == '<') {
    int end = (value == '"' ? '"' : '>');
    for(int i = 1;; ++i) {
      value = it.peek(i);
      if (value == EndOfFile || value == '\n')
        throw logic_error("Unterminated header name");
      if (value == end)
        return i + 1;
    }
  }
  return 0;
}

int match_preprocessing_op_or_punc(BufferedIterator & it)
{
  PeekScope scope(PS_MATCH_OP_OR_PUNC);
  int value = *it;
  int next;
  int third;
  switch(value) {
  case '{':
  case '}':
  case '[':
  case ']':
  case '(':
  case ')':
  case ';':
  case '?':
  case '~':
  case ',':
    return 1;
  case '#':
    return it.peek(1) == '#' ? 2 : 1;
  case ':':
    next = it.peek(1);
    return next == '>' || next == ':' ? 2 : 1;
  case '*':
  case '/':
  case '^':
    return it.peek(1) == '=' ? 2 : 1;
  case '=':
  case '!':
    return it.peek(1) == '=' ? 2 : 1;
  case '+':
    next = it.peek(1);
    return next == '=' || next == '+' ? 2 : 1;
  case '&':
    next = it.peek(1);
    return next == '=' || next == '&' ? 2 : 1;
  case '|':
    next = it.peek(1);
    return next == '=' || next == '|' ? 2 : 1;
  case '.':
    next = it.peek(1);
    if(next == '.') {
      return it.peek(2) == '.' ? 3 : 1;
    }
    return next == '*' ? 2 : 1;
  case '-':
    next = it.peek(1);
    if(next == '=' || next == '-') {
      return 2;
    }
    if(next != '>') {
      return 1;
    }
    return it.peek(2) == '*' ? 3 : 2;
  case '%':
    next = it.peek(1);
    if(next == '>' || next == '=') {
      return 2;
    }
    if(next != ':') {
      return 1;
    }
    return it.peek(2) == '%' && it.peek(3) == ':' ? 4 : 2;
  case '<':
    next = it.peek(1);
    if(next == '%' || next == '=') {
      return 2;
    }
    if(next == '<') {
      return it.peek(2) == '=' ? 3 : 2;
    }
    if(next != ':') {
      return 1;
    }
    third = it.peek(2);
    if(third != ':') {
      return 2;
    }
    return it.peek(3) == ':' || it.peek(3) == '>' ? 2 : 1;
  case '>':
    next = it.peek(1);
    if(next == '=') {
      return 2;
    }
    if(next != '>') {
      return 1;
    }
    return it.peek(2) == '=' ? 3 : 2;
  default:
    return 0;
  }
}

inline int match_whitespace(BufferedIterator & it)
{
  PeekScope scope(PS_MATCH_WHITESPACE);
  int i;
  int value;
  for(i = 0;; ++i) {
    value = *it;
    if (value == ' ' || value == '\t' || value == '\v' ||
        value == '\f' || value == '\r') {
      it.next();
    } else if (value == '/') {
      int next = it.peek(1);
      if (next == '/') {
        it.next();
        while (next != EndOfFile) {
          next = it.next();
          if (next == '\n') {
            break;
          }
        }
      } else if (next == '*') {
        it.next();
        while (next != EndOfFile) {
          next = it.next();
          if (next == '*') {
            next = it.next();
            while(next == '*')
              next = it.next();
            if (next != '/')
              continue;
            it.next();
            break;
          }
        }
      } else {
        break;
      }
      if (next == EndOfFile)
        throw logic_error("Unterminated comment");
    } else {
      break;
    }
  }
  return i;
}

PPTokenizer::PPTokenizer(streambuf * buf) :
  norm(buf),
  raw(norm),
  translator(norm),
  // cast is to avoid a copy constructor
  buffer(static_cast<CodePointIterator &>(translator)),
  header_state(HeaderState::Start)
{}

EPPToken PPTokenizer::get() {
  int result;
  string str;
  string ud_suffix;
  bool is_float;
  int value = *buffer;
  token_ln = buffer.ln;
  token_ch = buffer.ch;
  if (value == EndOfFile) {
    return token(PP_EOF);
  } else if (value == '\n') {
    header_state = HeaderState::Start;
    buffer.pop();
    return token(PP_NEW_LINE);
  } else if (match_whitespace(buffer)) {
    return token(PP_WHITESPACE);
  } else if (header_state == HeaderState::Include &&
             (result = match_header_name(buffer))) {
      header_state = HeaderState::None;
      return token(PP_HEADER_NAME, buffer.extract(result));
  } else if ((is_ascii_identifier_initial(value) ||
              (value >= 0x80 && is_identifier_initial(value))) &&
             value != 'u' && value != 'U' && value != 'L' && value != 'R') {
    consume_identifier_known_initial(buffer, str, value);
    if (header_state == HeaderState::Hash && str == "include" ) {
      header_state = HeaderState::Include;
    } else {
      header_state = HeaderState::None;
    }
    if (is_identifier_like_operator(str)) {
      return token(PP_PREPROCESSING_OP, std::move(str));
    } else {
      return token(PP_IDENTIFIER, std::move(str));
    }
  } else if (consume_pp_number(buffer, str, is_float)) {
    header_state = HeaderState::None;
    if(is_float)
      return token(PP_FLOAT_LITERAL, std::move(str));
    else
      return token(PP_INT_LITERAL, std::move(str));
  } else if (is_always_single_character_operator(value)) {
    buffer.pop();
    header_state = HeaderState::None;
    return token(PP_PREPROCESSING_OP,
                 string(1, static_cast<char>(value)));
  } else if ((result = match_preprocessing_op_or_punc(buffer))) {
    str = buffer.extract(result);
    if (header_state == HeaderState::Start && (str == "#" || str == "%:")) {
      header_state = HeaderState::Hash;
    } else {
      header_state = HeaderState::None;
    }
    return token(PP_PREPROCESSING_OP, std::move(str));
  } else if (get_stringlike(str)) {
    if (consume_identifier(buffer, ud_suffix)) {
      str += ud_suffix;
    }
    header_state = HeaderState::None;
    return token(PP_QUOTE_LITERAL, std::move(str));
  } else if (consume_identifier(buffer, str)) {
    if (header_state == HeaderState::Hash && str == "include" ) {
      header_state = HeaderState::Include;
    } else {
      header_state = HeaderState::None;
    }
    if (is_identifier_like_operator(str)) {
      return token(PP_PREPROCESSING_OP, std::move(str));
    } else {
      return token(PP_IDENTIFIER, std::move(str));
    }
  } else {
    return token(PP_NON_WHITESPACE, buffer.extract(1));
  }
}

void PPTokenizer::stream(IPPTokenStream & output)
{
  stream_pp_tokens(*this, output);
}

string PPTokenizer::get_raw_string(unsigned int i)
{
  PeekScope scope(PS_RAW_STRING);
  u32string data = buffer.extract32(i);
  int value;
  u32string rawmatch;
  for(i = 0; i < 17; ++raw, ++i) {
    value = *raw;
    if(value == EndOfFile)
      throw logic_error("Unterminated raw string delimiter");
    rawmatch += value;
    if(value == '(')
      break;
  }
  if(i == 17)
      throw logic_error("Raw string identifier too long");
  data += rawmatch;
  rawmatch.insert(0, 1, ')');
  rawmatch.replace(rawmatch.size() - 1, 1, 1, '"');
  unsigned int j = 0;
  unsigned int matchlen = rawmatch.length();
  for(++raw;j < matchlen; ++raw) {
    value = *raw;
    if (value == EndOfFile)
      throw logic_error("Unterminated raw string");
    data += value;
    if ((unsigned int)value == rawmatch[0]) {
      j = 1;
    } else if ((unsigned int)value == rawmatch[j]) {
      ++j;
    } else {
      j = 0;
    }
  }
  return encode_utf8(data);
}

string PPTokenizer::get_regular_string(unsigned int i, const char quote)
{
  PeekScope scope(PS_GET_REGULAR_STRING);
  int value;
  for(;; ++i) {
    value = buffer.peek(i);
    if (value == EndOfFile || value == '\n') {
      if (quote == '"')
        throw logic_error("Unterminated string literal");
      else
        throw logic_error("Unterminated character literal");
    }
    if (value == '\\') {
      value = buffer.peek(++i);
      if (value == 'x') {
        // only one hex character is needed for a valid hex excape
        value = buffer.peek(++i);
        try {
          hex_to_value(value);
        } catch (const logic_error &) {
          throw logic_error("Invalid escape sequence");
        }
      } else if ((value < '0' || value > '7') &&
                 !is_escape_character(value)) {
        throw logic_error("Invalid escape sequence");
      }
    } else if (value == quote) {
      return buffer.extract(++i);
    }
  }
}

bool PPTokenizer::get_stringlike(string & out)
{
  PeekScope scope(PS_GET_STRINGLIKE);
  char quote = '\'';
  unsigned int i = 0;
  int value = *buffer;
  if (value == 'u') {
    value = buffer.peek(++i);
    if (value == '8') {
      quote = '"';
      value = buffer.peek(++i);
    }
  } else if (value == 'U' || value == 'L') {
    value = buffer.peek(++i);
  }
  if (value == 'R') {
    value = buffer.peek(++i);
    if (value != '"')
      return false;
    out = get_raw_string(++i);
    return true;
  }

  if (value != '"' && value != quote) {
    return false;
  }
  out = get_regular_string(++i, value);
  return true;
}

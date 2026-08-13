#include <algorithm>
#include <stdexcept>

using namespace std;

#include "encoding.h"

constexpr unsigned char UTF8TailLengths[256] = {
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
  3,3,3,3,3,3,3,3,4,4,4,4,5,5,0,0
};

int hex_to_value(int c)
{
  switch (c)
  {
  case '0': return 0;
  case '1': return 1;
  case '2': return 2;
  case '3': return 3;
  case '4': return 4;
  case '5': return 5;
  case '6': return 6;
  case '7': return 7;
  case '8': return 8;
  case '9': return 9;
  case 'A': return 10;
  case 'a': return 10;
  case 'B': return 11;
  case 'b': return 11;
  case 'C': return 12;
  case 'c': return 12;
  case 'D': return 13;
  case 'd': return 13;
  case 'E': return 14;
  case 'e': return 14;
  case 'F': return 15;
  case 'f': return 15;
  default: throw logic_error("hex_to_value of nonhex char");
  }
}

char value_to_hex(int c)
{
  switch (c)
  {
  case 0: return '0';
  case 1: return '1';
  case 2: return '2';
  case 3: return '3';
  case 4: return '4';
  case 5: return '5';
  case 6: return '6';
  case 7: return '7';
  case 8: return '8';
  case 9: return '9';
  case 10: return 'A';
  case 11: return 'B';
  case 12: return 'C';
  case 13: return 'D';
  case 14: return 'E';
  case 15: return 'F';
  default: throw logic_error("value_to_hex of nonhex value");
  }
}

// hex dump memory range
string hex_dump(const void* pdata, size_t nbytes)
{
  unsigned char* p = (unsigned char*) pdata;

  string s(nbytes*2, '?');

  for(size_t i = 0; i < nbytes; i++)
  {
    s[2*i+0] = value_to_hex((p[i] & 0xF0) >> 4);
    s[2*i+1] = value_to_hex((p[i] & 0x0F) >> 0);
  }

  return s;
}

u32string decode_utf8(const string& data) {
  u32string result;
  for(size_t index = 0; index < data.size(); ++index) {
    int value = static_cast<unsigned char>(data[index]);
    if(value >= 0x80 && value <= 0xFF) {
      auto tail = utf8_tail_length(value);
      if (!tail)
        throw logic_error("Invalid utf-8 character");
      value &= ( 0x3F >> tail );
      for(int i = 0; i < tail; ++i) {
        ++index;
        if(index >= data.size())
          throw logic_error("Invalid utf-8 character");
        unsigned char next = static_cast<unsigned char>(data[index]);
        if ((next & 0xc0) != 0x80)
          throw logic_error("Invalid utf-8 character");
        value = ((value << 6) + (next & 0x3F));
      }
    }
    result.push_back(value);
  }
  return result;
}

string encode_utf8(const u32string& data) {
  string result;
  for(size_t i = 0; i < data.size(); ++i) {
    append_utf8_bytes(data[i], result);
  }
  return result;
}

void append_utf8_bytes(char32_t value, string& result) {
  if (value < 0x80) {
    result.push_back(static_cast<char>(value));
  } else if (value < 0x800) {
    result.push_back(static_cast<char>(0xC0 | (value >> 6)));
    result.push_back(static_cast<char>(0x80 | (value & 0x3F)));
  } else if (value < 0x10000) {
    result.push_back(static_cast<char>(0xE0 | (value >> 12)));
    result.push_back(static_cast<char>(0x80 | ((value >> 6) & 0x3F)));
    result.push_back(static_cast<char>(0x80 | (value & 0x3F)));
  } else if (value <= 0x0010FFFF) {
    result.push_back(static_cast<char>(0xF0 | (value >> 18)));
    result.push_back(static_cast<char>(0x80 | ((value >> 12) & 0x3F)));
    result.push_back(static_cast<char>(0x80 | ((value >> 6) & 0x3F)));
    result.push_back(static_cast<char>(0x80 | (value & 0x3F)));
  } else {
    throw runtime_error("Invalid 32 bit value for utf8");
  }
}

int utf8_tail_length(unsigned char byte) {
  return UTF8TailLengths[byte];
}

u16string encode_utf16(const u32string& data) {
  u16string result;
  for(size_t i = 0; i < data.size(); ++i) {
    char32_t value = data[i];
    if(value < 0x10000) {
      result.push_back((char16_t)value);
    } else {
      result.push_back(0xD800 | ((value - 0x10000) >> 10));
      result.push_back(0xDC00 | (0x3FF & value));
    }
  }
  return result;
}

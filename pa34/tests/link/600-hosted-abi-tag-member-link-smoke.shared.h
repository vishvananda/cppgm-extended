#pragma once

struct AbiTagBuffer
{
  int value;

  AbiTagBuffer();
  AbiTagBuffer(const AbiTagBuffer &) __attribute__((__abi_tag__("nqe220100")));
  int gptr() const __attribute__((__abi_tag__("nqe220100")));
  int egptr() const __attribute__((__abi_tag__("nqe220100")));
  void setstate(unsigned int) __attribute__((__abi_tag__("nqe220100")));
};

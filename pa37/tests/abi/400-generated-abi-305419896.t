#include "400-generated-abi-305419896.t.h"
extern "C" unsigned observe0(unsigned delta) {
  Record0 original = {static_cast<unsigned long long>(234)};
  Record0 (*volatile target)(Record0, unsigned) = transform0;
  Record0 changed = target(original, delta);
  return static_cast<unsigned>(changed.field0) * 1u + static_cast<unsigned>(original.field0) * 2u;
}
extern "C" unsigned observe1(unsigned delta) {
  Record1 original = {static_cast<unsigned short>(283)};
  Record1 (*volatile target)(Record1, unsigned) = transform1;
  Record1 changed = target(original, delta);
  return static_cast<unsigned>(changed.field0) * 1u + static_cast<unsigned>(original.field0) * 2u;
}
extern "C" unsigned observe2(unsigned delta) {
  Record2 original = {static_cast<unsigned>(639), static_cast<float>(212), static_cast<float>(589)};
  Record2 (*volatile target)(Record2, unsigned) = transform2;
  Record2 changed = target(original, delta);
  return static_cast<unsigned>(changed.field0) * 1u + static_cast<unsigned>(original.field0) * 2u + static_cast<unsigned>(changed.field1) * 3u + static_cast<unsigned>(original.field1) * 4u + static_cast<unsigned>(changed.field2) * 5u + static_cast<unsigned>(original.field2) * 6u;
}
extern "C" unsigned observe3(unsigned delta) {
  Record3 original = {static_cast<unsigned short>(465), static_cast<unsigned short>(398), static_cast<unsigned long long>(143), static_cast<unsigned short>(548), static_cast<unsigned short>(477)};
  Record3 (*volatile target)(Record3, unsigned) = transform3;
  Record3 changed = target(original, delta);
  return static_cast<unsigned>(changed.field0) * 1u + static_cast<unsigned>(original.field0) * 2u + static_cast<unsigned>(changed.field1) * 3u + static_cast<unsigned>(original.field1) * 4u + static_cast<unsigned>(changed.field2) * 5u + static_cast<unsigned>(original.field2) * 6u + static_cast<unsigned>(changed.field3) * 7u + static_cast<unsigned>(original.field3) * 8u + static_cast<unsigned>(changed.field4) * 9u + static_cast<unsigned>(original.field4) * 10u;
}
extern "C" unsigned observe4(unsigned delta) {
  Record4 original = {static_cast<unsigned>(609), static_cast<unsigned>(350), static_cast<float>(159), static_cast<unsigned>(372), static_cast<unsigned>(365)};
  Record4 (*volatile target)(Record4, unsigned) = transform4;
  Record4 changed = target(original, delta);
  return static_cast<unsigned>(changed.field0) * 1u + static_cast<unsigned>(original.field0) * 2u + static_cast<unsigned>(changed.field1) * 3u + static_cast<unsigned>(original.field1) * 4u + static_cast<unsigned>(changed.field2) * 5u + static_cast<unsigned>(original.field2) * 6u + static_cast<unsigned>(changed.field3) * 7u + static_cast<unsigned>(original.field3) * 8u + static_cast<unsigned>(changed.field4) * 9u + static_cast<unsigned>(original.field4) * 10u;
}
extern "C" unsigned observe5(unsigned delta) {
  Record5 original = {static_cast<double>(241), static_cast<unsigned long long>(302), static_cast<double>(175), static_cast<double>(196), static_cast<double>(253)};
  Record5 (*volatile target)(Record5, unsigned) = transform5;
  Record5 changed = target(original, delta);
  return static_cast<unsigned>(changed.field0) * 1u + static_cast<unsigned>(original.field0) * 2u + static_cast<unsigned>(changed.field1) * 3u + static_cast<unsigned>(original.field1) * 4u + static_cast<unsigned>(changed.field2) * 5u + static_cast<unsigned>(original.field2) * 6u + static_cast<unsigned>(changed.field3) * 7u + static_cast<unsigned>(original.field3) * 8u + static_cast<unsigned>(changed.field4) * 9u + static_cast<unsigned>(original.field4) * 10u;
}

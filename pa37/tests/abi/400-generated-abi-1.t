#include "400-generated-abi-1.t.h"
extern "C" unsigned observe0(unsigned delta) {
  Record0 original = {static_cast<unsigned char>(603)};
  Record0 (*volatile target)(Record0, unsigned) = transform0;
  Record0 changed = target(original, delta);
  return static_cast<unsigned>(changed.field0) * 1u + static_cast<unsigned>(original.field0) * 2u;
}
extern "C" unsigned observe1(unsigned delta) {
  Record1 original = {static_cast<unsigned>(480)};
  Record1 (*volatile target)(Record1, unsigned) = transform1;
  Record1 changed = target(original, delta);
  return static_cast<unsigned>(changed.field0) * 1u + static_cast<unsigned>(original.field0) * 2u;
}
extern "C" unsigned observe2(unsigned delta) {
  Record2 original = {static_cast<unsigned short>(276), static_cast<double>(397), static_cast<double>(506), static_cast<double>(363)};
  Record2 (*volatile target)(Record2, unsigned) = transform2;
  Record2 changed = target(original, delta);
  return static_cast<unsigned>(changed.field0) * 1u + static_cast<unsigned>(original.field0) * 2u + static_cast<unsigned>(changed.field1) * 3u + static_cast<unsigned>(original.field1) * 4u + static_cast<unsigned>(changed.field2) * 5u + static_cast<unsigned>(original.field2) * 6u + static_cast<unsigned>(changed.field3) * 7u + static_cast<unsigned>(original.field3) * 8u;
}
extern "C" unsigned observe3(unsigned delta) {
  Record3 original = {static_cast<double>(463), static_cast<unsigned long long>(612), static_cast<unsigned long long>(285)};
  Record3 (*volatile target)(Record3, unsigned) = transform3;
  Record3 changed = target(original, delta);
  return static_cast<unsigned>(changed.field0) * 1u + static_cast<unsigned>(original.field0) * 2u + static_cast<unsigned>(changed.field1) * 3u + static_cast<unsigned>(original.field1) * 4u + static_cast<unsigned>(changed.field2) * 5u + static_cast<unsigned>(original.field2) * 6u;
}
extern "C" unsigned observe4(unsigned delta) {
  Record4 original = {static_cast<unsigned>(417), static_cast<float>(414), static_cast<unsigned char>(479), static_cast<unsigned>(436), static_cast<unsigned char>(173)};
  Record4 (*volatile target)(Record4, unsigned) = transform4;
  Record4 changed = target(original, delta);
  return static_cast<unsigned>(changed.field0) * 1u + static_cast<unsigned>(original.field0) * 2u + static_cast<unsigned>(changed.field1) * 3u + static_cast<unsigned>(original.field1) * 4u + static_cast<unsigned>(changed.field2) * 5u + static_cast<unsigned>(original.field2) * 6u + static_cast<unsigned>(changed.field3) * 7u + static_cast<unsigned>(original.field3) * 8u + static_cast<unsigned>(changed.field4) * 9u + static_cast<unsigned>(original.field4) * 10u;
}
extern "C" unsigned observe5(unsigned delta) {
  Record5 original = {static_cast<double>(561), static_cast<unsigned long long>(366), static_cast<unsigned long long>(495), static_cast<unsigned short>(260), static_cast<unsigned short>(573)};
  Record5 (*volatile target)(Record5, unsigned) = transform5;
  Record5 changed = target(original, delta);
  return static_cast<unsigned>(changed.field0) * 1u + static_cast<unsigned>(original.field0) * 2u + static_cast<unsigned>(changed.field1) * 3u + static_cast<unsigned>(original.field1) * 4u + static_cast<unsigned>(changed.field2) * 5u + static_cast<unsigned>(original.field2) * 6u + static_cast<unsigned>(changed.field3) * 7u + static_cast<unsigned>(original.field3) * 8u + static_cast<unsigned>(changed.field4) * 9u + static_cast<unsigned>(original.field4) * 10u;
}

#ifndef CPPGM_C_LANE_API_H
#define CPPGM_C_LANE_API_H
#ifdef __cplusplus
extern "C" {
#endif
struct Pair { int left; double right; };
extern const int c_constant;
struct Pair c_transform(struct Pair value, int (*callback)(int));
int c_variadic(int count, ...);
#ifdef __cplusplus
}
#endif
#endif

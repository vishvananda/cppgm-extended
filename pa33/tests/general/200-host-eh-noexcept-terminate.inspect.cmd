set -eu
nm "__OBJ1__" | grep -q 'cppgm_call_terminate'
nm -u "__OBJ1__" | grep -q 'cxa_begin_catch'
nm -u "__OBJ1__" | grep -q 'ZSt9terminatev'
echo noexcept_terminate_surface_ok

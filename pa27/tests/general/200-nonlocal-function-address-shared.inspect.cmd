set -eu
host_cxx=${CPPGM_HOST_CXX:-${CXX:-g++}}
temporary=$(mktemp -d)
trap 'rm -rf "$temporary"' EXIT
"$host_cxx" -shared "__OBJ1__" -o "$temporary/addresses.so"
python3 - "$temporary/addresses.so" <<'PY'
import ctypes
import sys
library = ctypes.CDLL(sys.argv[1])
function_type = ctypes.CFUNCTYPE(ctypes.c_int, ctypes.c_int)
for name, expected in [('public_address', 8), ('inline_address', 9),
                       ('template_address', 10), ('local_address', 11)]:
    address = getattr(library, name)
    address.restype = ctypes.c_void_p
    assert function_type(address())(1) == expected
print('shared_function_addresses 1')
PY

set -e
host_cxx=${CPPGM_HOST_CXX:-${CXX:-g++}}
temporary=$(mktemp -d)
trap 'rm -rf "$temporary"' EXIT
"$host_cxx" -shared "__OBJ1__" -o "$temporary/data.so"
cat > "$temporary/override.cpp" <<'CPP'
int forward_data = 29;
int external_data = 31;
int array_data[3] = { 37, 41, 43 };
template<class T> struct Holder { static int data; };
template<> int Holder<int>::data = 47;
CPP
"$host_cxx" -fPIC -shared "$temporary/override.cpp" -o "$temporary/override.so"
python3 - "$temporary/override.so" "$temporary/data.so" <<'PY'
import ctypes
import sys
import threading
provider = ctypes.CDLL(sys.argv[1], mode=ctypes.RTLD_GLOBAL)
library = ctypes.CDLL(sys.argv[2])
for name, expected in [('forward_address', 29), ('external_address', 31),
                       ('internal_address', 11), ('template_address', 47),
                       ('array_address', 41)]:
    address = getattr(library, name)
    address.restype = ctypes.POINTER(ctypes.c_int)
    assert address().contents.value == expected
for name, expected in [('external_value', 31), ('template_value', 47), ('array_value', 41)]:
    assert getattr(library, name)() == expected
library.store_values(53)
assert ctypes.c_int.in_dll(provider, 'external_data').value == 53
assert library.external_value() == 53 and library.template_value() == 54
library.tls_address.restype = ctypes.POINTER(ctypes.c_int)
main_tls = library.tls_address()
assert main_tls.contents.value == 59
main_tls.contents.value = 61
thread_values = []
def check_thread():
    other = library.tls_address()
    thread_values.append(other.contents.value)
    other.contents.value = 67
thread = threading.Thread(target=check_thread)
thread.start()
thread.join()
assert thread_values == [59] and main_tls.contents.value == 61
print('shared_data_interposition 1')
PY

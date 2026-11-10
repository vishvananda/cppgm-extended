#include <cstdio>
#include <unwind.h>
extern "C" void assembly_personality_call(void (*)());
extern "C" const unsigned char assembly_exception_data[];
int personality_calls = 0;
int bad_data = 0;
extern "C" _Unwind_Reason_Code assembly_personality(int version, _Unwind_Action,
    _Unwind_Exception_Class, _Unwind_Exception*, _Unwind_Context* context) {
    ++personality_calls;
    const unsigned char* data = reinterpret_cast<const unsigned char*>(_Unwind_GetLanguageSpecificData(context));
    if (version != 1 || data != assembly_exception_data || data[0] != 69 || data[3] != 53) ++bad_data;
    return _URC_CONTINUE_UNWIND;
}
void callback() { throw 89; }
int main() {
    bool caught = false;
    try { assembly_personality_call(callback); }
    catch (int value) { caught = value == 89; }
    if (!caught || personality_calls < 2 || bad_data != 0) return 1;
    std::puts("assembly personality indirection and exception-data relocations passed");
}

// AUDIT-ID: EH-UNWIND-DTOR
// AUDIT-EXPECT: run
extern "C" void _Exit(int);
namespace std {
 typedef void (*terminate_handler)();
 terminate_handler set_terminate(terminate_handler) noexcept;
}
void terminated(){_Exit(0);}
struct S { ~S() noexcept(false){throw 2;} };
void f(){S object;throw 1;}
int main(){std::set_terminate(terminated);try{f();}catch(...){return 1;}return 2;}

// AUDIT-ID: EXPLICIT-CONTEXT
// AUDIT-EXPECT: run
// AUDIT-DIALECT: hosted-conditional-explicit
struct Missing {};
struct Value {int selected; template<class T> explicit(T{}) Value(T):selected(1){} Value(...):selected(2){} };
int main(){Value v(Missing{});return v.selected!=2;}

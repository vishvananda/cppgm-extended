// expect reject
void* operator new(unsigned long);
void operator delete(void*) noexcept;
using array=int[2];
int main(){int* p=::new array(1);return p[0];}

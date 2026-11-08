// expect reject
// P2280R4: an unknown this pointer cannot become a constant-result address.
struct Owner{void inspect(){constexpr const Owner* pointer=this;}};int main(){}

// expect exit 0
// P2280R4: explicit this and dereferenced this may identify unknown subobjects without reading their value.
struct Fixed{int n;constexpr int size()const{return 3;}};struct Owner{Fixed inner;void inspect(){static_assert(this->inner.size()==3,"this");static_assert((*this).inner.size()==3,"deref");}};int main(){Owner x;x.inspect();}

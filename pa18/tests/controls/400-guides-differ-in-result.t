template<bool B, class T = void> struct enable_if {};
template<class T> struct enable_if<true, T> { typedef T type; };
struct Alloc {};
template<class T> struct is_alloc { static const bool value = false; };
template<> struct is_alloc<Alloc> { static const bool value = true; };
template<class T> struct Less { int order() const { return 1; } };
template<class T> struct Greater { int order() const { return 2; } };
template<class T, class A = int> struct Store { int kind() const { return 10; } };
template<class T> struct Store<T, Alloc> { int kind() const { return 20; } };
template<class T, class C = Store<T>, class P = Less<T> > struct Heap {
  C store; P compare;
  template<class I> Heap(I, I) {}
  template<class I, class X> Heap(I, I, X) {}
  template<class I, class X, class Y> Heap(I, I, X, Y) {}
  int code() const { return store.kind() + compare.order(); }
};
template<class I, class P = Less<int>, class C = Store<int>,
         class = typename enable_if<!is_alloc<P>::value>::type,
         class = typename enable_if<!is_alloc<C>::value>::type>
Heap(I, I, P = P(), C = C()) -> Heap<int, C, P>;
template<class I, class P, class A,
         class = typename enable_if<!is_alloc<P>::value>::type,
         class = typename enable_if<is_alloc<A>::value>::type>
Heap(I, I, P, A) -> Heap<int, Store<int, A>, P>;
int main() {
  int values[2] = { 3, 4 };
  Heap plain(values, values + 2);
  Heap greater(values, values + 2, Greater<int>());
  Heap stored(values, values + 2, Greater<int>(), Store<int>());
  Heap allocated(values, values + 2, Greater<int>(), Alloc());
  Heap<int, Store<int>, Less<int> >* a = &plain;
  Heap<int, Store<int>, Greater<int> >* b = &greater;
  Heap<int, Store<int>, Greater<int> >* c = &stored;
  Heap<int, Store<int, Alloc>, Greater<int> >* d = &allocated;
  return a->code() == 11 && b->code() == 12 && c->code() == 12 && d->code() == 22 ? 0 : 1;
}

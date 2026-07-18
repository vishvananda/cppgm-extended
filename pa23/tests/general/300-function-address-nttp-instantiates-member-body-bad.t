// VALIDATION: compile-fail
// Boost.ConceptCheck takes the address of requirement<Model>::failed as a
// function-valued non-type argument.  Instantiating failed() must recursively
// validate the explicitly called Model destructor, where concept constraints
// are encoded.

template<void (*)()>
struct instantiate
{
};

template<class Model>
struct requirement
{
  static void failed()
  {
    ((Model *)0)->~Model();
  }
};

template<class T>
struct concept_model
{
  ~concept_model()
  {
    T::missing_requirement();
  }
};

struct incomplete_model
{
};

typedef instantiate<
    &requirement<concept_model<incomplete_model> >::failed> check;

int main()
{
  return 0;
}

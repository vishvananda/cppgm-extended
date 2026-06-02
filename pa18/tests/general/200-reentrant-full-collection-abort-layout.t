struct pa18_reentrant_layout_other {};

template<class From, class To>
struct pa18_reentrant_layout_compatible {
  static char test(To);
  static int test(...);
  enum { value = sizeof(test((From)0)) };
};

template<class T>
struct pa18_reentrant_layout_shared
    : pa18_reentrant_layout_compatible<pa18_reentrant_layout_other *, T *> {
  int data;
};

struct pa18_reentrant_layout_node;

struct pa18_reentrant_layout_node {
  int f0;
  int f1;
  int f2;
  int f3;
  int f4;
  int f5;
  pa18_reentrant_layout_shared<pa18_reentrant_layout_node> link;
  int tail;
};

int pa18_reentrant_layout_force_shared_size()
{
  return sizeof(pa18_reentrant_layout_shared<pa18_reentrant_layout_node>);
}

int pa18_reentrant_layout_read_tail(const pa18_reentrant_layout_node * node)
{
  return node->tail;
}

struct base {
  base(int);
  base(const base&);
  base(base&&);
};

struct target : base { using base::base; };
struct source { operator target(); };

static_assert(__is_constructible(target, source&), "");

int main() {}

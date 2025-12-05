#define fn(name) __attribute__((import_module("env"), import_name(name)))

typedef struct {
  int a;
  int b;
  int c;
} Foo;

fn("print_string") void print_string(Foo* foo);

void add(int a, int b) {
  Foo foo = {1, 125, -137};
  Foo* adr = &foo;
  print_string(adr);
}

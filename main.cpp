#include <iostream>
#include "generator.h"

using coro_exp::generator;

static const double demo_ceiling = 10E44;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
generator<int> f() {
  int i = 0;
  while (true)
    co_yield i++;
}
#pragma clang diagnostic pop

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma clang diagnostic ignored "-Wuninitialized"
#pragma clang diagnostic ignored "-Wfor-loop-analysis"
generator<double> fibonacci(const double ceiling) {
  double j = 0;
  double i = 1;
  co_yield j;
  if (ceiling > j) {
    do {
      co_yield i;
      double tmp = i;
      i += j;
      j = tmp;
    } while (i <= ceiling);
  }
}
#pragma clang diagnostic pop

template <class T>
void print(T &&arg) {
  std::cout << arg << ' ';
}
template <class T, class ... Ts>
void print(T &&arg, Ts &&... args) {
  print(std::forward<T>(arg));
  print(std::forward<Ts>(args)...);
}

int main() {
  std::cout << "Example using C++20 coroutines to implement Simple Integer and Fibonacci Sequence generators" << '\n';

  std::cout << '\n' << "Simple Integer Sequence Generator" << '\n' << ' ';
  auto iter1 = f();
  for(int i = 1; i <= 10; i++) {
    if (iter1.next()) {
      const auto value = iter1.getValue();
      print(i, ':', value, '\n');
    }
  }

  std::cout << '\n' << "Fibonacci Sequence Generator" << '\n' << ' ';
  int i = 1;
  auto iter2 = fibonacci(demo_ceiling);
  while(iter2.next()) {
    const auto value = iter2.getValue();
    print(i++, ':', value, '\n');
  }
}
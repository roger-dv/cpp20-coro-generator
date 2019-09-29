#include <experimental/coroutine>
#include "generator.h"
#include <iostream>

using coro_exp::generator;

static const double demo_ceiling = 10E44;

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
  co_return;
}
#pragma clang diagnostic pop

int main() {
  std::cout << "Example program using C++20 coroutine to implement a Fibonacci Sequence generator" << '\n';
  auto iter = fibonacci(demo_ceiling);
  while(iter.next()) {
    const auto value = iter.getValue();
    std::cout << value << '\n';
  }
}
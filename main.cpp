#include <experimental/coroutine>
#include "generator.h"
#include <cstdio>
#include <iostream>

using coro_exp::generator;

static const double ceiling = 10E44;

generator<double> fibonacci() {
  const double maxCeiling = ceiling;
  double j = 0;
  double i = 1;
  co_yield j;
  if (maxCeiling <= j) {
    co_return;
  }
  co_yield i;
  if (maxCeiling == i) {
    co_return;
  }
  for(;;) {
    double tmp = i;
    i += j;
    j = tmp;
    if (i > maxCeiling) {
      break;
    }
    co_yield i;
  }
}

int main() {
  puts("hello world");
  auto iter = fibonacci();
  while(iter.next()) {
    const auto value = iter.getValue();
    std::cout << value << '\n';
  }
}
#include <limits>
#include <iostream>
#include "generator.h"

using coro_exp::generator;

static const auto demo_ceiling1 = std::numeric_limits<unsigned long>::max() / 1'000ul;
static const auto demo_ceiling2 = std::numeric_limits<unsigned long long>::max() / 1'000ul;
static const auto demo_ceiling3 = std::numeric_limits<double>::max() / 1'000.0f;
static const auto demo_ceiling4 = std::numeric_limits<long double>::max() / 1'000.0f;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
template<typename T, std::enable_if_t<std::is_integral_v<T>, void*> = nullptr>
generator<T> f(T start) {
  T i = start;
  while (true)
    co_yield i++;
}
#pragma clang diagnostic pop

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma clang diagnostic ignored "-Wuninitialized"
#pragma clang diagnostic ignored "-Wfor-loop-analysis"
template<typename T, std::enable_if_t<std::is_arithmetic_v<T>, void*> = nullptr>
generator<T> fibonacci(const T ceiling) {
  T j = 0;
  T i = 1;
  co_yield j;
  if (ceiling > j) {
    do {
      co_yield i;
      T tmp = i;
      i += j;
      j = tmp;
    } while (i <= ceiling);
  }
}
#pragma clang diagnostic pop

/*
template <class T>
void print(T &&arg) {
  std::cout << arg << ' ';
}
template <class T, class ... Ts>
void print(T &&arg, Ts &&... args) {
  print(std::forward<T>(arg));
  print(std::forward<Ts>(args)...);
}
*/
// C++ (C++17 fold expressions)
template <class T>
void print_one(T &&arg) {
  std::cout << arg << ' ';
}
template <class ... Ts>
void print(Ts &&... args) {
  (print_one(std::forward<Ts>(args)), ...);
}

int main() {
  std::cout << "Example using C++20 coroutines to implement Simple Integer and Fibonacci Sequence generators" << '\n';

  std::cout << '\n' << "Simple Integer Sequence Generator" << '\n' << ' ';
  auto iter1 = f(0);
  for(int i = 1; i <= 10 && iter1.next(); i++) {
    const auto value = iter1.getValue();
    print(i, ": bytes", sizeof(value), ':', value, '\n');
  }

  auto const invoke_fib_seq = [](auto&& iter) {
    std::cout << '\n' << "Fibonacci Sequence Generator" << '\n' << ' ';
    for (int i = 1; iter.next(); i++) {
      const auto value = iter.getValue();
      print(i, ": bytes", sizeof(value), ':', value, '\n');
    }
  };

  invoke_fib_seq(fibonacci(demo_ceiling1));
  invoke_fib_seq(fibonacci(demo_ceiling2));
  invoke_fib_seq(fibonacci(demo_ceiling3));
  invoke_fib_seq(fibonacci(demo_ceiling4));
}
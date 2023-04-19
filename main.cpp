/** main.cpp
 *
 * Copyright 2023 Roger D. Voss
 *
 * Created by github user roger-dv on 09/28/2019.
 * Updated by github user roger-dv on 04/16/2023.
 *
 * Licensed under the MIT License - refer to LICENSE project document.
 *
 * Latest updates were to remove use of Clang experimental coroutines
 * and to instead use C++20 official coroutine implementation. Also,
 * C++20 Concepts are used to constrain template types instead of C++11
 * SFINAE type traits. The current implementation has been verified via
 * gcc/g++ v12.1 and with clang++ v16.
 */
#include <limits>
#include <iostream>
#include <algorithm>
#include "generator.h" // general purpose C++20 coroutine generator template class

static const auto demo_ceiling1 = std::numeric_limits<unsigned long>::max() / 1'000ul;
static const auto demo_ceiling2 = std::numeric_limits<unsigned long long>::max() / 1'000ul;
static const auto demo_ceiling3 = std::numeric_limits<double>::max() / 1'000.0f;
static const auto demo_ceiling4 = std::numeric_limits<long double>::max() / 1'000.0f;

// concept to constrain function templates that follow to only accept arithmetic types
template <typename T>
concept arithmetic = std::integral<T> || std::floating_point<T>;

/**
 * Returns number in ascending sequence starting at specified value.
 *
 * @tparam T arithmetic type of number returned
 * @param start value to begin sequence at
 * @return coroutine task iterator
 */
template<arithmetic T>
coro::generator<T> ascending_sequence(const T start) {
  T i = start;
  while (true) {
    T j = i++;
    co_yield j;
  }
}

/**
 * Generates Fibonacci sequence up to specified ceiling value.
 *
 * @tparam T arithmetic type of number returned
 * @param ceiling terminates generation of sequence when reaching
 * @return coroutine task iterator
 */
template<arithmetic T>
coro::generator<T> fibonacci(const T ceiling) {
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
  auto iter1 = ascending_sequence(0);
  try {
    for(int i = 1; i <= 10 && iter1.next(); i++) {
      const auto value = iter1.getValue().value();
      print(i, ": bytes", sizeof(value), ':', value, '\n');
    }
  } catch(const std::bad_optional_access& e) {
    // calling iter1.next() with true result prior to calling iter1.getValue().value()
    // should insure a value is always returned, so should never reach here
    std::cerr << e.what() << '\n';
  }

  auto const invoke_fib_seq = [](auto&& iter) {
    std::cout << '\n' << "Fibonacci Sequence Generator" << '\n' << ' ';
    try {
      for (int i = 1; iter.next(); i++) {
        const auto value = iter.getValue().value();
        print(i, ": bytes", sizeof(value), ':', value, '\n');
      }
    } catch(const std::bad_optional_access& e) {
      // calling iter.next() with true result prior to calling iter.getValue().value()
      // should insure a value is always returned, so should never reach here
      std::cerr << e.what() << '\n';
    }
  };

  invoke_fib_seq(fibonacci(demo_ceiling1));
  invoke_fib_seq(fibonacci(demo_ceiling2));
  invoke_fib_seq(fibonacci(demo_ceiling3));
  invoke_fib_seq(fibonacci(demo_ceiling4));

  // use the generator's coroutine task iterator
  try {
    std::cout << '\n' << "Fibonacci Sequence Generator" << '\n' << ' ';
    auto rng = fibonacci(demo_ceiling1);
    static_assert(std::ranges::input_range<decltype(rng)>);
    int i = 1;
    std::ranges::for_each(rng, [&i](const auto &value){ print(i++, ": bytes", sizeof(value), ':', value, '\n'); });
  } catch(const std::bad_optional_access& e) {
    // should never reach here
    std::cerr << e.what() << '\n';
  }
}
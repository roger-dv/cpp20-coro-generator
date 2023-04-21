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

  using coro_gen_ints_promise_type = coro::generator<decltype(0)>::promise_type;
  using coro_gen_ints = coro::generator<decltype(0)>;
  std::cerr << sizeof(coro_gen_ints_promise_type) << " bytes : coro::generator<int>::promise_type\n";
  std::cerr << sizeof(coro_gen_ints) << " bytes : coro::generator<int>\n";

  // insure instantiation of a decltype(0) coro::generator promise_type is on the stack - not the heap
  std::cerr << "set coro::generator<int> to stack memory buffer pmr allocator\n";
//  size_t buf_size = sizeof(coro_gen_ints_promise_type);
  size_t buf_size = 64; // allocating sizeof promise_type is insufficient for g++
  coro::fixed_buffer_pmr_allocator pmr_alloc{ alloca(buf_size), buf_size };
  coro::set_pmr_mem_pool(&pmr_alloc);

  std::cout << '\n' << "Simple Integer Sequence Generator" << '\n' << ' ';
  try {
    auto iter1 = ascending_sequence(0);
    for(int i = 1; i <= 10 && iter1.next(); i++) {
      const auto value = iter1.getValue().value();
      print(i, ": bytes", sizeof(value), ':', value, '\n');
    }
  } catch(const std::bad_optional_access& e) {
    // calling iter1.next() with true result prior to calling iter1.getValue().value()
    // should insure a value is always returned, so should never reach here
    std::cerr << e.what() << '\n';
  } catch(const std::bad_alloc& e) {
    // only reaches here if stack buffer was insufficient for allocating the coro::generator promise_type context
    std::cerr << e.what() << '\n';
  }

  using coro_gen_ulongs_promise_type = coro::generator<decltype(demo_ceiling1)>::promise_type;
  using coro_gen_ulongs = coro::generator<decltype(demo_ceiling1)>;
  using coro_gen_ulonglongs_promise_type = coro::generator<decltype(demo_ceiling2)>::promise_type;
  using coro_gen_ulonglongs = coro::generator<decltype(demo_ceiling2)>;
  using coro_gen_doubles_promise_type = coro::generator<decltype(demo_ceiling3)>::promise_type;
  using coro_gen_doubles = coro::generator<decltype(demo_ceiling3)>;
  using coro_gen_ldoubles_promise_type = coro::generator<decltype(demo_ceiling4)>::promise_type;
  using coro_gen_ldoubles = coro::generator<decltype(demo_ceiling4)>;

  std::cerr << sizeof(coro_gen_ulongs_promise_type) << " bytes : coro::generator<unsigned long>::promise_type\n";
  std::cerr << sizeof(coro_gen_ulongs) << " bytes : coro::generator<unsigned long>\n";

  std::cerr << sizeof(coro_gen_ulonglongs_promise_type) << " bytes : coro::generator<unsigned long long>::promise_type\n";
  std::cerr << sizeof(coro_gen_ulonglongs) << " bytes : coro::generator<unsigned long long>\n";

  std::cerr << sizeof(coro_gen_doubles_promise_type) << " bytes : coro::generator<double>::promise_type\n";
  std::cerr << sizeof(coro_gen_doubles) << " bytes : coro::generator<double>\n";

  std::cerr << sizeof(coro_gen_ldoubles_promise_type) << " bytes : coro::generator<long double>::promise_type\n";
  std::cerr << sizeof(coro_gen_ldoubles) << " bytes : coro::generator<long double>\n";

  // reset the coro::generator class to the default promise_type allocator (global new and delete)
  std::cerr << "reset coro::generator to default pmr allocator (global new and delete)\n";
  coro::reset_default_pmr_mem_pool();

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

  std::cerr << "now set coro::generator<long double> to stack memory buffer pmr allocator\n";
  // insure instantiation of a decltype(demo_ceiling4) coro::generator promise_type is on the stack - not the heap
//  buf_size = sizeof(coro_gen_ldoubles_promise_type);
  buf_size = 256; // allocating sizeof promise_type is insufficient for g++
  coro::fixed_buffer_pmr_allocator pmr_alloc_ldbl{ alloca(buf_size), buf_size };
  coro::set_pmr_mem_pool(&pmr_alloc_ldbl);

  try {
    invoke_fib_seq(fibonacci(demo_ceiling4)); // instantiates a coro::generator fibonacci for decltype(demo_ceiling4)
  } catch(const std::bad_alloc& e) {
    // only reaches here if stack buffer was insufficient for allocating the coro::generator promise_type context
    std::cerr << e.what() << '\n';
  }

  // reset the coro::generator class to the default promise_type allocator (global new and delete)
  std::cerr << "reset coro::generator again to default pmr allocator (global new and delete)\n";
  coro::reset_default_pmr_mem_pool();

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
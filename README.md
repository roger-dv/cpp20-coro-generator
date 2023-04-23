# Exploring C++20 coroutines

## Introduction

A simple program implementing one specific capability of C++20 coroutines<sup>[1](#fn1)</sup> — a generator. The `coro::generator<T>` template class also supports C++20 range iteration, e.g., `std::ranges::for_each()` can be used to consume an instantiated generator-based function's output.

This generator example generates the Fibonacci Sequence up to some specified ceiling.

## Description

Here is the function - its the use of `co_yield` that make it a C++20 coroutine (as opposed to an ordinary function):

```cpp
    template<arithmetic T>
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
```
The generator function's return value `generator<T>` is an iterator for the type of its template argument. In this programming example it is using the template class `coro::generator<>` which the implementation of is provided in `generator.h`.

**NOTE:** The template class `coro::generator<>` has been customized off of Rainer Grimm's implementation.<sup>[2](#fn2)</sup>

Here is code that consumes generated values from `fibonacci()`:
```cpp
    const auto demo_ceiling = std::numeric_limits<double>::max() / 1'000.0f;

    auto iter = fibonacci(demo_ceiling);

    while(iter.next()) {
      const auto value = iter.getValue();
      std::cout << value << '\n';
    }

```
This will print out 1463 values of the Fibonacci Sequence.

The consuming code and the generator code are executing on the same thread context and yet the `fibonacci()` function enjoys a preserved local scope state as it executes and then resumes from `co_yield`. The generator function just falls out of the loop when the specified ceiling is exceeded to terminate itself - the consuming code will detect this in the `while(iter.next()) {...}` loop condition and fall out of the loop.

## C++17 pmr allocators

The `coro::generator<T>` template class now uses C++17 pmr `memory_resource` allocators. By default the `std::pmr::new_delete_resource` allocator is used which relies on the global `new` and `delete`. It is also `std::pmr::synchronized_pool_resource` so is thread-safe. The function `coro::set_pmr_mem_pool()` can be used to set an alternative or custom pmr allocator. The helper class `coro::fixed_buffer_pmr_allocator` can be used to setup a stack-based, fixed-size buffer (or, say, a data segment fixed-sized buffer).

This program shows two cases of instantiating and invoking a generator where the function `coro::set_pmr_mem_pool()` is used to specify a stack-based pmr allocator.

Then the function `coro::reset_default_pmr_mem_pool()` can be invoked to reset the `coro::generator<T>` template class back to using the default allocator.

## Building the program

The program has been built with cmake and with g++ version 12.1.0 or clang++ version 16.0.0. <sup>[3](#fn3)</sup>

**NOTE:** On my computer I stalled version 16 of clang/llvm from a downloaded `.tar.gz` file; per the directory as to where I *untarred* to, I then had to update these symbolic links to reference the version 16 clang shared libraries:
```
/lib/x86_64-linux-gnu/libc++.so.1.0
/lib/x86_64-linux-gnu/libc++abi.so.1.0
/lib/x86_64-linux-gnu/libunwind.so.1.0
```

<a name="fn1">1</a>: [cppreference.com - Coroutines (C++20)](https://en.cppreference.com/w/cpp/language/coroutines)

<a name="fn2">2</a>: [Rainer Grimm, Concurrency with Modern C++ (Leanpub, 2017 - 2019), 207-209.](https://leanpub.com/concurrencywithmodernc)

<a name="fn3">3</a>: [Clang-LLVM downloads](http://releases.llvm.org/download.html#16.0.0)

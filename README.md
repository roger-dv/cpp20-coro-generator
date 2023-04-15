# Exploring C++20 coroutines

## Introduction

A simple program implementing one specific capability of C++20 coroutines<sup>[1](#fn1)</sup> - a generator.

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

## Building the program

The program can be built with cmake and with Clang++ version 16.0.0. <sup>[3](#fn3)</sup>


<a name="fn1">1</a>: [cppreference.com - Coroutines (C++20)](https://en.cppreference.com/w/cpp/language/coroutines)

<a name="fn2">2</a>: [Rainer Grimm, Concurrency with Modern C++ (Leanpub, 2017 - 2019), 207-209.](https://leanpub.com/concurrencywithmodernc)

<a name="fn3">3</a>: [Clang-LLVM downloads](http://releases.llvm.org/download.html#16.0.0)

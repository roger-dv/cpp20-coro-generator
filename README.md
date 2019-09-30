# Exploring C++20 coroutines (using Clang++ 9.0.0)

## Introduction

A simple program implementing one specific capability of C++20 coroutines - a generator.

This generator example generates the Fibonacci Sequence up to some specified ceiling.

## Description

Here is the function - its the use of `co_yield` and `co_return` that make it a C++20 coroutine (as opposed to an ordinary function):

```cpp
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

```
The generator function's return value `generator<double>` is an iterator for the type of its template argument. In this programming example it is using the template class `coro_exp::generator<>` which the implementation of is provided. However, the C++20 standard is specifying `std::generator<>`<sup>[1](#fn1)</sup>, but it is not available in the Clang++ (9.0.0) experimental implementation yet.

**NOTE:** The template class `coro_exp::generator<>` has been customized off of Rainer Grimm's implementation.<sup>[2](#fn2)</sup>

Here is code that consumes generated values from `fibonacci()`:
```cpp
    const double demo_ceiling = 10E44;

    auto iter = fibonacci(demo_ceiling);

    while(iter.next()) {
      const auto value = iter.getValue();
      std::cout << value << '\n';
    }

```
This will print out 217 values of the Fibonacci Sequence.

The consuming code and the generator code are executing on the same thread context and yet the `fibonacci()` function enjoys a preserved local scope state as it executes and then resumes from `co_yield`. As a coroutine it is then required to execute `co_return` to terminate itself - the consuming code will detect this in the `while(iter.next()) {...}` loop condition and fall out of the loop.

## Building the program

The program can be built with cmake as a `CMakeLists.txt` file is provided. Because it depends on using a Clang C++ compiler that has the experimental implementation of coroutines, will need to insure that the cmake variable `CMAKE_CXX_COMPILER` is suitably defined to `clang++`.

I installed Clang/LLVM from the version 9.0.0 pre-built binary distribution<sup>[3](#fn3)</sup>. On my Ubuntu Linux 18.04 I had to also install `libtinfo5`<sup>[4](#fn4)</sup>, which clang required:
```shell
    sudo apt-get install libtinfo5
```

Adjust accordingly to suit your environment.

<a name="fn1">1</a>: [cppreference.com - Coroutines (C++20)](https://en.cppreference.com/w/cpp/language/coroutines)

<a name="fn2">2</a>: [Rainer Grimm, Concurrency with Modern C++ (Leanpub, 2017 - 2019), 207-209.](https://leanpub.com/concurrencywithmodernc)

<a name="fn3">3</a>: [Clang-LLVM downloads](http://releases.llvm.org/download.html#9.0.0)

<a name="fn4">4</a>: [libtinfo5 package](https://ubuntu.pkgs.org/18.04/ubuntu-main-amd64/libtinfo5_6.1-1ubuntu1_amd64.deb.html)
/**
 * Created by github roger-dv on 10/11/2020
 * Updated by github roger-dv on 04/16/2023
 *
 * Based on example code (but with significant cleanup) found in:
 * Rainer Grimm, Concurrency with Modern C++ (Leanpub, 2017 - 2019), 207-209.
 *
 * Latest updates were to remove use of Clang experimental coroutines
 * and to instead use C++20 official coroutine implementation. Also,
 * C++20 Concepts are used to constrain template types instead of C++11
 * SFINAE type traits. The current implementation has been verified via
 * gcc/g++ v12.1 and with clang++ v16.
 *
 * Compile with gcc/g++:
 * g++ -O3 -std=c++20 -o g++-coroutines gcc-coroutine-example.cpp
 */
#include <limits>
#include <concepts>
#include <coroutine>
#include <iostream>
#include <ranges>

static const auto demo_ceiling1 = std::numeric_limits<unsigned long>::max() / 1'000ul;
static const auto demo_ceiling2 = std::numeric_limits<unsigned long long>::max() / 1'000ul;
static const auto demo_ceiling3 = std::numeric_limits<double>::max() / 1'000.0f;
static const auto demo_ceiling4 = std::numeric_limits<long double>::max() / 1'000.0f;


namespace coro {

  /**
   * General purpose C++20 coroutine generator template class.
   *
   * @tparam T the type of value that the generator returns to the caller
   */
  template<typename T>
  class [[nodiscard]]  generator {
  public:
    struct promise_type;
    using coro_handle_type = std::coroutine_handle<promise_type>;
  private:
    coro_handle_type coro;
  public:
    explicit generator(coro_handle_type h) : coro{h} {}
    generator(const generator &) = delete;            // do not allow copy construction
    generator &operator=(const generator &) = delete; // do not allow copy assignment
    generator(generator &&oth) noexcept : coro{std::move(oth.coro)} {
      oth.coro = nullptr; // insure the other moved handle is null
    }
    generator &operator=(generator &&other) noexcept {
      if (this != &other) {     // ignore assignment to self
        if (coro != nullptr) {  // destroy self current handle
          coro.destroy();
        }
        coro = std::move(other.coro); // move other coro handle into self
        other.coro = nullptr;         // insure other moved handle is null
      }
      return *this;
    }
    ~generator() {
      if (coro) {
        coro.destroy();
        coro = nullptr;
      }
    }

  public: // API
    bool next() {
      if (coro == nullptr || coro.done()) return false; // nothing more to process
      coro.resume();
      return !coro.done();
    }

    std::optional<T> getValue() {
      return coro != nullptr ? std::make_optional(coro.promise().current_value) : std::nullopt;
    }

  public:
    // implementation of above opaque declaration promise_type
    struct promise_type {
    private:
      T current_value{};
      friend class generator;
    public:
      promise_type() = default;
      ~promise_type() = default;
      promise_type(const promise_type&) = delete;
      promise_type(promise_type&&) = delete;
      promise_type &operator=(const promise_type&) = delete;
      promise_type &operator=(promise_type&&) = delete;

      auto get_return_object() {
        return generator{coro_handle_type::from_promise(*this)};
      }

      auto initial_suspend() {
        return std::suspend_always{};
      }

      auto final_suspend() noexcept {
        return std::suspend_always{};
      }

      auto return_void() {
        return std::suspend_never{};
      }

      auto yield_value(T some_value) {
        current_value = some_value;
        return std::suspend_always{};
      }

      void unhandled_exception() {
        std::terminate();
      }
    };
  };

} // namespace coro


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
template <arithmetic T>
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
template <arithmetic T>
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
    for(int i : std::ranges::views::iota(1, 11)) {
      if (iter1.next()) {
        const auto value = iter1.getValue().value();
        print(i, ": bytes", sizeof(value), ':', value, '\n');
      }
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
}
#include <limits>
#include <concepts>
#include <coroutine>
#include <iostream>
#include <ranges>

static const auto demo_ceiling1 = std::numeric_limits<unsigned long>::max() / 1'000ul;
static const auto demo_ceiling2 = std::numeric_limits<unsigned long long>::max() / 1'000ul;
static const auto demo_ceiling3 = std::numeric_limits<double>::max() / 1'000.0f;
static const auto demo_ceiling4 = std::numeric_limits<long double>::max() / 1'000.0f;

template <typename T>
concept arithmetic = std::integral<T> || std::floating_point<T>;

namespace coro_exp {

  template<arithmetic T>
  class generator {
  public:
    struct promise_type;
    using handle_type = std::coroutine_handle<promise_type>;
  private:
    handle_type coro;
  public:
    explicit generator(handle_type h) : coro(h) {}
    generator(const generator &) = delete;
    generator(generator &&oth) noexcept : coro(oth.coro) {
      oth.coro = nullptr;
    }
    generator &operator=(const generator &) = delete;
    generator &operator=(generator &&other) noexcept {
      coro = other.coro;
      other.coro = nullptr;
      return *this;
    }
    ~generator() {
      if (coro) {
        coro.destroy();
      }
    }

    bool next() {
      coro.resume();
      return not coro.done();
    }

    T getValue() {
      return coro.promise().current_value;
    }

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

      auto initial_suspend() {
        return std::suspend_always{};
      }

      auto final_suspend() noexcept {
        return std::suspend_always{};
      }

      auto get_return_object() {
        return generator{handle_type::from_promise(*this)};
      }

      auto return_void() {
        return std::suspend_never{};
      }

      auto yield_value(T some_value) {
        current_value = some_value;
        return std::suspend_always{};
      }

      void unhandled_exception() {
        std::exit(1);
      }
    };
  };

} // coroutn_exp

using coro_exp::generator;

template <std::integral T>
generator<T> f(T start) {
    T i = start;
    while (true)
    co_yield i++;
}

template <arithmetic T>
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
  for(int i : std::ranges::views::iota(1, 11)) {
    if (iter1.next()) {
      const auto value = iter1.getValue();
      print(i, ": bytes", sizeof(value), ':', value, '\n');
    }
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
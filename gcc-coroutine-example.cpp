#include <concepts>
#include <coroutine>
#include <iostream>
#include <ranges>

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

      auto final_suspend() {
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

generator<int> f() {
    int i = 0;
    while (true)
    co_yield i++;
}

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
}

template <class T>
void print(T &&arg) {
  std::cout << arg << ' ';
}
template <class T, class ... Ts>
void print(T &&arg, Ts &&... args) {
  print(std::forward<T>(arg));
  print(std::forward<Ts>(args)...);
}

static const double demo_ceiling = 10E44;

int main() {
  std::cout << "Example using C++20 coroutines to implement Simple Integer and Fibonacci Sequence generators" << '\n';

  std::cout << '\n' << "Simple Integer Sequence Generator" << '\n' << ' ';
  auto iter1 = f();
  for(int i : std::ranges::views::iota(1, 11)) {
    if (iter1.next()) {
      const auto value = iter1.getValue();
      print(i, ':', value, '\n');
    }
  }

  std::cout << '\n' << "Fibonacci Sequence Generator" << '\n' << ' ';
  int i = 1;
  auto iter2 = fibonacci(demo_ceiling);
  while(iter2.next()) {
    const auto value = iter2.getValue();
    print(i++, ':', value, '\n');
  }
}
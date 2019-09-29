//
// Created by rogerv on 9/29/19.
//

#ifndef GENERATOR_H
#define GENERATOR_H

// infiniteDataStream.cpp
#include <experimental/coroutine>
#include <memory>
#include <iostream>

namespace coro_exp {

  template<typename T>
  class generator {
  public:
    struct promise_type;
    using handle_type = std::experimental::coroutine_handle<promise_type>;
  private:
    handle_type coro;
    std::shared_ptr<T> value;
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

    T getValue() {
      return coro.promise().current_value;
    }

    bool next() {
      coro.resume();
      return not coro.done();
    }

    struct promise_type {
      friend class generator;
      promise_type() = default;
      ~promise_type() = default;
      promise_type(const promise_type&) = delete;
      promise_type(promise_type&&) = delete;
      promise_type &operator=(const promise_type&) = delete;
      promise_type &operator=(promise_type&&) = delete;

      auto initial_suspend() {
        return std::experimental::suspend_always{};
      }

      auto final_suspend() {
        return std::experimental::suspend_always{};
      }

      auto get_return_object() {
        return generator{handle_type::from_promise(*this)};
      }

      auto return_void() {
        return std::experimental::suspend_never{};
      }

      auto yield_value(T some_value) {
        current_value = some_value;
        return std::experimental::suspend_always{};
      }

      void unhandled_exception() {
        std::exit(1);
      }

    private:
      T current_value;
    };
  };

} // coroutn_exp

#endif //GENERATOR_H
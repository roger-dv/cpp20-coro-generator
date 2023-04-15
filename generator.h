//
// Created by github roger-dv on 9/29/2019
// Updated by github roger-dv on 4/14/2023
//
// Based on example code (but with significant cleanup) found in:
// Rainer Grimm, Concurrency with Modern C++ (Leanpub, 2017 - 2019), 207-209.
//

#ifndef GENERATOR_H
#define GENERATOR_H

// infiniteDataStream.cpp
#include <coroutine>
#include <memory>
#include <iostream>

namespace coro {

  // restrict this template class to only arithmetic types
  template<typename T>
    requires std::integral<T> || std::floating_point<T>
  class generator {
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
      if (coro != nullptr) {
        coro.destroy();
        coro = nullptr;
      }
    }

  public: // API
    bool next() const {
      if (coro == nullptr || coro.done()) return false; // nothing more to process
      coro.resume();
      return !coro.done();
    }

    std::optional<T> getValue() noexcept {
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

      auto initial_suspend() {
        return std::suspend_always{};
      }

      auto final_suspend() noexcept {
        return std::suspend_always{};
      }

      auto get_return_object() {
        return generator{coro_handle_type::from_promise(*this)};
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

} // coro

#endif //GENERATOR_H

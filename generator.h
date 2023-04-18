/**
 * Created by github roger-dv on 09/29/2019
 * Updated by github roger-dv on 04/16/2023
 *
 * Based on example code (but with significant cleanup) found in:
 * Rainer Grimm, Concurrency with Modern C++ (Leanpub, 2017 - 2019), 207-209.
 *
 * Latest updates were to remove use of Clang experimental coroutines
 * and to instead use C++20 official coroutine implementation. The
 * current implementation has been verified via gcc/g++ v12.1 and with
 * clang++ v16.
 */
#ifndef GENERATOR_H
#define GENERATOR_H

#include <coroutine>
#include <memory>
#include <iostream>

namespace coro {

  /**
   * General purpose C++20 coroutine generator template class.
   *
   * @tparam T the type of value that the generator returns to the caller
   */
  template<typename T>
  class [[nodiscard]] generator {
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
      if (this != &other) { // ignore assignment to self
        if (coro) {         // destroy self current handle
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
    bool next() const {
      if (!coro || coro.done()) return false; // nothing more to process
      coro.resume();
      return !coro.done();
    }

    std::optional<T> getValue() noexcept {
      return coro ? std::make_optional(coro.promise().current_value) : std::nullopt;
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

    struct iterator {
      coro_handle_type hdl; // nullptr
      iterator(coro_handle_type h) : hdl{h} {}
      void getNext() {
        if (hdl) {
          hdl.resume();
          if (hdl.done()) {
            hdl = nullptr;
          }
        }
      }
      std::optional<T> operator*() const {
        return hdl ? std::make_optional(hdl.promise().current_value) : std::nullopt;
      }
      iterator operator++() {
        getNext();
        return *this;
      }
      bool operator==(const iterator& i) const = default;
    };

    iterator begin() const {
      if (!coro || coro.done()) {
        return iterator{nullptr};
      }
      iterator itr{coro};
      itr.getNext();
      return itr;
    }

    iterator end() const {
      return iterator{nullptr};
    }
  };

} // namespace coro

#endif //GENERATOR_H

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
#include <optional>
#include <iostream>
#include <memory_resource>
#include <assert.h>

namespace coro {

  // the default pmr memory resource is a thread-safe allocator that uses the global new and delete
  inline static std::pmr::synchronized_pool_resource mem_pool{std::pmr::new_delete_resource()}; // default allocator

  inline static thread_local std::pmr::memory_resource* pmem_pool = &mem_pool; // never owns any supplied mem resource
  inline static void set_pmr_mem_pool(std::pmr::memory_resource* mem_pool_cust) {
    pmem_pool = mem_pool_cust; // set a custom pmr allocator (but does not take ownership)
  }
  inline static void reset_default_pmr_mem_pool() {
    pmem_pool = &mem_pool; // reset using the default allocator (does not take ownership)
  }

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
    public:
      void* operator new(std::size_t sz) {
        assert(pmem_pool != nullptr);
        return pmem_pool->allocate(sz);
      }
      void operator delete(void* ptr, std::size_t sz) {
        assert(pmem_pool != nullptr);
        pmem_pool->deallocate(ptr, sz);
      }
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

      void return_void() {}

      auto yield_value(T some_value) {
        current_value = some_value;
        return std::suspend_always{};
      }

      void unhandled_exception() {
        std::terminate();
      }
    };

    struct iterator {
      using difference_type [[maybe_unused]] = std::ptrdiff_t;
      using value_type [[maybe_unused]] = T;
      coro_handle_type hdl = nullptr;
      iterator() = default;
      iterator(coro_handle_type h) : hdl{h} {}
      void getNext() {
        if (hdl) {
          hdl.resume();
          if (hdl.done()) {
            hdl = nullptr;
          }
        }
      }
      T& operator*() const {
        assert(hdl);
        return hdl.promise().current_value;
      }
      iterator& operator++() { // pre-incrementable
        getNext();
        return *this;
      }
      void operator ++ (int) { // post-incrementable
       ++*this;
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

  /**
   * Helper class for establishing a pmr memory_resource compliant
   * allocator that allocates from a supplied fixed size buffer, e.g.,
   * such as a buffer allocated on the stack. Allocations are not
   * freed so depends on buffer context being reclaimed when going
   * out of scope (this class does not take ownership of the buffer).
   */
  class fixed_buffer_pmr_allocator : public std::pmr::memory_resource {
  private:
    void * const buf;
    size_t buf_size;
  public:
    const size_t max_buf_size;
    fixed_buffer_pmr_allocator(void* buf, size_t buf_size) : buf(buf), buf_size(buf_size), max_buf_size(buf_size) {}
    fixed_buffer_pmr_allocator() = delete;
    fixed_buffer_pmr_allocator(const fixed_buffer_pmr_allocator&) = delete;
    fixed_buffer_pmr_allocator(fixed_buffer_pmr_allocator&&) = delete;
    fixed_buffer_pmr_allocator& operator=(const fixed_buffer_pmr_allocator&) = delete;
    fixed_buffer_pmr_allocator& operator=(fixed_buffer_pmr_allocator&&) = delete;
  private:
    virtual void* do_allocate(size_t bytes, size_t alignment) {
      if (bytes > buf_size) {
        std::cerr << "requested bytes: " << bytes << ", remaining bytes capacity: " << buf_size << '\n';
        throw std::bad_alloc();
      }
      buf_size -= bytes;
      return buf;
    }
    virtual void do_deallocate(void* p, size_t bytes, size_t alignment) {}
    virtual bool do_is_equal(const memory_resource& __other) const noexcept { return false; }
  };

} // namespace coro

#endif //GENERATOR_H

#pragma once

#include <algorithm>
#include <cstddef>
#include <iostream>
#include <iterator>
#include <memory>
#include <new>
#include <utility>

template <typename T> class vector {

public:
  using value_type = T;

  using reference = T &;
  using const_reference = const T &;

  using pointer = T *;
  using const_pointer = const T *;

  using iterator = pointer;
  using const_iterator = const_pointer;

private:
  std::size_t size_{};
  std::size_t capacity_{};
  pointer data_{};

private:
  void reverse_destroy_n(iterator begin, std::size_t n) noexcept {
    std::destroy(std::reverse_iterator<iterator>(begin + n),
                 std::reverse_iterator<iterator>(begin));
  }

  void uninitialized_copy_n_with_reverse_destroy(iterator dest, iterator from,
                                                 std::size_t n) {
    iterator cur = dest;
    for (std::size_t i = 0; i < n; ++i, ++from, ++cur) {
      try {
        new (cur) value_type(*from);
      } catch (...) {
        reverse_destroy_n(dest, i);
        throw;
      }
    }
  }

  pointer reallocate(std::size_t new_capacity) {
    auto new_data =
        static_cast<pointer>(operator new(new_capacity * sizeof(value_type)));
    try {
      uninitialized_copy_n_with_reverse_destroy(new_data, data_, size_);
    } catch (...) {
      operator delete(new_data);
      throw;
    }
    return new_data;
  }

  void swap_data(pointer new_data, std::size_t new_capacity) {
    reverse_destroy_n(data_, size_);
    operator delete(data_);
    data_ = new_data;
    capacity_ = new_capacity;
  }

  iterator no_const(const_iterator it) { return begin() + (it - begin()); }

public:
  // O(1) nothrow
  vector() noexcept = default;

  // O(1) nothrow
  void swap(vector &other) noexcept {
    std::swap(data_, other.data_);
    std::swap(size_, other.size_);
    std::swap(capacity_, other.capacity_);
  }

  // O(N) strong
  vector(const vector &other)
      : size_(other.size_), capacity_(size_),
        data_(other.capacity_ ? static_cast<pointer>(operator new(
                                    other.size_ * sizeof(value_type)))
                              : nullptr) {
    try {
      uninitialized_copy_n_with_reverse_destroy(data_, other.data_,
                                                other.size_);
    } catch (...) {
      operator delete(data_);
      throw;
    }
  }

  // O(1) strong
  vector(vector &&other)
      : size_(other.size_), capacity_(other.capacity_), data_(other.data_) {
    other.data_ = nullptr;
    other.size_ = 0;
  }

  // O(N) strong
  vector &operator=(const vector &other) {
    if (this != &other) {
      vector copy(other);
      swap(copy);
    }
    return *this;
  }

  // O(1) strong
  vector &operator=(vector &&other) {
    if (this != &other) {
      swap(other);
    }
    return *this;
  }

  // O(N) nothrow
  ~vector() noexcept {
    reverse_destroy_n(begin(), size_);
    operator delete(data_);
  }

  // O(1) nothrow
  reference operator[](size_t index) { return data_[index]; }

  // O(1) nothrow
  const_reference operator[](size_t index) const { return data_[index]; }

  // O(1) nothrow
  pointer data() noexcept { return data_; }

  // O(1) nothrow
  const_pointer data() const noexcept { return data_; }

  // O(1) nothrow
  size_t size() const noexcept { return size_; }

  // O(1) nothrow
  reference front() { return *data_; }

  // O(1) nothrow
  const_reference front() const { return *data_; }

  // O(1) nothrow
  reference back() { return data_[size_ - 1]; }

  // O(1) nothrow
  const_reference back() const { return data_[size_ - 1]; }

  // O(1)* strong
  void push_back(const T &element) {
    if (size_ == capacity_) {
      std::size_t new_capacity = capacity_ * 2 + 1;
      pointer new_data = reallocate(new_capacity);

      try {
        new (&new_data[size_]) value_type(element);
      } catch (...) {
        reverse_destroy_n(new_data, size_);
        operator delete(new_data);
        throw;
      }

      swap_data(new_data, new_capacity);
    } else {
      new (&data_[size_]) value_type(element);
    }

    size_++;
  }

  // O(1) nothrow
  void pop_back() { data_[--size_].~value_type(); }

  // O(1) nothrow
  bool empty() const noexcept { return size() == 0; }

  // O(1) nothrow
  size_t capacity() const noexcept { return capacity_; }

  // O(N) strong
  void reserve(size_t new_capacity) {
    if (new_capacity > capacity_) {
      swap_data(reallocate(new_capacity), new_capacity);
    }
  }

  // O(N) strong
  void shrink_to_fit() {
    if (empty()) {
      data_ = nullptr;
      capacity_ = 0;
    } else if (size_ != capacity_) {
      swap_data(reallocate(size_), size_);
    }
  }

  // O(N) nothrow
  void clear() noexcept {
    reverse_destroy_n(begin(), size_);
    size_ = 0;
  }

  // O(1) nothrow
  iterator begin() noexcept { return data_; }

  // O(1) nothrow
  iterator end() noexcept { return data_ + size_; }

  // O(1) nothrow
  const_iterator begin() const noexcept { return data_; }

  // O(1) nothrow
  const_iterator end() const noexcept { return data_ + size_; }

  // O(N) strong (swap)
  iterator insert(const_iterator pos, const T &value) {
    std::size_t index = pos - begin();
    push_back(value);
    iterator ret_it = begin() + index;
    for (iterator last = std::prev(end()); last != ret_it; --last) {
      std::swap(*last, last[-1]);
    }
    return ret_it;
  }

  // O(N) nothrow(swap)
  iterator erase(const_iterator pos) { return erase(pos, std::next(pos)); }

  // O(N) nothrow(swap)
  iterator erase(const_iterator first, const_iterator last) {
    iterator ret_it = no_const(first), last_ = no_const(last),
             first_ = no_const(first);
    while (last_ != end()) {
      std::swap(*(first_++), *(last_++));
    }
    std::destroy(first_, last_);
    size_ = first_ - begin();
    return ret_it;
  }
};

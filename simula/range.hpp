#ifndef RANGE_HPP
#define RANGE_HPP

#include <cstddef>

template<class T>
class range {
public:
    using value_type = T;
    using reference = T&;
    using const_reference = T const&;
    using pointer = T*;
    using const_pointer = T const*;

    using iterator = pointer;
    using const_iterator = const_pointer;

    range() : begin_(nullptr), end_(nullptr) { }
    range(range const&) = default;
    range(range&&) = default;
    range& operator=(range const&) = default;
    range& operator=(range&&) = default;

    range(iterator begin__, iterator end__) noexcept : begin_(begin__), end_(end__) { }

    iterator begin() noexcept { return begin_; }
    iterator end()   noexcept { return end_; }

    const_iterator begin() const noexcept { return begin_; }
    const_iterator end()   const noexcept { return end_; }

    const_iterator cbegin() const noexcept { return begin_; }
    const_iterator cend()   const noexcept { return end_; }

    reference operator[](std::size_t const index) noexcept { return begin_[index]; }
    reference operator[](std::ptrdiff_t const index) noexcept { return begin_[index]; }

    const_reference operator[](std::size_t const index) const noexcept { return begin_[index]; }
    const_reference operator[](std::ptrdiff_t const index) const noexcept { return begin_[index]; }

    std::size_t size() const noexcept { return static_cast<std::size_t>(end_ - begin_); }
private:
    iterator begin_;
    iterator end_;
};

#endif // RANGE_HPP

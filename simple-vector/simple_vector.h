#pragma once

#include <stdexcept>
#include <initializer_list>
#include <algorithm>
#include "array_ptr.h"
#include <utility>
#include <iterator>

class ReserveProxyObj {
public:
    ReserveProxyObj(size_t size)
            : size_(size)
    { }
    size_t size_;
};

template <typename Type>
class SimpleVector {
private:
    size_t size_ = 0;
    size_t capacity_ = 0;
    ArrayPtr<Type> my_vector_;
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    explicit SimpleVector(size_t size)
            : size_(size), capacity_(size), my_vector_(size)
    {}

    SimpleVector(ReserveProxyObj Obj)
            : size_(0), capacity_(Obj.size_), my_vector_(Obj.size_)
    {}

    SimpleVector(size_t size, const Type& value)
            : size_(size), capacity_(size), my_vector_(size)
    {
        std::fill(begin(), end(), value);
    }

    SimpleVector(std::initializer_list<Type> init)
            : size_(init.size()), capacity_(init.size()), my_vector_(init.size())
    {
        std::copy(init.begin(), init.end(), begin());
    }

    size_t GetSize() const noexcept {
        return size_;
    }

    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    bool IsEmpty() const noexcept {
        return !size_;
    }

    Type& operator[](size_t index) noexcept {
        return my_vector_[index];
    }

    const Type& operator[](size_t index) const noexcept {
        return my_vector_[index];
    }

    Type& At(size_t index) {
        if (index >= size_) {
            throw std::out_of_range("out_of_range");
        }
        return my_vector_[index];
    }

    const Type& At(size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("out_of_range");
        }
        return my_vector_[index];
    }

    void Clear() noexcept {
        size_ = 0;
    }

    void Resize(size_t new_size) {
        if (new_size < size_) {
            size_ = new_size;
            return;
        }
        if (new_size > size_ && new_size <= capacity_) {
            std::generate(end(), begin() + new_size, [](){
                return std::move(Type{});
            });
            size_ = new_size;
            return;
        }
        Reserve(new_size);
        size_ = new_size;
    }

    Iterator begin() noexcept {
        return Iterator{&my_vector_[0]};
    }

    Iterator end() noexcept {
        return Iterator{&my_vector_[size_]};
    }

    ConstIterator begin() const noexcept {
        return ConstIterator{&my_vector_[0]};
    }

    ConstIterator end() const noexcept {
        return ConstIterator{&my_vector_[size_]};
    }

    ConstIterator cbegin() const noexcept {
        return ConstIterator(&my_vector_[0]);
    }

    ConstIterator cend() const noexcept {
        return ConstIterator(&my_vector_[size_]);
    }

    SimpleVector(const SimpleVector& other) {
        size_ = other.size_;
        capacity_ = other.capacity_;
        ArrayPtr<Type> tmp(other.size_);
        std::copy(other.begin(), other.end(), &tmp[0]);
        my_vector_.swap(tmp);
    }

    SimpleVector& operator=(const SimpleVector& rhs) {
        if (this == &rhs) {
            return *this;
        }
        SimpleVector tmp(rhs);
        swap(tmp);
        return *this;
    }

    SimpleVector(SimpleVector&& other) {
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
        my_vector_.swap(other.my_vector_);
    }

    SimpleVector operator=(SimpleVector&& rhs) {
        if (this == &rhs) {
            return *this;
        }
        SimpleVector tmp(std::move(rhs));
        swap(tmp);
        return *this;
    }

    void swap(SimpleVector& other) noexcept {
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
        my_vector_.swap(other.my_vector_);
    }

    void PushBack(const Type& item) {
        if (size_ >= capacity_) {
            Reserve(std::max(size_t(1), capacity_ * 2));
        }
        my_vector_[size_++] = item;
    }

    void PushBack(Type&& item) {
        if (size_ >= capacity_) {
            Reserve(std::max(size_t(1), capacity_ * 2));
        }
        my_vector_[size_++] = std::move(item);
    }

    Iterator Insert(ConstIterator pos, const Type& value) {
        size_t n_pos = std::distance(begin(), const_cast<Iterator>(pos));
        if (size_ < capacity_) {
            std::move(&my_vector_[n_pos], end(), &my_vector_[n_pos + 1]);
            *const_cast<Iterator>(pos) = value;
            ++size_;
            return const_cast<Iterator>(pos);
        }
        Reserve(std::max(size_t(1), capacity_ * 2));
        std::move(&my_vector_[n_pos], end(), &my_vector_[n_pos + 1]);
        ++size_;
        my_vector_[n_pos] = value;
        return Iterator{&my_vector_[n_pos]};
    }

    Iterator Insert(ConstIterator pos, Type&& value) {
        size_t n_pos = std::distance(begin(), const_cast<Iterator>(pos));
        if (size_ < capacity_) {
            std::move(&my_vector_[n_pos], end(), &my_vector_[n_pos + 1]);
            *const_cast<Iterator>(pos) = std::move(value);
            ++size_;
            return const_cast<Iterator>(pos);
        }
        Reserve(std::max(size_t(1), capacity_ * 2));
        std::move(&my_vector_[n_pos], end(), &my_vector_[n_pos + 1]);
        ++size_;
        my_vector_[n_pos] = std::move(value);
        return Iterator{&my_vector_[n_pos]};
    }

    void PopBack() noexcept {
        if (size_)
            --size_;
    }

    Iterator Erase(ConstIterator pos) {
        size_t n_pos = std::distance(begin(), const_cast<Iterator>(pos));
        if (n_pos > size_) {
            throw std::overflow_error("Invalid iterator");
        }
        std::move(&my_vector_[n_pos + 1], end(), const_cast<Iterator>(pos));
        --size_;
        return const_cast<Iterator>(pos);
    }

    void Reserve(size_t new_capacity) {
        if (capacity_ >= new_capacity) {
            return;
        }
        ArrayPtr<Type> tmp(new_capacity);
        std::move(begin(), end(), &tmp[0]);
        std::generate(&tmp[0] + size_, &tmp[0] + new_capacity, [](){
            return std::move(Type{});
        });
        capacity_ = new_capacity;
        my_vector_.swap(tmp);
    }
};

ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
}

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());;
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs == rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return (!(lhs > rhs));
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return rhs < lhs;
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return rhs <= lhs;
}

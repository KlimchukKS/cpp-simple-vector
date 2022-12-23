#pragma once

#include <stdexcept>

template <typename Type>
class ArrayPtr {
private:
    Type *my_vector_ = nullptr;
public:
    ArrayPtr() noexcept = default;

    explicit ArrayPtr(size_t size) {
        my_vector_ = new Type[size]();
    }

    ArrayPtr(const ArrayPtr&) = delete;
    ArrayPtr& operator=(const ArrayPtr&) = delete;


    ArrayPtr(ArrayPtr&& other) {
        swap(other);
    }
    ArrayPtr& operator=(ArrayPtr&& other) {
        swap(other);
        return *this;
    }

    void swap(ArrayPtr& other) {
        std::swap(my_vector_, other.my_vector_);
    }

    ~ArrayPtr() {
        delete[] my_vector_;
        my_vector_ = nullptr;
    }

    explicit operator bool() const {
        return my_vector_;
    }

    Type& operator[](size_t index) noexcept {
        return my_vector_[index];
    }

    const Type& operator[](size_t index) const noexcept {
        return my_vector_[index];
    }
};

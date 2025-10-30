#ifndef MYVECTOR_H
#define MYVECTOR_H

#include <algorithm>
#include <initializer_list>
#include <stdexcept>
#include <iostream>

template<typename T>
class MyVector {
private:
    T* data_;
    size_t size_;
    size_t capacity_;

    void reallocate(size_t new_capacity) {
        auto new_data = new T[new_capacity];
        
        for (size_t i = 0; i < size_; ++i) {
            new_data[i] = std::move(data_[i]);
        }
        
        delete[] data_;
        data_ = new_data;
        capacity_ = new_capacity;
    }

public:
    class Iterator {
    private:
        T* ptr_;
    
    public:
        explicit Iterator(T* ptr) : ptr_(ptr) {}
        
        T& operator*() const { return *ptr_; }
        T* operator->() const { return ptr_; }
        
        Iterator& operator++() {
            ++ptr_;
            return *this;
        }
        
        Iterator operator++(int) {
            Iterator temp = *this;
            ++ptr_;
            return temp;
        }
        
        bool operator==(const Iterator& other) const = default;
    };

    MyVector() : data_(nullptr), size_(0), capacity_(0) {}
    
    explicit MyVector(size_t count, const T& value = T()) 
        : data_(new T[count]), size_(count), capacity_(count) {
        for (size_t i = 0; i < size_; ++i) {
            data_[i] = value;
        }
    }
    
    MyVector(std::initializer_list<T> init) 
        : data_(new T[init.size()]), size_(init.size()), capacity_(init.size()) {
        size_t i = 0;
        for (const auto& item : init) {
            data_[i++] = item;
        }
    }
    
    MyVector(const MyVector& other) 
        : data_(new T[other.capacity_]), size_(other.size_), capacity_(other.capacity_) {
        for (size_t i = 0; i < size_; ++i) {
            data_[i] = other.data_[i];
        }
    }
    
    MyVector(MyVector&& other) noexcept 
        : data_(other.data_), size_(other.size_), capacity_(other.capacity_) {
        other.data_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
    }
    
    MyVector& operator=(const MyVector& other) {
        if (this != &other) {
            delete[] data_;
            
            data_ = new T[other.capacity_];
            size_ = other.size_;
            capacity_ = other.capacity_;
            
            for (size_t i = 0; i < size_; ++i) {
                data_[i] = other.data_[i];
            }
        }
        return *this;
    }
    
    MyVector& operator=(MyVector&& other) noexcept {
        if (this != &other) {
            delete[] data_;
            
            data_ = other.data_;
            size_ = other.size_;
            capacity_ = other.capacity_;
            
            other.data_ = nullptr;
            other.size_ = 0;
            other.capacity_ = 0;
        }
        return *this;
    }
    
    ~MyVector() {
        delete[] data_;
    }
    
    T& operator[](size_t index) {
        if (index >= size_) {
            throw std::out_of_range("Index out of range");
        }
        return data_[index];
    }
    
    const T& operator[](size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("Index out of range");
        }
        return data_[index];
    }
    
    T& at(size_t index) {
        if (index >= size_) {
            throw std::out_of_range("Index out of range");
        }
        return data_[index];
    }
    
    const T& at(size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("Index out of range");
        }
        return data_[index];
    }
    
    T& front() {
        if (empty()) {
            throw std::out_of_range("Vector is empty");
        }
        return data_[0];
    }
    
    const T& front() const {
        if (empty()) {
            throw std::out_of_range("Vector is empty");
        }
        return data_[0];
    }
    
    T& back() {
        if (empty()) {
            throw std::out_of_range("Vector is empty");
        }
        return data_[size_ - 1];
    }
    
    const T& back() const {
        if (empty()) {
            throw std::out_of_range("Vector is empty");
        }
        return data_[size_ - 1];
    }
    
    T* data() noexcept { return data_; }
    const T* data() const noexcept { return data_; }
    
    Iterator begin() noexcept { return Iterator(data_); }
    Iterator end() noexcept { return Iterator(data_ + size_); }
    
    bool empty() const noexcept { return size_ == 0; }
    size_t size() const noexcept { return size_; }
    size_t capacity() const noexcept { return capacity_; }
    
    void reserve(size_t new_capacity) {
        if (new_capacity > capacity_) {
            reallocate(new_capacity);
        }
    }
    
    void shrink_to_fit() {
        if (size_ < capacity_) {
            reallocate(size_);
        }
    }
    
    void clear() noexcept {
        size_ = 0;
    }
    
    void push_back(const T& value) {
        if (size_ >= capacity_) {
            reserve(capacity_ == 0 ? 1 : capacity_ * 2);
        }
        data_[size_++] = value;
    }
    
    void push_back(T&& value) {
        if (size_ >= capacity_) {
            reserve(capacity_ == 0 ? 1 : capacity_ * 2);
        }
        data_[size_++] = std::move(value);
    }
    
    template<typename... Args>
    void emplace_back(Args&&... args) {
        if (size_ >= capacity_) {
            reserve(capacity_ == 0 ? 1 : capacity_ * 2);
        }
        data_[size_++] = T(std::forward<Args>(args)...);
    }
    
    void pop_back() {
        if (!empty()) {
            --size_;
        }
    }
    
    void resize(size_t count, const T& value = T()) {
        if (count > capacity_) {
            reserve(count);
        }
        
        if (count > size_) {
            for (size_t i = size_; i < count; ++i) {
                data_[i] = value;
            }
        }
        size_ = count;
    }
    
    void swap(MyVector& other) noexcept {
        std::swap(data_, other.data_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }
    
    void print() const {
        std::cout << "[";
        for (size_t i = 0; i < size_; ++i) {
            if constexpr (std::is_same_v<T, QString>) {
                std::cout << data_[i].toStdString();
            } else {
                std::cout << data_[i];
            }
            if (i != size_ - 1) {
                std::cout << ", ";
            }
        }
        std::cout << "]" << std::endl;
    }
};

#endif

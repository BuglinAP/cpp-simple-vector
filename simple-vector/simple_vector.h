#pragma once

#include <cassert>
#include <initializer_list>
#include <algorithm>
#include <stdexcept>
#include "array_ptr.h"

class ReserveProxyObj
{
public:
    ReserveProxyObj(size_t capacity)
        : capacity_to_reserve_(capacity)
    {
    }
    size_t capacity_to_reserve_;
};

template <typename Type>
class SimpleVector
{
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    // Создаёт вектор из size элементов, инициализированных значением по умолчанию
    explicit SimpleVector(size_t size)
        : array_vector_(size)
        , size_(size)
        , capacity_(size)
    {
        std::fill(begin(), end(), Type());
    }

    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type& value)
        : array_vector_(size)
        , size_(size)
        , capacity_(size)
    {
        for (size_t i = 0; i < size; ++i)
        {
            array_vector_[i] = value;
        }
    }

    // Создаёт вектор из std::initializer_list
    SimpleVector(std::initializer_list<Type> init)
        : array_vector_(init.size())
        , size_(init.size())
        , capacity_(init.size())
    {
        std::copy(init.begin(), init.end(), begin());
    }

    SimpleVector(ReserveProxyObj other)
    {
        Reserve(other.capacity_to_reserve_);
    }

    SimpleVector(const SimpleVector& other)
    {
        SimpleVector temp(other.size_);
        std::copy(other.begin(), other.end(), temp.begin());
        swap(temp);
    }

    SimpleVector(SimpleVector&& other)
        : array_vector_(std::move(other.array_vector_))
    {
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

    SimpleVector& operator=(const SimpleVector& rhs)
    {
        if (*this == rhs)
        {
            return *this;
        }

        SimpleVector temp(rhs);
        swap(temp);
        return *this;
    }

    SimpleVector& operator=(SimpleVector&& rhs)
    {
        if (*this == rhs)
        {
            return *this;
        }
        array_vector_.swap(rhs.array_vector_);
        size_ = std::move(rhs.size_);
        capacity_ = std::move(rhs.size_);

        return *this;
    }

    // Возвращает количество элементов в массиве
    size_t GetSize() const noexcept
    {
        return size_;
    }

    // Возвращает вместимость массива
    size_t GetCapacity() const noexcept
    {
        return capacity_;
    }

    // Сообщает, пустой ли массив
    bool IsEmpty() const noexcept
    {
        return size_ == 0;
    }

    // Возвращает ссылку на элемент с индексом index
    Type& operator[](size_t index) noexcept
    {
        return array_vector_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    const Type& operator[](size_t index) const noexcept
    {
        return array_vector_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    Type& At(size_t index)
    {
        if (index >= size_)
        {
            throw std::out_of_range("out of range");
        }
        return array_vector_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    const Type& At(size_t index) const
    {
        if (index >= size_)
        {
            throw std::out_of_range("out of range");
        }
        return array_vector_[index];
    }

    // Обнуляет размер массива, не изменяя его вместимость
    void Clear() noexcept
    {
        size_ = 0;
    }

    void Reserve(size_t new_capacity)
    {
        if (new_capacity > capacity_)
        {
            ArrayPtr<Type> temp(new_capacity);
            std::move(begin(), end(), temp.Get());
            array_vector_.swap(temp);
            capacity_ = new_capacity;
        }
    }

    // Изменяет размер массива.
    // При увеличении размера новые элементы получают значение по умолчанию для типа Type
    void Resize(size_t new_size)
    {
        if (new_size < size_)
        {
            size_ = new_size;
        }
        if (new_size > capacity_)
        {
            size_t new_capacity = std::max(new_size, capacity_ * 2);
            Reserve(new_capacity);
            std::generate(end(), begin() + new_size, [] { return Type(); });
            size_ = new_size;
        }
        else if (new_size > size_)
        {
            std::generate(end(), begin() + new_size, [] { return Type(); });
            size_ = new_size;
        }
    }

    void PushBack(const Type& item)
    {
        if (IsEmpty())
        {
            if (capacity_ == 0)
            {
                size_t new_capacity;
                Reserve(new_capacity);
            }
            array_vector_[0] = item;
            size_ = 1;
            return;
        }
        else if (size_ == capacity_)
        {
            size_t new_capacity = capacity_ * 2;
            Reserve(new_capacity);
        }
        array_vector_[size_] = item;
        ++size_;
    }

    void PushBack(Type&& item)
    {
        if (IsEmpty())
        {
            if (capacity_ == 0)
            {
                Reserve(10);
            }
            array_vector_[0] = std::move(item);
            size_ = 1;
            return;
        }
        else if (size_ == capacity_)
        {
            size_t new_capacity = capacity_ * 2;
            Reserve(new_capacity);
        }
        array_vector_[size_] = std::move(item);
        ++size_;
    }

    Iterator Insert(ConstIterator pos, const Type& value)
    {
        assert(pos >= begin() && pos <= end());
        if (pos == end())
        {
            PushBack(value);
            return end() - 1;
        }
        else if (size_ == capacity_)
        {
            size_t new_capacity = capacity_ * 2;
            Reserve(new_capacity);
        }
        std::copy_backward((Iterator)pos, end(), end() + 1);
        *((Iterator)pos) = value;
        ++size_;
        return (Iterator)pos;
    }

    Iterator Insert(ConstIterator pos, Type&& value)
    {
        assert(pos >= begin() && pos <= end());
        if (pos == end())
        {
            PushBack(std::move(value));
            return end() - 1;
        }
        else if (size_ == capacity_)
        {
            size_t new_capacity = capacity_ * 2;
            Reserve(new_capacity);
        }
        std::move_backward((Iterator)pos, end(), end() + 1);
        *((Iterator)pos) = std::move(value);
        ++size_;
        return (Iterator)pos;
    }

    void PopBack() noexcept
    {
        if (!IsEmpty())
        {
            --size_;
        }
    }

    Iterator Erase(ConstIterator pos) 
    {
        std::move(std::next(Iterator(pos)), end(), Iterator(pos));
        --size_;
        return Iterator(pos);
    }

    void swap(SimpleVector& other) noexcept
    {
        array_vector_.swap(other.array_vector_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

    // Возвращает итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator begin() noexcept
    {
        return array_vector_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator end() noexcept
    {
        return array_vector_.Get() + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator begin() const noexcept
    {
        return array_vector_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator end() const noexcept
    {
        return array_vector_.Get() + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cbegin() const noexcept
    {
        return array_vector_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cend() const noexcept
    {
        return array_vector_.Get() + size_;
    }
private:
    ArrayPtr<Type> array_vector_;
    size_t size_ = 0;
    size_t capacity_ = 0;
};

ReserveProxyObj Reserve(size_t capacity_to_reserve)
{
    return ReserveProxyObj(capacity_to_reserve);
}

template <typename Type>
void swap(SimpleVector<Type>& lhs, SimpleVector<Type>& rhs) noexcept
{
    lhs.swap(rhs);
}

template<typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs)
{
    return lhs.GetSize() == rhs.GetSize() && std::equal(lhs.cbegin(), lhs.cend(), rhs.cbegin());
}

template<typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs)
{
    return !(lhs == rhs);
}

template<typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs)
{
    return std::lexicographical_compare(lhs.cbegin(), lhs.cend(), rhs.cbegin(), rhs.cend());
}

template<typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs)
{
    return (lhs < rhs || lhs == rhs);
}

template<typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs)
{
    return (rhs < lhs);
}

template<typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs)
{
    return !(lhs < rhs);
}
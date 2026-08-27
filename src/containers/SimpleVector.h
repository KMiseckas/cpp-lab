#pragma once

#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <memory>
#include <new>
#include <type_traits>
#include <utility>

/**
 * @brief Simple vector. Uses default std::allocator.
 * - Small API surface.
 * - Min capacity 8 by default.
 * - Capacity increased *2 when size is about to increase over current capacity.
 * 
 * @tparam T 
 */
template<typename T>
class SimpleVector
{
public:
    SimpleVector(std::size_t capacity = k_minCapacity)
    {
        reserve(capacity < k_minCapacity ? k_minCapacity : capacity);
    }

    ~SimpleVector() 
    {
        if (m_data != nullptr) 
        {
            for (std::size_t i = 0; i < m_size; ++i) 
            {
                (m_data + i)->~T();
            }

            m_allocator.deallocate(m_data, m_capacity);
        }
    }

    SimpleVector(const SimpleVector<T>& other)
    {
        reserve(other.m_capacity);

        std::size_t i{0U};
        try
        {
            for(; i < other.m_size; ++i)
            {
                push_back_imp(other.m_data[i]);
            }
        }
        catch(...)
        {
            for(std::size_t j = 0; j < i; ++j)
            {
                (m_data + j)->~T();
            }

            m_allocator.deallocate(m_data, m_capacity);
            m_data = nullptr;
            m_capacity = 0;
            m_size = 0;

            throw;
        }
    }

    SimpleVector(SimpleVector<T>&& other) noexcept
    {
        m_data = other.m_data;
        m_size = other.m_size;
        m_capacity = other.m_capacity;

        other.m_data = nullptr;
        other.m_capacity = 0;
        other.m_size = 0;
    }

    SimpleVector<T>& operator=(const SimpleVector<T>& other)
    {
        if(&other == this)
        {
            return *this;
        }

        SimpleVector temp(other);
        std::swap(m_data, temp.m_data);
        std::swap(m_size, temp.m_size);
        std::swap(m_capacity, temp.m_capacity);

        return *this;
    }

    SimpleVector<T>& operator=(SimpleVector<T>&& other) noexcept
    {
        if(&other == this)
        {
            return *this;
        }

        if (m_data != nullptr) 
        {
            for (std::size_t i = 0; i < m_size; ++i) 
            {
                (m_data + i)->~T();
            }

            m_allocator.deallocate(m_data, m_capacity);
        }

        m_data = other.m_data;
        m_size = other.m_size;
        m_capacity = other.m_capacity;

        other.m_data = nullptr;
        other.m_capacity = 0;
        other.m_size = 0;
        return *this;
    }

    void push_back(const T& value)
    {
        if(!capacityCheck())
        {
            return;
        }

        push_back_imp(value);
    }

    void push_back(T&& value)
    {
        if(!capacityCheck())
        {
            return;
        }
        
        push_back_imp(std::move(value));
    }

    void pop_back()
    {
        if(m_size == 0)
        {
            return;
        }

        --m_size;
        (m_data + m_size)->~T();
    }

    void reserve(std::size_t capacity)
    {
        if(capacity <= m_capacity)
        {
            return;
        }

        T* newData = m_allocator.allocate(capacity);
        std::size_t i{0U};

        try
        {
            for (; i < m_size; ++i) 
            {
                ::new (newData + i) T(std::move_if_noexcept(m_data[i]));
            }
        }
        catch(...)
        {
            for (std::size_t j = 0; j < i; ++j) 
            {
                (newData + j)->~T();
            }

            m_allocator.deallocate(newData, capacity);
            throw;
        }

        for(std::size_t i = 0; i < m_size; ++i)
        {
            (m_data + i)->~T();
        }

        if(m_data != nullptr)
        {
            m_allocator.deallocate(m_data, m_capacity);
        }

        m_data = newData;
        m_capacity = capacity;
    }

    T& operator[](std::size_t index)
    {
        assert(index < m_size);
        return m_data[index];
    }

    const T& operator[](std::size_t index) const
    {
        assert(index < m_size);
        return m_data[index];
    }

    std::size_t size() const
    {
        return m_size;
    }

    bool empty() const
    {
        return m_size == 0;
    }

    std::size_t capacity() const
    {
        return m_capacity;
    }

private:

    bool capacityCheck()
    {
        if(m_size < m_capacity)
        {
            return true;
        }

        constexpr auto sizeTMax = std::numeric_limits<std::size_t>::max();
        if(sizeTMax == m_capacity)
        {
            assert(false); // Reached max allowed element count.
            return false;
        }

        const bool overflow = m_capacity > (sizeTMax / k_resizeMultiplier);
        const std::size_t newCapacity = overflow ? sizeTMax : m_capacity * k_resizeMultiplier;
        reserve(newCapacity);
        return true;
    }

    template<typename U>
    void push_back_imp(U&& value) 
    {
        ::new (m_data + m_size)T(std::forward<U>(value));
        ++m_size;
    }

    static constexpr std::size_t k_minCapacity{8U};
    static constexpr std::size_t k_resizeMultiplier{2};

    T *m_data{nullptr};

    std::size_t m_capacity{0U};
    std::size_t m_size{0U};

    std::allocator<T> m_allocator;

};
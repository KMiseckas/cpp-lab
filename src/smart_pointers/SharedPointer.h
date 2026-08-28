#pragma once

#include <cstddef>
#include <atomic>
#include <memory>
#include <utility>

template<typename T>
class SharedPointer;

template<typename T, typename... Args>
SharedPointer<T> make_shared(Args&&... args);

/**
 * @brief Shared pointer, simple implementation.
 * - Control block is called `SharedData`. TODO: Rename to ControlBlock in future.
 * - Use `make_shared` to create a populated SharedPointer instance.
 * - Atomics for safe thread decrement/increment. Decrement with acq_rel memory ordering.
 */
template<typename T>
class SharedPointer
{
    struct SharedData
    {
        ~SharedData()
        {
            if(value != nullptr)
            {
                delete value;
                value = nullptr;
            }
        }

        T* value{nullptr};
        std::atomic<std::size_t> refCount{0U};
    };

    // Make friend of make_shared function to be able to access shared data block.
    template<typename U, typename... Args>
    friend SharedPointer<U> make_shared(Args&&... args);

public:

    SharedPointer() = default;

    ~SharedPointer()
    {
        release();
    }

    SharedPointer(const SharedPointer<T>& other)
    {
        m_data = other.m_data;

        if(m_data != nullptr)
        {
            m_data->refCount.fetch_add(1, std::memory_order_relaxed);
        }
    }

    SharedPointer(SharedPointer<T>&& other) noexcept
    {
        //Simple set.
        m_data = other.m_data;
        other.m_data = nullptr;
    }

    SharedPointer<T>& operator=(const SharedPointer<T>& other)
    {
        if(this == &other)
        {
            return *this;
        }

        auto* data = other.m_data;

        if(data != nullptr)
        {
            data->refCount.fetch_add(1, std::memory_order_relaxed);
        }

        release(); // Release any current resources.
        m_data = data;

        return *this;
    }

    SharedPointer<T>& operator=(SharedPointer<T>&& other) noexcept
    {
        if(this == &other)
        {
            return *this;
        }

        release(); // Release old resources if any.

        m_data = other.m_data;
        other.m_data = nullptr;

        return *this;
    }

    void release()
    {
        if (m_data == nullptr) 
        {
            return;
        }

        if (m_data->refCount.fetch_sub(1, std::memory_order_acq_rel) == 1)
        {
            delete m_data;
        }

        m_data = nullptr;
    }

    T* get() const
    {
        return m_data ? m_data->value : nullptr;
    }

    const T* operator->() const
    {
        return m_data ? m_data->value : nullptr;
    }

    T* operator->()
    {
        return m_data ? m_data->value : nullptr;
    }

    const T& operator*() const
    {
        return *(m_data->value);
    }

    T& operator*()
    {
        return *(m_data->value);
    }

private:

    SharedData* m_data{nullptr};
};

template<typename T, typename ...Args>
SharedPointer<T> make_shared(Args&&... args)
{
    SharedPointer<T> sharedPtr;
    sharedPtr.m_data = new typename SharedPointer<T>::SharedData(new T(std::forward<Args>(args)...), 1);
    return sharedPtr;
}
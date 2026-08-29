#pragma once

#include <utility>

#include <red3lib/IRTTIType.hpp>

namespace red3lib
{
// Owns a value the engine built for us and gives it back through the engine's
// own destructor for that type.
//
// Natives hand out engine allocations: an out-parameter array's storage, a
// reference on every handle inside it, the buffer behind a returned String.
// Nothing in CFunction releases any of that - it cannot, because it has to hand
// the value to the caller - so ownership lands here.
//
// The type comes from the value's own CProperty, which is why this works for
// anything without knowing what it is:
//
//     owned<String> version(fn->return_type(), fn->call_native<String>(game));
//     use(*version);   // released at end of scope
template<typename T>
class [[nodiscard]] owned
{
public:
    owned() noexcept = default;

    owned(const IRTTIType* type, const T& value) noexcept
        : m_type(type)
        , m_value(value)
    {
    }

    ~owned() noexcept
    {
        release();
    }

    owned(const owned&) = delete;
    owned& operator=(const owned&) = delete;

    owned(owned&& other) noexcept
        : m_type(std::exchange(other.m_type, nullptr))
        , m_value(std::exchange(other.m_value, T{}))
    {
    }

    owned& operator=(owned&& other) noexcept
    {
        if (this != &other)
        {
            release();
            m_type = std::exchange(other.m_type, nullptr);
            m_value = std::exchange(other.m_value, T{});
        }

        return *this;
    }

    // Hands the value back to its type, which frees whatever it owns. The
    // engine's destructors also clear the storage, so the value is left empty
    // rather than dangling.
    void release() noexcept
    {
        if (m_type)
        {
            m_type->destruct(&m_value);
            m_type = nullptr;
        }

        m_value = T{};
    }

    [[nodiscard]] T& operator*() noexcept
    {
        return m_value;
    }

    [[nodiscard]] const T& operator*() const noexcept
    {
        return m_value;
    }

    [[nodiscard]] T* operator->() noexcept
    {
        return &m_value;
    }

    [[nodiscard]] const T* operator->() const noexcept
    {
        return &m_value;
    }

    // False when there is nothing to release - a value the engine never built.
    [[nodiscard]] explicit operator bool() const noexcept
    {
        return m_type != nullptr;
    }

private:
    const IRTTIType* m_type = nullptr;
    T m_value{};
};
} // namespace red3lib

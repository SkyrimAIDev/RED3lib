#pragma once

#include <cstdint>
#include <string_view>

namespace red3lib
{
class [[nodiscard]] CNameHash
{
public:
    // Must be defined here, not in CNameHash.cpp: constexpr implies inline, so
    // an out-of-line definition emits no linkable symbol and every consumer
    // that default-constructs a CNameHash fails to link.
    constexpr CNameHash() noexcept
        : m_index(0)
    {
    }

    CNameHash(const wchar_t* text);

    // TODO: Implement other constructors.

    constexpr CNameHash(const CNameHash&) noexcept = default;
    constexpr CNameHash& operator=(const CNameHash&) noexcept = default;

    constexpr CNameHash(CNameHash&&) noexcept = default;
    constexpr CNameHash& operator=(CNameHash&&) noexcept = default;

    constexpr ~CNameHash() noexcept = default;

    // CNamePool::Add interns: it hashes the name (FNV-1a, modulo 8191 buckets)
    // and returns the existing index when the name is already pooled. So equal
    // names always carry equal indices, and comparing indices is exact - no
    // need to materialise either name as a string.
    [[nodiscard]] constexpr bool operator==(const CNameHash& rhs) const noexcept = default;

    [[nodiscard]] std::wstring_view to_wide() const;

private:
    std::uint32_t m_index; // 00
};
} // namespace red3lib

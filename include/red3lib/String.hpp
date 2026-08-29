#pragma once

#include <cstddef>
#include <cstdint>

#include <red3lib/detail/Asserts.hpp>

namespace red3lib
{
// REDengine 3 "String": a UTF-16 buffer plus an element count.
//
// Size (0xC) and alignment (4) were read from CSimpleRTTITypeString's RTTI
// vtable (slot 2 = GetSize, slot 3 = GetAlignment) in witcher3.exe 4.0.0.103190
// by tools/w3offsets. Two things pin down the shape:
//
//   * "StringAnsi" reports the same 0xC size, so this is one container template
//     parameterised on the character type, not two distinct layouts.
//   * The StrLen implementation allocates "count * 2" bytes with alignment 2,
//     so the buffer is UTF-16 and "size" counts characters, not bytes.
//
// That makes it structurally identical to TDynArray<wchar_t>.
//
// "size" INCLUDES the terminating null. Read out of a live call:
// GetApplicationVersion() returned size = 7 over the buffer
// 'v' ' ' '4' '.' '0' '4' NUL - six visible characters and the null, counted.
//
// Ownership: the engine reads from a caller-owned buffer without retaining or
// freeing it. Confirmed by passing a static buffer through Pause() and
// IsPausedForReason() and getting a match back.
#pragma pack(push, 4)
struct [[nodiscard]] String
{
    wchar_t* data;      // 00
    std::uint32_t size; // 08 - wchar_t count INCLUDING the terminating null
};
#pragma pack(pop)
RED3LIB_ASSERT_SIZE(String, 0xC);
RED3LIB_ASSERT_OFFSET(String, data, 0x0);
RED3LIB_ASSERT_OFFSET(String, size, 0x8);

// Wrap caller-owned memory without copying. `length` follows the engine's own
// convention and counts the terminating null, so a null-terminated buffer of N
// visible characters passes N + 1. The buffer must outlive every use of the
// returned String.
[[nodiscard]] constexpr String borrow_string(wchar_t* text, std::uint32_t length) noexcept
{
    return String{text, length};
}

// Same, for a null-terminated array, sizing it the way the engine does.
template<std::size_t N>
[[nodiscard]] constexpr String borrow_string(wchar_t (&text)[N]) noexcept
{
    return String{text, static_cast<std::uint32_t>(N)};
}
} // namespace red3lib

#pragma once

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
// TODO: confirm whether "size" includes the terminating null. TODO: confirm
// ownership - passing a String that borrows caller memory is expected to be
// safe for a read-only parameter, but not if the callee retains or frees it.
#pragma pack(push, 4)
struct [[nodiscard]] String
{
    wchar_t* data;      // 00
    std::uint32_t size; // 08 - wchar_t count, not bytes
};
#pragma pack(pop)
RED3LIB_ASSERT_SIZE(String, 0xC);
RED3LIB_ASSERT_OFFSET(String, data, 0x0);
RED3LIB_ASSERT_OFFSET(String, size, 0x8);

// Wrap caller-owned memory without copying. The buffer must outlive every use
// of the returned String, and the engine must not take ownership of it - see
// the ownership TODO above.
[[nodiscard]] constexpr String borrow_string(wchar_t* text, std::uint32_t length) noexcept
{
    return String{text, length};
}
} // namespace red3lib

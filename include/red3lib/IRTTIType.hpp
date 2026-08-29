#pragma once

#include <cstdint>

#include <red3lib/CNameHash.hpp>
#include <red3lib/IRTTIBaseObject.hpp>

namespace red3lib
{
struct IRTTIType : IRTTIBaseObject
{
    // Slot 1 returns a POINTER to the name, not the name by value. Every
    // implementation is `lea rax, [rcx + N] ; ret`, where N is wherever that
    // type keeps its CNameHash - 0x2C for CClass, 0x14 for CRTTIArrayType,
    // 0x10 for CRTTIHandleType. Reading it as a returned value would treat the
    // pointer as an index.
    virtual const CNameHash* name() const noexcept = 0; // slot 1

    // `mov eax, [rcx + 0x60] ; ret` for CClass; the fixed-size types return a
    // constant - 12 for CRTTIArrayType, 8 for CRTTIHandleType.
    virtual std::uint32_t size() const noexcept = 0;      // slot 2
    virtual std::uint32_t alignment() const noexcept = 0; // slot 3
};
RED3LIB_ASSERT_SIZE(IRTTIType, 0x8);
} // namespace red3lib

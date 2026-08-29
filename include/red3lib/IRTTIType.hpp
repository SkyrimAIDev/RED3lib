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

    // A small constant per type class: 3 for CRTTIArrayType, 6 for
    // CRTTIPointerType, 7 for CRTTIHandleType. The meaning of the individual
    // values beyond that has not been established.
    virtual std::uint32_t kind() const noexcept = 0; // slot 4

    // Two predicates returning a fixed bool per type class. Declared so the
    // slots below them land correctly; not otherwise understood.
    virtual bool unknown_predicate_5() const noexcept = 0; // slot 5
    virtual bool unknown_predicate_6() const noexcept = 0; // slot 6

    // Construct and destruct a value of this type IN PLACE, with the value's
    // address in the second argument. This is the engine's own cleanup path and
    // it is what makes engine-allocated results releasable:
    //
    //   CRTTIPointerType  destruct = `mov qword ptr [rdx], 0` - owns nothing
    //   CRTTIHandleType   destruct = `lock xadd dword ptr [rcx], -1` and the
    //                     release that follows - it owns a reference
    //   CRTTIArrayType    destruct = destruct each element, then free storage
    //
    // So anything the engine built for us - an out-parameter array, a returned
    // String, a handle - is released by handing it back to its own type.
    virtual void construct(void* value) const = 0; // slot 7
    virtual void destruct(void* value) const = 0;  // slot 8
};
RED3LIB_ASSERT_SIZE(IRTTIType, 0x8);
} // namespace red3lib

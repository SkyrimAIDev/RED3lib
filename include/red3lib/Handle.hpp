#pragma once

#include <cstdint>

#include <red3lib/detail/Asserts.hpp>

namespace red3lib
{
// A refcounted engine handle.
//
// The eight bytes stored in a field - or returned by a native whose RTTI return
// type is CRTTIHandleType - point at a CONTROL BLOCK, not at the object. Read
// out of a live call to CCommonGame::GetJournalManager():
//
//     +0x00  refcount        (observed as 2 and later 7 for the same handle)
//     +0x08  the object      (vptr resolved to CWitcherJournalManager)
//     +0x10  ...
//
// Treating the handle itself as an object pointer reads the refcount as a vptr,
// which is what made object returns look broken.
template<typename T>
struct [[nodiscard]] Handle
{
    struct control_block
    {
        std::uint64_t references; // 00
        T* instance;              // 08
    };

    control_block* block;

    [[nodiscard]] T* get() const noexcept
    {
        return block ? block->instance : nullptr;
    }

    [[nodiscard]] T* operator->() const noexcept
    {
        return get();
    }

    [[nodiscard]] explicit operator bool() const noexcept
    {
        return get() != nullptr;
    }
};
static_assert(sizeof(Handle<int>) == 0x8, "a handle is one pointer wide");
} // namespace red3lib

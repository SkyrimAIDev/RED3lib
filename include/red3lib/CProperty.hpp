#pragma once

#include <cstdint>

#include <red3lib/CNameHash.hpp>
#include <red3lib/IRTTIBaseObject.hpp>
#include <red3lib/detail/Asserts.hpp>

namespace red3lib
{
struct IRTTIType;

// A variable slot: a type plus where the value lives relative to some base -
// the stack frame's params, its locals, or an object.
//
// Offsets read out of the running game via the VM's variable-load opcodes
// (0x0F/0x10/0x11), which do:
//
//     mov edx, [property + 0x20]   ; offset
//     mov ecx, [property + 0x24]   ; flags
//     lea r8,  [base + offset]     ; address of the value
//     mov rcx, [property + 0x08]   ; type
//     jmp [type_vtable + 0x50]     ; the type copies the value out
//
// Cross-checked against live parameter lists: IsPausedForReason's single String
// parameter is at offset 0 in a 12-byte frame, and IsSpecificRumbleActive's two
// floats are at 0 and 4 in an 8-byte frame.
struct CProperty : IRTTIBaseObject
{
    IRTTIType* type;      // 08
    // Read out of the engine: resolving the dword at +0x10 as a name index
    // yields CR4Game's real property names - horseCamera, telemetryScriptProxy,
    // secondScreenScriptProxy - which the RTTI dump lists independently.
    CNameHash name;       // 10
    std::uint32_t unk14;  // 14
    std::uint64_t unk18;  // 18
    std::uint32_t offset; // 20 - byte offset of the value from its base
    std::uint32_t flags;  // 24
};
RED3LIB_ASSERT_OFFSET(CProperty, type, 0x8);
RED3LIB_ASSERT_OFFSET(CProperty, offset, 0x20);
RED3LIB_ASSERT_OFFSET(CProperty, flags, 0x24);
} // namespace red3lib

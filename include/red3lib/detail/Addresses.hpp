#pragma once

#include <cstdint>

// Addresses for The Witcher 3: Wild Hunt.
//
// Everything marked "name-anchored" below was extracted from
// bin/x64_dx12/witcher3.exe, file version 4.0.0.103190, by
// tools/w3offsets/w3offsets.py. Those are located via MSVC RTTI class names and
// UTF-16 script-function names rather than hardcoded offsets, so they can be
// regenerated after a game patch:
//
//     python tools/w3offsets/w3offsets.py --format hpp -o Addresses.hpp
//
namespace red3lib::detail::addresses
{
constexpr std::uintptr_t image_base = 0x140000000;

namespace CFunction
{
// Name-anchored. Global table of native implementations, indexed by
// CFunction::registration_offset (the field at +0xA8).
//
// RegisterNative stores the implementation here and records only the index in
// the CFunction, so a native can be invoked without execute_native:
//
//     table[fn->registration_offset](context, &frame, &result)
//
// The table is filled in at runtime; it reads as zeroes in the on-disk image.
constexpr std::uintptr_t native_table = 0x1457ed890 - image_base;

// Name-anchored. Class methods live in a SEPARATE table from global natives,
// with 24-byte pointer-to-member entries rather than 8-byte plain pointers.
// Both registrars share one counter, so each table is sparse and holds only
// entries of its own kind; `flags & 2` says which kind a CFunction is.
//
// Verified against the running game: 2245 populated entries, every one a .text
// pointer, matching the RTTI dump's native function count exactly. Two entries
// were checked against implementations read statically from their registration
// sites and matched.
constexpr std::uintptr_t class_method_table = 0x1457d5890 - image_base;

// Name-anchored. Kept for reference - CFunction::call_native does not need it.
constexpr std::uintptr_t register_native = 0x141496fa0 - image_base;

// WARNING - STALE. These target a different 4.00 build and have NOT been
// re-derived for 4.0.0.103190. They are not reachable from a name anchor (the
// functions are not virtual: CFunction's vtable holds only its destructor), so
// w3offsets.py cannot regenerate them.
//
// CFunction::execute() calls these and will jump into the wrong code on any
// build but the original. Use CFunction::call_native() instead until they are
// resolved for your build.
constexpr std::uintptr_t execute_native = 0x1402FD190 - image_base;
constexpr std::uintptr_t execute_scripted = 0x1402FD880 - image_base;
} // namespace CFunction

namespace CNamePool
{
// Name-anchored, 12/12 anchor consensus.
constexpr std::uintptr_t get = 0x1402843a0 - image_base;
constexpr std::uintptr_t add_wide = 0x14145a3a0 - image_base;

// WARNING - STALE, as above. CNameHash::to_wide() depends on this, and so does
// CClass::find_function(), which compares names as wide strings. Comparing
// CNameHash indices directly would avoid the dependency entirely.
constexpr std::uintptr_t find_wide = 0x1402E1540 - image_base;
} // namespace CNamePool
} // namespace red3lib::detail::addresses

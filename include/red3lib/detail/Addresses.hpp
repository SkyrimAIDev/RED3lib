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

// WARNING - STALE. Targets a different 4.00 build and has NOT been re-derived.
// call_native() replaces it and nothing uses it.
constexpr std::uintptr_t execute_native = 0x1402FD190 - image_base;

// The scripted-function interpreter. Re-derived for this build and reachable
// without a name: it is the only function in the image containing
// `add rax, qword ptr [rcx + 0x80]` - CFunction's bytecode pointer - so
// w3offsets anchors on that single occurrence and walks back to the prologue.
//
// Called as (CFunction*, IScriptable* context, void* params, void* result). It
// runs the function's own bytecode and allocates its own locals from
// stack_size, so unlike a native call there is no code stream to synthesise.
constexpr std::uintptr_t execute_scripted = 0x141496020 - image_base;
} // namespace CFunction

namespace CNamePool
{
// Name-anchored, 12/12 anchor consensus.
constexpr std::uintptr_t get = 0x1402843a0 - image_base;
constexpr std::uintptr_t add_wide = 0x14145a3a0 - image_base;

// Offset WITHIN the pool object, not an image address, so it needs no
// relocation. Name-anchored: the tail of CNamePool::Add pushes the new entry
// into `[pool + entry_array]` and then reads the count from eight bytes later
// to derive the index, so w3offsets reads both displacements straight out of
// that function.
//
// This replaces a stale `find_wide` call address. Reversing an index to text is
// an array index, not a call - which is also bounds-checkable, where the call
// was not.
constexpr std::uintptr_t entry_array = 0x11830;
} // namespace CNamePool
} // namespace red3lib::detail::addresses

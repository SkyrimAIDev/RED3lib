#pragma once

#include <cstdint>
#include <cstring>

namespace red3lib
{
struct CProperty;

// Writes call arguments into a CStackFrame's code stream.
//
// An argument is a "load parameter" instruction: opcode 0x10 followed by an
// 8-byte CProperty pointer. The handler reads the property's offset and type,
// forms the address `frame->params + offset`, and lets the type copy the value
// into the native's destination. Because the type does the copying, this works
// for any type - including String, which is too wide for the VM's immediate
// literal opcodes.
//
// Sibling opcodes read from other bases: 0x0F is frame->locals and 0x11 is the
// context object. 0x16, which this library previously emitted, is a no-op -
// which is why arguments never arrived.
//
// An argument is NINE bytes: the opcode, then the pointer. The fetch loop reads
// the opcode (advancing one) and calls the handler (which advances eight, past
// the pointer) - and that is all it does per argument.
//
// The single extra advance belongs to the END of the parameter list, not to each
// argument. Both a one-argument native and a two-argument one do exactly one
// `inc qword ptr [frame + 0x30]`, after their last fetch:
//
//   IsPausedForReason  fetch, inc                 -> 1 + 8 + 1  = 10 bytes
//   ShowOneliner       fetch, fetch, inc          -> 9 + 9 + 1  = 19 bytes
//
// This previously wrote ten bytes PER argument, putting a filler byte after each
// one. At one argument that is indistinguishable from correct - the filler is
// consumed by the end-of-params advance - which is why every single-argument
// call worked and why the error survived so long. At two, the loop reads its
// second opcode from the filler, gets 0x00, and 0x00 is a no-op: the second
// argument is silently dropped and its destination keeps whatever it held.
//
// That is one bug behind several symptoms. ShowOneliner got a null entity and so
// rendered nothing while returning cleanly; GetNPCsByTag never had its out array
// bound and crashed; IsSpecificRumbleActive(0, 0) returned false either way and
// so never showed it.
class [[nodiscard]] CStackFrameCodeWriter
{
public:
    static constexpr std::size_t bytes_per_argument = 1 + sizeof(void*);

    // The end-of-params marker the fetch loop steps over after the last
    // argument. Written once, by end_params().
    static constexpr std::size_t bytes_after_arguments = 1;

    CStackFrameCodeWriter(std::uint8_t* ptr) noexcept;
    ~CStackFrameCodeWriter() noexcept = default;

    void write(CProperty* property);
    void end();

private:
    std::uint8_t* m_cursor;
};
} // namespace red3lib

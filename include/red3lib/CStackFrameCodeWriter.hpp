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
// The caller's fetch loop reads the opcode (advancing one), calls the handler
// (which advances eight, past the pointer), then advances one more. So each
// argument occupies ten bytes.
class [[nodiscard]] CStackFrameCodeWriter
{
public:
    static constexpr std::size_t bytes_per_argument = 1 + sizeof(void*) + 1;

    CStackFrameCodeWriter(std::uint8_t* ptr) noexcept;
    ~CStackFrameCodeWriter() noexcept = default;

    void write(CProperty* property);

private:
    std::uint8_t* m_cursor;
};
} // namespace red3lib

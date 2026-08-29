#pragma once

#include <cstdint>
#include <cstring>
#include <type_traits>

namespace red3lib
{
// Writes call arguments into a CStackFrame's code stream.
//
// Arguments are NOT passed through the params buffer with CProperty pointers -
// that was an earlier misreading. The VM encodes them as typed immediate
// literals inline in the code stream, and a native fetches each one by reading
// an opcode byte and dispatching to a handler that copies the following bytes
// into the destination.
//
// The opcode handlers were read out of the running game's dispatch table:
//
//     0x05   copy 2 bytes    0x08   copy 4 bytes    0x09   copy 1 byte
//     0x0A   literal true    0x0B   literal false
//     0x00   no-op           0x16   no-op
//
// The caller's fetch loop reads the opcode (advancing one), calls the handler
// (which advances by the immediate's width), then advances one more - so each
// argument occupies 1 + sizeof(T) + 1 bytes.
class [[nodiscard]] CStackFrameCodeWriter
{
public:
    CStackFrameCodeWriter(std::uint8_t* ptr) noexcept;
    ~CStackFrameCodeWriter() noexcept = default;

    // Bytes emitted for an argument of type T.
    template<typename T>
    static constexpr std::size_t encoded_size() noexcept
    {
        return 1 + sizeof(T) + 1;
    }

    template<typename T>
    void write(T value);

private:
    std::uint8_t* m_cursor;
};

template<typename T>
inline void CStackFrameCodeWriter::write(T value)
{
    static_assert(std::is_trivially_copyable_v<T>, "arguments must be trivially copyable");
    static_assert(sizeof(T) == 1 || sizeof(T) == 2 || sizeof(T) == 4,
                  "no immediate opcode is known for this argument size - only 1, 2 and 4 byte "
                  "types can be passed yet. Wider types (String, handles) use a different "
                  "encoding that has not been reverse engineered.");

    constexpr std::uint8_t opcode = sizeof(T) == 1 ? 0x09 : (sizeof(T) == 2 ? 0x05 : 0x08);

    *m_cursor = opcode;
    m_cursor++;

    std::memcpy(m_cursor, &value, sizeof(T));
    m_cursor += sizeof(T);

    // The fetch loop advances once more after the handler returns; the value of
    // this byte is never read.
    *m_cursor = 0;
    m_cursor++;
}
} // namespace red3lib

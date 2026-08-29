#include <red3lib/CStackFrameCodeWriter.hpp>

#include <red3lib/CProperty.hpp>

red3lib::CStackFrameCodeWriter::CStackFrameCodeWriter(std::uint8_t* ptr) noexcept
    : m_cursor(ptr)
{
}

void red3lib::CStackFrameCodeWriter::write(CProperty* property)
{
    constexpr std::uint8_t load_parameter = 0x10;

    *m_cursor = load_parameter;
    m_cursor++;

    std::memcpy(m_cursor, &property, sizeof(property));
    m_cursor += sizeof(property);

    // The fetch loop advances once more after the handler returns; this byte is
    // never read.
    *m_cursor = 0;
    m_cursor++;
}

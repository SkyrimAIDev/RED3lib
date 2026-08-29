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
}

void red3lib::CStackFrameCodeWriter::end()
{
    // One byte, after the last argument - the fetch loop steps over it exactly
    // once regardless of how many arguments there were.
    *m_cursor = 0;
    m_cursor++;
}

#include <red3lib/CStackFrameCodeWriter.hpp>

red3lib::CStackFrameCodeWriter::CStackFrameCodeWriter(std::uint8_t* ptr) noexcept
    : m_cursor(ptr)
{
}

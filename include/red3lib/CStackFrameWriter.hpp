#pragma once

#include <cstring>
#include <type_traits>

#include <red3lib/CProperty.hpp>
#include <red3lib/CStackFrame.hpp>
#include <red3lib/CStackFrameCodeWriter.hpp>
#include <red3lib/CStackFrameParamWriter.hpp>
#include <red3lib/detail/Common.hpp>

namespace red3lib
{
// Places call arguments where the VM expects them: the value goes into the
// frame's params buffer at the offset its CProperty declares, and a matching
// load instruction goes into the code stream.
class [[nodiscard]] CStackFrameWriter
{
public:
    CStackFrameWriter(const CStackFrame& stack_frame) noexcept;
    ~CStackFrameWriter() noexcept = default;

    RED3LIB_NON_COPYABLE_AND_NON_MOVABLE(CStackFrameWriter);

    template<typename T>
    void write_argument(CProperty* property, T value);

    void end_params();

private:
    std::uint8_t* m_params;
    CStackFrameCodeWriter m_code;
};

template<typename T>
inline void CStackFrameWriter::write_argument(CProperty* property, T value)
{
    static_assert(std::is_trivially_copyable_v<T>, "arguments must be trivially copyable");

    // Arguments are not packed sequentially - each one goes where its property
    // says. For two floats those offsets happen to be 0 and 4, but the property
    // is authoritative and wider types are not tightly packed.
    std::memcpy(m_params + property->offset, &value, sizeof(T));
    m_code.write(property);
}
} // namespace red3lib

#pragma once

#include <array>
#include <cstdint>
#include <cstring>
#include <malloc.h>
#include <memory>
#include <type_traits>

#include <red3lib/CNameHash.hpp>
#include <red3lib/CStackFrame.hpp>
#include <red3lib/CStackFrameWriter.hpp>
#include <red3lib/Handle.hpp>
#include <red3lib/IRTTIBaseObject.hpp>
#include <red3lib/containers/TDynArray.hpp>
#include <red3lib/detail/Addresses.hpp>
#include <red3lib/detail/Asserts.hpp>
#include <red3lib/detail/Relocation.hpp>

namespace red3lib
{
struct CClass;
struct CProperty;
struct IScriptable;

struct CFunction : IRTTIBaseObject
{
    struct unk
    {
        std::int64_t unk0;
        std::int32_t unk8;
        std::int32_t unkC;
        std::int32_t unk10;
        std::int32_t unk14;
        std::int64_t unk18;
        std::int64_t unk20;
        std::int64_t unk28;
        std::int32_t size;
        std::int64_t unk38;
        std::int32_t unk40;
        std::int32_t unk44;
        std::int32_t unk48;
        std::int32_t unk4C;
    };

    // Signature every native implementation is called with. Confirmed across
    // StrLen, StrUpper, StrCmp, IntToString and StringToInt: rdx is the stack
    // frame (each reads frame+0x30 for the code pointer and frame+0x00 for the
    // context) and r8 is where the return value is written.
    using native_fn_t = void (*)(IScriptable* context, CStackFrame* frame, void* result);

    // A class-method entry: MSVC's most general pointer-to-member form. Surveyed
    // across all 2245 live entries - `code` is always a .text pointer,
    // `this_adjust` is 0 except for fifteen methods reached through a secondary
    // base where it is 16, and the trailing field is uninitialised stack the
    // registrar never fills in.
    struct native_method_entry
    {
        native_fn_t code;           // 00
        std::int32_t this_adjust;   // 08 - byte offset applied to the context
        std::int32_t unused0;       // 0C
        std::int32_t unused1;       // 10
        std::int32_t uninitialised; // 14 - never read
    };
    static_assert(sizeof(native_method_entry) == 24, "class-method table has a 24-byte stride");

    // Globals are registered into a different table with plain 8-byte entries.
    [[nodiscard]] bool is_global() const noexcept
    {
        constexpr auto is_global_flag = 1 << 1;
        return (flags & is_global_flag) == is_global_flag;
    }

    template<typename R = void, typename... Args>
    R execute(IScriptable* context, Args&&... args);

    // Invoke a native directly through the engine's dispatch table, bypassing
    // CFunction::execute_native.
    //
    // RegisterNative does not store the implementation in the CFunction - it
    // appends it to a global table and records only the index here, in
    // registration_offset. That table is reachable by name (see
    // tools/w3offsets), whereas execute_native is not, so this path survives
    // game patches and execute() currently does not.
    //
    // Only valid for native functions; check the native flag first.
    template<typename R = void, typename... Args>
    R call_native(IScriptable* context, Args&&... args);

    // The RTTI types of what this function hands back, so a caller can release
    // engine allocations through IRTTIType::destruct. Null when there is
    // nothing to release.
    [[nodiscard]] IRTTIType* return_type() const noexcept
    {
        return return_property ? return_property->type : nullptr;
    }

    [[nodiscard]] IRTTIType* param_type(std::uint32_t index) const noexcept
    {
        if (index >= params.size || !params.entries[index])
        {
            return nullptr;
        }

        return params.entries[index]->type;
    }

    [[nodiscard]] bool is_native() const noexcept
    {
        constexpr auto is_native_flag = 1 << 0;
        return (flags & is_native_flag) == is_native_flag;
    }

    CClass* owner;           // 08
    red3lib::CNameHash name; // 10
    std::int32_t flags;      // 14
    // Describes the return value, exactly like the entries in `params`;
    // null when the function returns nothing. Its `type` names the RTTI
    // type - CRTTIHandleType for anything returning an object.
    CProperty* return_property;       // 18
    TDynArray<CProperty*> params;     // 20
    TDynArray<CProperty*> locals;     // 2C
    std::int64_t unk38;               // 38
    std::int32_t unk40;               // 40
    std::int32_t unk44;               // 44
    std::int32_t unk48;               // 48
    std::int32_t unk4C;               // 4C
    std::int32_t stack_size;          // 50
    unk unk58;                        // 58
    std::int32_t registration_offset; // A8
    std::int64_t unkB0;               // B0
    std::int64_t unkB8;               // B8

private:
    template<typename R, typename... Args>
    R execute_native(IScriptable* context, Args&&... args);

    template<typename R, typename... Args>
    R execute_scripted(IScriptable* context, Args&&... args);

    std::size_t calculate_locals_stack_size() const;
};
RED3LIB_ASSERT_SIZE(CFunction, 0xC0);
RED3LIB_ASSERT_OFFSET(CFunction, owner, 0x8);
RED3LIB_ASSERT_OFFSET(CFunction, name, 0x10);
RED3LIB_ASSERT_OFFSET(CFunction, flags, 0x14);
RED3LIB_ASSERT_OFFSET(CFunction, return_property, 0x18);
RED3LIB_ASSERT_OFFSET(CFunction, params, 0x20);
RED3LIB_ASSERT_OFFSET(CFunction, locals, 0x2C);
RED3LIB_ASSERT_OFFSET(CFunction, registration_offset, 0xA8);

template<typename R, typename... Args>
inline R CFunction::execute(IScriptable* context, Args&&... args)
{
    // Maybe this one should be removed and add something similar to https://doc.rust-lang.org/std/result/.
    RED3LIB_ASSERT(params.size == sizeof...(args));

    constexpr auto is_native_flag = 1 << 0;
    if ((flags & is_native_flag) == is_native_flag)
    {
        return execute_native<R>(context, std::forward<Args>(args)...);
    }
    else
    {
        return execute_scripted<R>(context, std::forward<Args>(args)...);
    }
}

template<typename R, typename... Args>
inline R CFunction::execute_native(IScriptable* context, Args&&... args)
{
    constexpr auto args_count = sizeof...(Args);
    constexpr auto args_total_size = (0 + ... + sizeof(Args));
    auto locals_stack_size = calculate_locals_stack_size();
    auto stack_ptr = reinterpret_cast<std::uint8_t*>(_malloca(locals_stack_size));
    RED3LIB_ASSERT(stack_ptr);
    std::memset(stack_ptr, 0, locals_stack_size);

    std::unique_ptr<std::uint8_t, decltype(&_freea)> locals_stack(stack_ptr, &_freea);
    std::array<std::uint8_t, args_total_size + 1> params_stack{};
    std::array<std::uint8_t, 9 * args_count> code_stack{};

    CStackFrame frame(this, context, locals_stack.get(), params_stack.data(), code_stack.data());
    CStackFrameWriter writer(frame);

    std::size_t index = 0;
    (writer.write_value(params.entries[index++], std::forward<Args>(args)), ...);
    writer.end_params();

    detail::RelocFunc<bool, CFunction*, IScriptable*, CStackFrame&, R*> func(
        detail::addresses::CFunction::execute_native);

    if constexpr (std::is_same_v<R, void>)
    {
        func(this, context, frame, nullptr);
    }
    else
    {
        R result{};
        func(this, context, frame, &result);

        return result;
    }
}

template<typename R, typename... Args>
inline R CFunction::execute_scripted(IScriptable* context, Args&&... args)
{
    constexpr auto args_total_size = (0 + ... + sizeof(Args));
    std::array<std::uint8_t, args_total_size + 1> params_stack{};

    CStackFrameParamWriter writer(params_stack.data());
    (writer.write(std::forward<Args>(args)), ...);
    writer.write_end();

    detail::RelocFunc<bool, CFunction*, IScriptable*, std::uint8_t*, R*> func(
        detail::addresses::CFunction::execute_scripted);

    if constexpr (std::is_same_v<R, void>)
    {
        func(this, context, params_stack.data(), nullptr);
    }
    else
    {
        R result{};
        func(this, context, params_stack.data(), &result);

        return result;
    }
}

template<typename R, typename... Args>
inline R CFunction::call_native(IScriptable* context, Args&&... args)
{
    RED3LIB_ASSERT(is_native());
    RED3LIB_ASSERT(params.size == sizeof...(args));
    RED3LIB_ASSERT(registration_offset >= 0);

    auto locals_stack_size = calculate_locals_stack_size();
    auto stack_ptr = reinterpret_cast<std::uint8_t*>(_malloca(locals_stack_size));
    RED3LIB_ASSERT(stack_ptr);
    if (!stack_ptr)
    {
        if constexpr (std::is_same_v<R, void>)
        {
            return;
        }
        else
        {
            return R{};
        }
    }

    std::memset(stack_ptr, 0, locals_stack_size);
    std::unique_ptr<std::uint8_t, decltype(&_freea)> locals_stack(stack_ptr, &_freea);

    // The params buffer is laid out by the function's own properties, so it is
    // sized from stack_size rather than from the C++ argument sizes.
    auto params_size = static_cast<std::size_t>(stack_size) + 1;
    auto params_ptr = reinterpret_cast<std::uint8_t*>(_malloca(params_size));
    RED3LIB_ASSERT(params_ptr);
    if (!params_ptr)
    {
        if constexpr (std::is_same_v<R, void>)
        {
            return;
        }
        else
        {
            return R{};
        }
    }

    std::memset(params_ptr, 0, params_size);
    std::unique_ptr<std::uint8_t, decltype(&_freea)> params_stack(params_ptr, &_freea);

    // Nine bytes per argument, then one end-of-params byte - which also keeps
    // the buffer non-empty when there are no arguments at all.
    constexpr auto code_size = CStackFrameCodeWriter::bytes_per_argument * sizeof...(Args) +
                               CStackFrameCodeWriter::bytes_after_arguments;
    std::array<std::uint8_t, code_size> code_stack{};

    CStackFrame frame(this, context, locals_stack.get(), params_stack.get(), code_stack.data());
    CStackFrameWriter writer(frame);

    std::size_t index = 0;
    (writer.write_argument(params.entries[index++], std::forward<Args>(args)), ...);
    writer.end_params();

    // registration_offset is one global counter shared by both registrars, so it
    // indexes whichever table this function was registered into; the other is
    // simply empty at that index.
    auto* self = context;
    native_fn_t impl = nullptr;

    if (is_global())
    {
        detail::RelocArray<native_fn_t> table(detail::addresses::CFunction::native_table);
        impl = table[static_cast<std::size_t>(registration_offset)];
    }
    else
    {
        detail::RelocArray<native_method_entry> table(detail::addresses::CFunction::class_method_table);
        const auto& entry = table[static_cast<std::size_t>(registration_offset)];
        impl = entry.code;
        self = reinterpret_cast<IScriptable*>(reinterpret_cast<std::uint8_t*>(context) + entry.this_adjust);
    }

    RED3LIB_ASSERT(impl);

    // Out parameters. A native writes its results back into the params buffer,
    // at the same offset its CProperty declares, so an argument passed as a
    // non-const lvalue reference is refreshed from that buffer after the call.
    // Passing by value opts out, which leaves every existing call unchanged.
    //
    // Whatever the engine allocated into an out parameter belongs to the
    // engine - an out array's storage, and a reference on every handle it
    // holds. Nothing here releases either.
    auto read_back = [&](CProperty* property, auto&& value)
    {
        using argument = decltype(value);
        using stored = std::remove_reference_t<argument>;

        if constexpr (std::is_lvalue_reference_v<argument> && !std::is_const_v<stored>)
        {
            std::memcpy(&value, params_stack.get() + property->offset, sizeof(stored));
        }
    };

    if constexpr (std::is_same_v<R, void>)
    {
        if (impl)
        {
            impl(self, &frame, nullptr);
        }

        if constexpr (sizeof...(Args) > 0)
        {
            std::size_t out_index = 0;
            (read_back(params.entries[out_index++], std::forward<Args>(args)), ...);
        }
    }
    else
    {
        R result{};
        if (impl)
        {
            impl(self, &frame, &result);
        }

        if constexpr (sizeof...(Args) > 0)
        {
            std::size_t out_index = 0;
            (read_back(params.entries[out_index++], std::forward<Args>(args)), ...);
        }

        return result;
    }
}
} // namespace red3lib

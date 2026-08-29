#include <red3lib/CNamePool.hpp>

#include <red3lib/detail/Addresses.hpp>
#include <red3lib/containers/TDynArray.hpp>
#include <red3lib/detail/Relocation.hpp>

std::uint32_t red3lib::name_pool::add(const wchar_t* text)
{
    detail::RelocFunc<std::uint32_t, name_pool*, const wchar_t*> func(detail::addresses::CNamePool::add_wide);

    auto ptr = instance();
    return func(ptr, text);
}

namespace
{
// One interned name. CNamePool::Add copies the wide and ANSI text into the same
// allocation right behind this header, so both pointers point into the entry
// itself.
struct name_entry
{
    const wchar_t* wide; // 00
    const char* ansi;    // 08
    name_entry* next;    // 10 - next in the hash bucket
    std::uint32_t hash;  // 18
    std::uint32_t index; // 1C - its position in the pool's entry array
};
} // namespace

std::wstring_view red3lib::name_pool::to_wide_string(const CNameHash& hash)
{
    // A CNameHash is an index into the pool's entry array, so reversing it is a
    // lookup rather than a call. Every step is checked: this runs on the game's
    // main thread, and a bad index here would fault it.
    auto* pool = reinterpret_cast<std::uint8_t*>(instance());
    if (!pool)
    {
        return {};
    }

    const auto& entries =
        *reinterpret_cast<const TDynArray<name_entry*>*>(pool + detail::addresses::CNamePool::entry_array);

    if (!entries.entries || hash.index() >= entries.size)
    {
        return {};
    }

    const auto* entry = entries.entries[hash.index()];
    if (!entry || !entry->wide)
    {
        return {};
    }

    return entry->wide;
}

red3lib::name_pool* red3lib::name_pool::instance()
{
    detail::RelocFunc<red3lib::name_pool*> func(detail::addresses::CNamePool::get);
    return func();
}

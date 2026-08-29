#include <red3lib/CClass.hpp>
#include <red3lib/CFunction.hpp>

red3lib::CFunction* red3lib::CClass::find_function(red3lib::CNameHash func_name) const
{
    // Compare pool indices rather than strings. CNamePool::Add interns, so two
    // equal names always share an index - this is exact, avoids materialising a
    // wide string per entry, and keeps lookup off CNamePool::find_wide, which is
    // not name-anchored and therefore goes stale on every game patch.
    for (std::uint32_t i = 0; i < functions.size; i++)
    {
        auto function = functions.entries[i];
        if (function->name == func_name)
        {
            return function;
        }
    }

    if (base)
    {
        return base->find_function(func_name);
    }

    return nullptr;
}

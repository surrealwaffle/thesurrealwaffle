
//          Copyright surrealwaffle 2018 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <sigscan/memory_range.hpp>

#include <optional> // std::optional

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <psapi.h>
#include <dbghelp.h>

namespace {

/** \brief Modifies the protections on the pages in \a range to \a flNewProtect,
 *         return the old protections on success and `nullopt` on error.
 */
std::optional<DWORD> modify_protections(sigscan::memory_range range,
                                        DWORD flNewProtect);

/** \brief Retrieves a pointer to the first byte the supplied module's image,
 *         or `nullptr` if it could not be retrieved.
 */
sigscan::byte* get_image_base(HMODULE hModule);

} // namespace (anonymous)

namespace sigscan {

std::optional<scope_guard<std::function<void()>>> hold_range_rwx(memory_range range)
{
    auto oldProtect = modify_protections(range, PAGE_EXECUTE_READWRITE);
    if (!oldProtect)
        return std::nullopt;

    auto restore_protections = [range, oldProtect = std::move(oldProtect)]
        { if (oldProtect) modify_protections(range, *oldProtect); };
    return std::function(restore_protections);
}

std::optional<std::vector<memory_range>> get_text_segments(char const* module_name)
{
    HMODULE hModule = NULL;

    if ((hModule = GetModuleHandleA(module_name)) == NULL)
        return std::nullopt;

    byte*             image_base   = get_image_base(hModule);
    IMAGE_NT_HEADERS* image_header = ImageNtHeader(hModule);

    if (image_base == nullptr || image_header == nullptr)
        return std::nullopt;

    IMAGE_SECTION_HEADER* image_section_headers =
        reinterpret_cast<IMAGE_SECTION_HEADER*>(image_header + 1);

    std::vector<memory_range> text_segments;
    auto push_if_text_segment = [&text_segments, image_base]
                                (const auto& section_header) {
        if (section_header.Characteristics & IMAGE_SCN_CNT_CODE) {
            byte* segment_base = image_base + section_header.VirtualAddress;
            byte* segment_end  = segment_base + section_header.Misc.VirtualSize;
            text_segments.push_back({segment_base, segment_end});
        }
    };

    std::for_each(image_section_headers,
                  image_section_headers + image_header->FileHeader.NumberOfSections,
                  push_if_text_segment);

    return text_segments;
}

void flush_range(memory_range range)
{
    FlushInstructionCache(GetCurrentProcess(),
                          range.first,
                          (SIZE_T)(range.last - range.first));
}

} // namespace SIGSCAN

namespace {

std::optional<DWORD> modify_protections(sigscan::memory_range range,
                                        DWORD flNewProtect)
{
    DWORD flOldProtect = 0;
    BOOL success = VirtualProtect(range.first,
                                  (SIZE_T)(range.last - range.first),
                                  flNewProtect, &flOldProtect);
    if (!success)
        return std::nullopt;

    return flOldProtect;
}


sigscan::byte* get_image_base(HMODULE hModule)
{
    if (hModule == NULL)
        return nullptr;

    MODULEINFO modinfo = {/* ZERO INITIALIZED */};
    if (GetModuleInformation(GetCurrentProcess(), hModule,
                             &modinfo, sizeof(modinfo)) == 0)
        return nullptr;

    return reinterpret_cast<sigscan::byte*>(modinfo.lpBaseOfDll);
}

} // namespace (anonymous)

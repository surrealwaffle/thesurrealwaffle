
//          Copyright surrealwaffle 2018 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "loader.hpp"

#include <cstdio>
#include <cstdlib>
#include <tchar.h>

#include <algorithm>
#include <vector>
#include <memory>

#include <sentinel/config.hpp>

namespace {

struct module_entry {
    std::basic_string<TCHAR> module_name;
    HMODULE                  module_handle;

    module_entry(LPCTSTR name, HMODULE hModule)
        : module_name(name)
        , module_handle(hModule) { }

    module_entry(module_entry&&)            = default;
    module_entry& operator=(module_entry&&) = default;

    module_entry(module_entry const&)            = delete;
    module_entry& operator=(module_entry const&) = delete;
};

std::basic_string<TCHAR> modules_directory;
std::vector<module_entry> modules;

void init_modules_directory();

}

namespace sentinel { namespace impl_loader {

bool LoadClientLibrary(LPCTSTR lpModuleName) {
    if (!lpModuleName)
        return false;

    init_modules_directory();
    _tprintf(_T("loading library \"%s\"\n"), lpModuleName);

    for (const auto& module : modules) {
        if (module.module_name == lpModuleName) {
            _tprintf(_T("library \"%s\" already loaded\n"), lpModuleName);
            return true;
        }
    }

    std::basic_string<TCHAR> path = modules_directory + "\\" + lpModuleName;
    HMODULE hModule = LoadLibrary(path.c_str());
    if (hModule == NULL) {
        _tprintf(_T("failed to load library \"%s\"\n"), lpModuleName);
        return false;
    }

    modules.emplace_back(lpModuleName, hModule);

    return true;
}

bool UnloadClientLibrary(LPCTSTR lpModuleName) {
    auto it = std::find_if(modules.cbegin(), modules.cend(),
                           [lpModuleName] (const auto& e)
                           { return e.module_name == lpModuleName; });

    if (it == modules.cend())
        return false;

    using client_unload_tproc = void(*)();
    auto unload = (client_unload_tproc)GetProcAddress(it->module_handle,
                                                      SENTINEL_CLIENT_UNLOAD_PROC);
    if (unload)
        unload();

    FreeLibrary(it->module_handle);
    modules.erase(it);
    return true;
}

void LoadClientLibraries()
{
    WIN32_FIND_DATA ffd;
    HANDLE          hFind;

    init_modules_directory();
    hFind = FindFirstFile((modules_directory + _T("\\*.dll")).c_str(), &ffd);

    if (hFind == INVALID_HANDLE_VALUE)
        return;

    do {
        LoadClientLibrary(ffd.cFileName);
    } while (FindNextFile(hFind, &ffd) != 0);

    FindClose(hFind);
}

void UnloadClientLibraries()
{
    while (!modules.empty())
        UnloadClientLibrary(modules.back().module_name.data());
}

void PerformSecondaryClientLoads() {
    init_modules_directory();
    for (auto&& module : modules) {
        using client_load_tproc = void (*)();
        auto load = (client_load_tproc)GetProcAddress(module.module_handle,
                                                      SENTINEL_CLIENT_LOAD_PROC);
        if (load)
            load();
    }
}

} } // namespace sentinel::impl_loader

namespace {

void init_modules_directory()
{
    if (!modules_directory.empty())
        return;

    if (const TCHAR* sentinel_load_dir = _tgetenv(_T(SENTINEL_ENV_MODULES_DIRECTORY)))
        modules_directory = sentinel_load_dir;
    else
        modules_directory = _T(SENTINEL_APPLICATION_DIR);
}

} // namespace (anonymous)

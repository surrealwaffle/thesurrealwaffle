#include "loader.hpp"

#include <algorithm>

#include <cstdio>
#include <tchar.h>

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

std::vector<module_entry> modules;

}

namespace sentinel { namespace impl_loader {

bool LoadClientLibrary(LPCTSTR lpModuleName) {
    if (!lpModuleName)
        return false;

    _tprintf(_T("loading library \"%s\"\n"), lpModuleName);

    for (const auto& module : modules) {
        if (module.module_name == lpModuleName) {
            _tprintf(_T("library \"%s\" already loaded\n"), lpModuleName);
            return true;
        }
    }

    TCHAR path[MAX_PATH + 1u];
    path[MAX_PATH] = (TCHAR)'\0';
    ::_sntprintf(path, MAX_PATH,
                 _T(SENTINEL_APPLICATION_DIR) _T("/%s"), lpModuleName);


    HMODULE hModule = LoadLibrary(path);
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

    hFind = FindFirstFile(SENTINEL_APPLICATION_DIR "/*.dll", &ffd);
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
    for (auto&& module : modules) {
        using client_load_tproc = void (*)();
        auto load = (client_load_tproc)GetProcAddress(module.module_handle,
                                                      SENTINEL_CLIENT_LOAD_PROC);
        if (load)
            load();
    }
}

} } // namespace sentinel::impl_loader

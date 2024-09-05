#include "pch.h"
#include <cstdint>
#include <iostream>

#include "dll.h"

const char* ModuleName[2]{ "mono.dll", "mono-2.0-bdwgc.dll" };
HMODULE hModule;

const char* nameSpace = "Rake";
const char* klassName = "Loader";
const char* MethodName = "Load";

typedef void* (*t_mono_get_root_domain)();
typedef void* (*t_mono_image_open_from_data)(char* data, size_t data_len, int32_t need_copy, void* status);
typedef void* (*t_mono_assembly_load_from_full)(void* image, const char* filename, void* status, int32_t refonly);
typedef void* (*t_mono_assembly_get_image)(void* assembly);
typedef void* (*t_mono_class_from_name)(void* image, const char* name_space, const char* name);
typedef void* (*t_mono_class_get_method_from_name)(void* klass, const char* name, int param_count);
typedef void* (*t_mono_runtime_invoke)(void* method, void* obj, void** params, void** exc);
typedef void* (*t_mono_thread_attach)(void* domain);
typedef void* (*t_mono_thread_detach)(void* thread);

static inline t_mono_get_root_domain mono_get_root_domain;
static inline t_mono_image_open_from_data mono_image_open_from_data;
static inline t_mono_assembly_load_from_full mono_assembly_load_from_full;
static inline t_mono_assembly_get_image mono_assembly_get_image;
static inline t_mono_class_from_name mono_class_from_name;
static inline t_mono_class_get_method_from_name mono_class_get_method_from_name;
static inline t_mono_runtime_invoke mono_runtime_invoke;
static inline t_mono_thread_attach mono_thread_attach;
static inline t_mono_thread_detach mono_thread_detach;

bool call = 0;
void start() {
    if (call)
        return;

    void* domain = mono_get_root_domain();
    if (!domain)
        return;

    void* thread = mono_thread_attach(domain);

    uint32_t data_len = sizeof(data) / sizeof(data[0]);
    void* rawImage = mono_image_open_from_data((char*)data, data_len, 1, nullptr);
    if (!rawImage)
        return;

    void* assembly = mono_assembly_load_from_full(rawImage, new char[1], nullptr, 0);
    if (!assembly)
        return;

    void* assemblyImage = mono_assembly_get_image(assembly);
    if (!assemblyImage)
        return;

    void* klass = mono_class_from_name(assemblyImage, nameSpace, klassName);
    if (!klass)
        return;

    void* method = mono_class_get_method_from_name(klass, MethodName, 0);
    if (!method)
        return;

    mono_runtime_invoke(method, nullptr, nullptr, nullptr);

    mono_thread_detach(thread);
    call = 1;
}

int init = 0;
DWORD WINAPI MainThread(LPVOID lpReserved)
{
    AllocConsole();
    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);;

    if (GetModuleHandleA(ModuleName[0])) {
        hModule = GetModuleHandleA(ModuleName[0]);
        std::cout << "module : '" << ModuleName[0] << "' " << hModule << std::endl;
    }

    else if (GetModuleHandleA(ModuleName[1])) {
        hModule = GetModuleHandleA(ModuleName[1]);
        std::cout << "module : '" << ModuleName[1] << "' " << hModule << std::endl;
    }
            
    if (!hModule)
        return FALSE;

    if (init == 0) {
        mono_get_root_domain = reinterpret_cast<t_mono_get_root_domain>(GetProcAddress(hModule, "mono_get_root_domain"));
        mono_image_open_from_data = reinterpret_cast<t_mono_image_open_from_data>(GetProcAddress(hModule, "mono_image_open_from_data"));
        mono_assembly_load_from_full = reinterpret_cast<t_mono_assembly_load_from_full>(GetProcAddress(hModule, "mono_assembly_load_from_full"));
        mono_assembly_get_image = reinterpret_cast<t_mono_assembly_get_image>(GetProcAddress(hModule, "mono_assembly_get_image"));
        mono_class_from_name = reinterpret_cast<t_mono_class_from_name>(GetProcAddress(hModule, "mono_class_from_name"));
        mono_class_get_method_from_name = reinterpret_cast<t_mono_class_get_method_from_name>(GetProcAddress(hModule, "mono_class_get_method_from_name"));
        mono_runtime_invoke = reinterpret_cast<t_mono_runtime_invoke>(GetProcAddress(hModule, "mono_runtime_invoke"));
        mono_thread_attach = reinterpret_cast<t_mono_thread_attach>(GetProcAddress(hModule, "mono_thread_attach"));
        mono_thread_detach = reinterpret_cast<t_mono_thread_detach>(GetProcAddress(hModule, "mono_thread_detach"));
        init = 1;
    }

    std::cout << "\nmono_get_root_domain : " << mono_get_root_domain << std::endl;
    std::cout << "mono_image_open_from_data : " << mono_image_open_from_data << std::endl;
    std::cout << "mono_assembly_load_from_full : " << mono_assembly_load_from_full << std::endl;
    std::cout << "mono_assembly_get_image : " << mono_assembly_get_image << std::endl;
    std::cout << "mono_class_from_name : " << mono_class_from_name << std::endl;
    std::cout << "mono_class_get_method_from_name : " << mono_class_get_method_from_name << std::endl;
    std::cout << "mono_runtime_invoke : " << mono_runtime_invoke << std::endl;
    std::cout << "mono_thread_attach : " << mono_thread_attach << std::endl;
    std::cout << "mono_thread_detach : " << mono_thread_detach << std::endl;

    start();

	return TRUE;
}

BOOL WINAPI DllMain( HMODULE hModule, DWORD dwReason, LPVOID lpReserved )
{
    if (dwReason == 1)
        CreateThread(nullptr, 0, MainThread, hModule, 0, nullptr);

    return TRUE;
}


#pragma once
// Minimal IL2CPP API bindings for Unity IL2CPP games (GameAssembly.dll)
#include <Windows.h>
#include <cstdint>

typedef void* Il2CppDomain;
typedef void* Il2CppAssembly;
typedef void* Il2CppImage;
typedef void* Il2CppClass;
typedef void* Il2CppObject;
typedef void* Il2CppMethod;
typedef void* Il2CppField;
typedef void* Il2CppString;
typedef void* Il2CppType;
typedef void* Il2CppArray;

// Function pointer typedefs
typedef Il2CppDomain(*t_il2cpp_domain_get)();
typedef const Il2CppAssembly**(*t_il2cpp_domain_get_assemblies)(const Il2CppDomain domain, size_t* size);
typedef const Il2CppImage*(*t_il2cpp_assembly_get_image)(const Il2CppAssembly* assembly);
typedef Il2CppClass*(*t_il2cpp_class_from_name)(const Il2CppImage* image, const char* namespaze, const char* name);
typedef Il2CppField*(*t_il2cpp_class_get_field_from_name)(Il2CppClass* klass, const char* name);
typedef Il2CppMethod*(*t_il2cpp_class_get_method_from_name)(Il2CppClass* klass, const char* name, int argsCount);
typedef void(*t_il2cpp_field_static_get_value)(Il2CppField* field, void* value);
typedef Il2CppClass*(*t_il2cpp_class_get_parent)(Il2CppClass* klass);
typedef const char*(*t_il2cpp_class_get_name)(Il2CppClass* klass);
typedef const char*(*t_il2cpp_image_get_name)(const Il2CppImage* image);
typedef Il2CppObject*(*t_il2cpp_object_new)(Il2CppClass* klass);
typedef Il2CppArray*(*t_il2cpp_array_new)(Il2CppClass* elementTypeInfo, size_t length);
typedef Il2CppString*(*t_il2cpp_string_new)(const char* str);

struct IL2CPP_API {
    HMODULE ga = nullptr;
    t_il2cpp_domain_get                    domain_get = nullptr;
    t_il2cpp_domain_get_assemblies         domain_get_assemblies = nullptr;
    t_il2cpp_assembly_get_image            assembly_get_image = nullptr;
    t_il2cpp_class_from_name               class_from_name = nullptr;
    t_il2cpp_class_get_field_from_name     class_get_field_from_name = nullptr;
    t_il2cpp_class_get_method_from_name    class_get_method_from_name = nullptr;
    t_il2cpp_field_static_get_value        field_static_get_value = nullptr;
    t_il2cpp_class_get_parent              class_get_parent = nullptr;
    t_il2cpp_class_get_name                class_get_name = nullptr;
    t_il2cpp_image_get_name                image_get_name = nullptr;
    t_il2cpp_object_new                    object_new = nullptr;
    t_il2cpp_array_new                     array_new = nullptr;
    t_il2cpp_string_new                    string_new = nullptr;

    bool init() {
        ga = GetModuleHandleA("GameAssembly.dll");
        if (!ga) return false;
        domain_get                    = (t_il2cpp_domain_get)                GetProcAddress(ga, "il2cpp_domain_get");
        domain_get_assemblies         = (t_il2cpp_domain_get_assemblies)     GetProcAddress(ga, "il2cpp_domain_get_assemblies");
        assembly_get_image            = (t_il2cpp_assembly_get_image)        GetProcAddress(ga, "il2cpp_assembly_get_image");
        class_from_name               = (t_il2cpp_class_from_name)           GetProcAddress(ga, "il2cpp_class_from_name");
        class_get_field_from_name     = (t_il2cpp_class_get_field_from_name) GetProcAddress(ga, "il2cpp_class_get_field_from_name");
        class_get_method_from_name    = (t_il2cpp_class_get_method_from_name)GetProcAddress(ga, "il2cpp_class_get_method_from_name");
        field_static_get_value        = (t_il2cpp_field_static_get_value)    GetProcAddress(ga, "il2cpp_field_static_get_value");
        class_get_parent              = (t_il2cpp_class_get_parent)          GetProcAddress(ga, "il2cpp_class_get_parent");
        class_get_name                = (t_il2cpp_class_get_name)            GetProcAddress(ga, "il2cpp_class_get_name");
        image_get_name                = (t_il2cpp_image_get_name)            GetProcAddress(ga, "il2cpp_image_get_name");
        object_new                    = (t_il2cpp_object_new)                GetProcAddress(ga, "il2cpp_object_new");
        array_new                     = (t_il2cpp_array_new)                 GetProcAddress(ga, "il2cpp_array_new");
        string_new                    = (t_il2cpp_string_new)                GetProcAddress(ga, "il2cpp_string_new");
        return domain_get && domain_get_assemblies && assembly_get_image && class_from_name
            && class_get_field_from_name && field_static_get_value;
    }

    // Search all assemblies for a class by name
    Il2CppClass* find_class(const char* ns, const char* name) {
        if (!domain_get || !domain_get_assemblies || !assembly_get_image || !class_from_name) return nullptr;
        Il2CppDomain domain = domain_get();
        if (!domain) return nullptr;
        size_t count = 0;
        const Il2CppAssembly** asms = domain_get_assemblies(domain, &count);
        if (!asms) return nullptr;
        for (size_t i = 0; i < count; ++i) {
            const Il2CppImage* img = assembly_get_image(asms[i]);
            if (!img) continue;
            Il2CppClass* k = class_from_name(img, ns, name);
            if (k) return k;
        }
        return nullptr;
    }
};

extern IL2CPP_API g_il2cpp;

// PlayerController field offsets (from dump.cs, class TypeDefIndex 1540)
static constexpr uintptr_t PC_MAIN_CAMERA_HOLDER = 0x20;  // Transform*
static constexpr uintptr_t PC_VELOCITY_BF        = 0xB4;  // Vector3 <bzwv>
static constexpr uintptr_t PC_BOT_CONTROLLER     = 0xC0;  // BotPlayerController* (null = real player)
static constexpr uintptr_t PC_WEAPONRY_CTRL      = 0xD0;  // WeaponryController*
static constexpr uintptr_t PC_TEAM_ENUM          = 0x151; // cwf team byte

// PlayerManager field offsets
static constexpr uintptr_t PM_PLAYERS_DICT       = 0x28;  // Dictionary<int, PlayerController>
static constexpr uintptr_t PM_LOCAL_PLAYER_BF    = 0x40;  // PlayerController* <bzzh>

// LazySingleton<T> static instance field
static constexpr uintptr_t LS_INSTANCE_FIELD_OFF = 0x0;   // T cgxr

//
// Created by 24657 on 2022/12/27.
//

#include "hook.h"
#include <cstring>
#include <cstdio>
#include <unistd.h>
#include <sys/system_properties.h>
#include <dlfcn.h>
#include <dobby.h>
#include <cstdlib>
#include "game.h"
#include "il2cpp-tabledefs.h"
#include "il2cpp-class.h"
#include <inttypes.h>

#define DO_API(r, n, p) r (*n) p

#include "il2cpp-api-functions.h"

#undef DO_API

static uint64_t il2cpp_base = 0;

void init_il2cpp_api() {
#define DO_API(r, n, p) n = (r (*) p)dlsym(il2cpp_handle, #n)

#include "il2cpp-api-functions.h"

#undef DO_API
}

typedef int(*hook_invincible_ptr)(void  *a1, void *a2);


unsigned long get_module_base(const char* module_name)
{
    FILE *fp;
    unsigned long addr = 0;
    char *pch;
    char filename[32];
    char line[1024];

    snprintf(filename, sizeof(filename), "/proc/self/maps");

    fp = fopen(filename, "r");

    if (fp != nullptr) {
        while (fgets(line, sizeof(line), fp)) {
            if (strstr(line, module_name) && strstr(line, "r-xp")) {
                pch = strtok(line, "-");
                addr = strtoul(pch, nullptr, 16);
                if (addr == 0x8000)
                    addr = 0;
                break;
            }
        }
        fclose(fp);
    }
    return addr;
}

int is_game(JNIEnv *env, jstring appDataDir) {
    if (!appDataDir)
        return 0;
    const char *app_data_dir = env->GetStringUTFChars(appDataDir, nullptr);
    int user = 0;
    static char package_name[256];
    if (sscanf(app_data_dir, "/data/%*[^/]/%d/%s", &user, package_name) != 2) {
        if (sscanf(app_data_dir, "/data/%*[^/]/%s", package_name) != 1) {
            package_name[0] = '\0';
            LOGW("can't parse %s", app_data_dir);
            return 0;
        }
    }
    if (strcmp(package_name, GamePackageName) == 0) {
        LOGI("detect game: %s", package_name);
        game_data_dir = new char[strlen(app_data_dir) + 1];
        strcpy(game_data_dir, app_data_dir);
        env->ReleaseStringUTFChars(appDataDir, app_data_dir);
        return 1;
    } else {
        env->ReleaseStringUTFChars(appDataDir, app_data_dir);
        return 0;
    }
}

static int GetAndroidApiLevel() {
    char prop_value[PROP_VALUE_MAX];
    __system_property_get("ro.build.version.sdk", prop_value);
    return atoi(prop_value);
}

void dlopen_process(const char *name, void *handle) {
    //LOGD("dlopen: %s", name);
    if (!il2cpp_handle) {
        if (strstr(name, "libil2cpp.so")) {
            il2cpp_handle = handle;
            LOGI("Got il2cpp handle!");
        }
    }
}

HOOK_DEF(void*, __loader_dlopen, const char *filename, int flags, const void *caller_addr) {
    void *handle = orig___loader_dlopen(filename, flags, caller_addr);
    dlopen_process(filename, handle);
    return handle;
}

HOOK_DEF(void*, do_dlopen_V24, const char *name, int flags, const void *extinfo,
         void *caller_addr) {
    void *handle = orig_do_dlopen_V24(name, flags, extinfo, caller_addr);
    dlopen_process(name, handle);
    return handle;
}

HOOK_DEF(void*, do_dlopen_V19, const char *name, int flags, const void *extinfo) {
    void *handle = orig_do_dlopen_V19(name, flags, extinfo);
    dlopen_process(name, handle);
    return handle;
}

void hook_dlopen_for_il2cpp(){
    int api_level = GetAndroidApiLevel();
    LOGI("api level: %d", api_level);
    if (api_level >= 30) {
        void *addr = DobbySymbolResolver(nullptr,
                                         "__dl__Z9do_dlopenPKciPK17android_dlextinfoPKv");
        if (addr) {
            LOGI("do_dlopen at: %p", addr);
            DobbyHook(addr, reinterpret_cast<dobby_dummy_func_t>(new_do_dlopen_V24),
                      reinterpret_cast<void (**)()>(&orig_do_dlopen_V24));
        }
    } else if (api_level >= 26) {
        void *libdl_handle = dlopen("libdl.so", RTLD_LAZY);
        void *addr = dlsym(libdl_handle, "__loader_dlopen");
        LOGI("__loader_dlopen at: %p", addr);
        DobbyHook(addr, reinterpret_cast<dobby_dummy_func_t>(new___loader_dlopen),
                  reinterpret_cast<void (**)()>(&orig___loader_dlopen));
    } else if (api_level >= 24) {
        void *addr = DobbySymbolResolver(nullptr,
                                         "__dl__Z9do_dlopenPKciPK17android_dlextinfoPv");
        if (addr) {
            LOGI("do_dlopen at: %p", addr);
            DobbyHook(addr, reinterpret_cast<dobby_dummy_func_t>(new_do_dlopen_V24),
                      reinterpret_cast<void (**)()>(&orig_do_dlopen_V24));
        }
    } else {
        void *addr = DobbySymbolResolver(nullptr,
                                         "__dl__Z9do_dlopenPKciPK17android_dlextinfo");
        if (addr) {
            LOGI("do_dlopen at: %p", addr);
            DobbyHook(addr, reinterpret_cast<dobby_dummy_func_t>(new_do_dlopen_V19),
                      reinterpret_cast<void (**)()>(&orig_do_dlopen_V19));
        }
    }
    while (!il2cpp_handle) {
        sleep(1);
    }
    LOGD("find il2cpp_handle : %p",il2cpp_handle);
}

void *hack_thread(void *arg) {
    LOGI("hack thread: %d", gettid());
    hook_dlopen_for_il2cpp();
    sleep(2);
    init_il2cpp_api();
    if (il2cpp_domain_get_assemblies) {
        Dl_info dlInfo;
        if (dladdr((void *) il2cpp_domain_get_assemblies, &dlInfo)) {
            il2cpp_base = reinterpret_cast<uint64_t>(dlInfo.dli_fbase);
        } else {
            LOGW("dladdr error, using get_module_base.");
            il2cpp_base = get_module_base("libil2cpp.so");
        }
        LOGI("il2cpp_base: %" PRIx64"", il2cpp_base);
    } else {
        LOGE("Failed to initialize il2cpp api.");
        return nullptr;
    }
    auto domain = il2cpp_domain_get();
    il2cpp_thread_attach(domain);
    size_t size;
    auto assemblies = il2cpp_domain_get_assemblies(domain, &size);
    int assemble_csharp_index = 0;
    for(int i = 0; i < size; ++i) {
        auto image = il2cpp_assembly_get_image(assemblies[i]);
        if(strcmp(il2cpp_image_get_name(image), "Assembly-CSharp.dll") == 0) {
            assemble_csharp_index = i;
            LOGD("assemble_csharp_index : %d",assemble_csharp_index);
            break;
        }
    }
    auto image_game = il2cpp_assembly_get_image(assemblies[assemble_csharp_index]);
    Il2CppClass* clazz = il2cpp_class_from_name(image_game, "Namespace", "Classname");
//    DobbyHook(static_cast<void *>(il2cpp_class_get_method_from_name(clazz, "MethodName",
//                                                                    0)->methodPointer),
//              reinterpret_cast<dobby_dummy_func_t>(replace_func),
//              reinterpret_cast<void (**)()>(&backup_func));
    /**
     * TODD: 处理重载函数情况
     *       处理static字段 和 普通字段
     *      最后说下字段（field）方面的api，首先是取需要的字段，这个跟前面的method类似，
     *      使用il2cpp_class_get_field_from_name直接通过名称获取或者用il2cpp_class_get_fields遍历。拿到字段后要对值进行操作的话，
     *      就需要分下情况，对于静态字段，读取和写入直接使用il2cpp_field_static_get_value和il2cpp_field_static_set_value就行，
     *      如果是需要修改实例类的字段值，大多数情况都需要使用hook获取到你需要修改的实例类，然后再通过il2cpp_field_get_value和il2cpp_field_set_value进行读取和写入。
     */


    LOGD("hack game finish");
    return nullptr;
}

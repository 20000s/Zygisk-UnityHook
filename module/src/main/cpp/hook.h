//
// Created by 24657 on 2022/12/27.
//

#ifndef ZYGISK_MODULETEMPLATE_HOOK_H
#define ZYGISK_MODULETEMPLATE_HOOK_H
#include <jni.h>
static int enable_hack;
static void *il2cpp_handle = NULL;
static char *game_data_dir = NULL;

int is_game(JNIEnv *env, jstring appDataDir);
unsigned long get_module_base(const char* module_name);
void *hack_thread(void *arg);

#define HOOK_DEF(ret, func, ...) \
  ret (*orig_##func)(__VA_ARGS__); \
  ret new_##func(__VA_ARGS__)
#endif //ZYGISK_MODULETEMPLATE_HOOK_H

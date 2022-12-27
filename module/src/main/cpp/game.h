//
// Created by 24657 on 2022/12/27.
//

#ifndef ZYGISK_MODULETEMPLATE_GAME_H
#define ZYGISK_MODULETEMPLATE_GAME_H

#include <android/log.h>

#define LOG_TAG "UNITYHOOK"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

#define GamePackageName "your package name"

#endif //ZYGISK_MODULETEMPLATE_GAME_H

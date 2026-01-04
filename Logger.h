#pragma once

#ifdef kANDROID_LOG
#include <android/log.h>
#ifndef kANDROID_LOG_TAG
#define kANDROID_LOG_TAG "INJECT"
#endif
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, kANDROID_LOG_TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  kANDROID_LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,  kANDROID_LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, kANDROID_LOG_TAG, __VA_ARGS__)
#define LOGF(...) __android_log_print(ANDROID_LOG_FATAL, kANDROID_LOG_TAG, __VA_ARGS__)
#else
#define LOGD(...) printf(__VA_ARGS__)
#define LOGI(...) printf(__VA_ARGS__)
#define LOGW(...) printf(__VA_ARGS__)
#define LOGE(...) printf(__VA_ARGS__)
#define LOGF(...) printf(__VA_ARGS__)
#endif

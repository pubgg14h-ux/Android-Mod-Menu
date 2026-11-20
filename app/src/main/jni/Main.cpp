#include <list>
#include <vector>
#include <cstring>
#include <pthread.h>
#include <thread>
#include <string>
#include <jni.h>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <dlfcn.h>
#include <android/log.h>
#include "Includes/Logger.h"
#include "Includes/obfuscate.h"
#include "Includes/Utils.hpp"
#include "Menu/Menu.hpp"
#include "Menu/Jni.hpp"
#include "Includes/Macros.h"

// Функции IL2CPP
void* (*il2cpp_domain_get)();
void* (*il2cpp_domain_get_assemblies)(void* domain, size_t* size);
void* (*il2cpp_assembly_get_image)(void* assembly);
void* (*il2cpp_class_from_name)(void* image, const char* namespaze, const char* name);
void* (*il2cpp_class_get_method_from_name)(void* klass, const char* name, int argsCount);
void* (*il2cpp_runtime_invoke)(void* method, void* obj, void** params, void** exc);

bool esp_enabled = true; 

void* ESPThread(void*) {
    void* handle = nullptr;
    while (!handle) {
        handle = dlopen("libil2cpp.so", RTLD_LAZY);
        sleep(1);
    }

    il2cpp_domain_get = (void*(*)())dlsym(handle, "il2cpp_domain_get");
    il2cpp_domain_get_assemblies = (void*(*)(void*, size_t*))dlsym(handle, "il2cpp_domain_get_assemblies");
    il2cpp_assembly_get_image = (void*(*)(void*))dlsym(handle, "il2cpp_assembly_get_image");
    il2cpp_class_from_name = (void*(*)(void*, const char*, const char*))dlsym(handle, "il2cpp_class_from_name");
    il2cpp_class_get_method_from_name = (void*(*)(void*, const char*, int))dlsym(handle, "il2cpp_class_get_method_from_name");
    il2cpp_runtime_invoke = (void*(*)(void*, void*, void**, void**))dlsym(handle, "il2cpp_runtime_invoke");

    while (true) {
        if (esp_enabled) {
            size_t assemblies_count = 0;
            void* domain = il2cpp_domain_get();
            void** assemblies = (void**)il2cpp_domain_get_assemblies(domain, &assemblies_count);
            void* image = il2cpp_assembly_get_image(assemblies[0]);
            
            void* playerClass = il2cpp_class_from_name(image, "", "BasePlayer");
            if (!playerClass) playerClass = il2cpp_class_from_name(image, "", "Player");

            if (playerClass) {
                __android_log_print(ANDROID_LOG_INFO, "KUST_MOD", "BasePlayer found!");
            }
        }
        usleep(10000);
    }
    return nullptr;
}

// Заглушки для JNI функций, которые ищет Setup.cpp
extern "C" JNIEXPORT void JNICALL Java_uk_lgl_modmenu_Preferences_Changes(JNIEnv* env, jclass clazz, jobjectArray obj, jint i, jstring str, jint i2, jlong j, jbyte b, jstring str2) {}
extern "C" JNIEXPORT void JNICALL Java_uk_lgl_modmenu_Preferences_RegisterPreferences(JNIEnv* env, jclass clazz) {}

__attribute__((constructor))
void lib_main() {
    pthread_t esp_thread;
    pthread_create(&esp_thread, nullptr, ESPThread, nullptr);
}

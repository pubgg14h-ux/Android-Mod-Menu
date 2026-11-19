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
#include "imgui.h"
#include "imgui_internal.h"

// Функции IL2CPP
void* (*il2cpp_domain_get)();
void* (*il2cpp_domain_get_assemblies)(void* domain, size_t* size);
void* (*il2cpp_assembly_get_image)(void* assembly);
void* (*il2cpp_class_from_name)(void* image, const char* namespaze, const char* name);
void* (*il2cpp_class_get_method_from_name)(void* klass, const char* name, int argsCount);
void* (*il2cpp_runtime_invoke)(void* method, void* obj, void** params, void** exc);

bool esp_enabled = false;

void* CallMethod(void* obj, const char* methodName, int args = 0, void** params = nullptr) {
    if (!obj) return nullptr;
    void* klass = *(void**)obj;
    void* method = il2cpp_class_get_method_from_name(klass, methodName, args);
    if (method) return il2cpp_runtime_invoke(method, obj, params, nullptr);
    return nullptr;
}

struct Vector3 { float x, y, z; };

Vector3 GetPosition(void* transform) {
    Vector3 pos = {0, 0, 0};
    void* result = CallMethod(transform, "get_position", 0);
    if (result) pos = *(Vector3*)result;
    return pos;
}

bool WorldToScreen(Vector3 pos, Vector3* screen, float* matrix, int width, int height) {
    float w = matrix[3] * pos.x + matrix[7] * pos.y + matrix[11] * pos.z + matrix[15];
    if (w < 0.01f) return false;
    float invW = 1.0f / w;
    float x = (matrix[0] * pos.x + matrix[4] * pos.y + matrix[8] * pos.z + matrix[12]) * invW;
    float y = (matrix[1] * pos.x + matrix[5] * pos.y + matrix[9] * pos.z + matrix[13]) * invW;
    screen->x = (width / 2.0f) * (x + 1.0f);
    screen->y = (height / 2.0f) * (1.0f - y);
    return true;
}

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
                __android_log_print(ANDROID_LOG_INFO, "KUST_MOD", "BasePlayer class found!");
            }
        }
        usleep(10000);
    }
    return nullptr;
}

void DrawMenu() {
    ImGui::Begin("KUST MOD MENU", nullptr, ImGuiWindowFlags_NoCollapse);
    ImGui::Checkbox("ESP Enabled", &esp_enabled);
    ImGui::End();
}

__attribute__((constructor))
void lib_main() {
    pthread_t esp_thread;
    pthread_create(&esp_thread, nullptr, ESPThread, nullptr);
}

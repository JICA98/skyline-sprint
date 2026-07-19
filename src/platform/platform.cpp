#include "platform.h"
#include <iostream>

const char* BUILD_ID = "SkylineSprint-0.1.5-ui-polish";

#ifdef PS4
#include <orbis/libkernel.h>
#include <orbis/Sysmodule.h>
#include <semaphore.h>

extern "C" {

int sem_init(sem_t *sem, int pshared, unsigned int value) {
    (void)pshared;
    OrbisKernelSema handle;
    int rc = sceKernelCreateSema(&handle, "sdl_audio_sem", 0, value, 32767, nullptr);
    if (rc < 0) {
        return -1;
    }
    *(OrbisKernelSema*)(sem) = handle;
    return 0;
}

int sem_destroy(sem_t *sem) {
    OrbisKernelSema handle = *(OrbisKernelSema*)(sem);
    if (handle != nullptr) {
        sceKernelDeleteSema(handle);
        *(OrbisKernelSema*)(sem) = nullptr;
    }
    return 0;
}

int sem_wait(sem_t *sem) {
    OrbisKernelSema handle = *(OrbisKernelSema*)(sem);
    if (handle == nullptr) return -1;
    int rc = sceKernelWaitSema(handle, 1, nullptr);
    return (rc == 0) ? 0 : -1;
}

int sem_post(sem_t *sem) {
    OrbisKernelSema handle = *(OrbisKernelSema*)(sem);
    if (handle == nullptr) return -1;
    int rc = sceKernelSignalSema(handle, 1);
    return (rc == 0) ? 0 : -1;
}

int sem_trywait(sem_t *sem) {
    OrbisKernelSema handle = *(OrbisKernelSema*)(sem);
    if (handle == nullptr) return -1;
    int rc = sceKernelPollSema(handle, 1);
    return (rc == 0) ? 0 : -1;
}

int sem_getvalue(sem_t *sem, int *sval) {
    if (sval) *sval = 1;
    return 0;
}

}

const char* TOOLCHAIN_ID = "OpenOrbis-v0.5.4";

bool Platform_Init() {
    // Force load freetype module on PS4 to avoid unresolved NID crashes
    int rc = sceSysmoduleLoadModule(ORBIS_SYSMODULE_FREETYPE_OL);
    if (rc < 0) {
        std::cerr << "Failed to load FreeType module on PS4: " << rc << std::endl;
        return false;
    }
    return true;
}

void Platform_Shutdown() {
    // Unload system modules if necessary
}

std::string Platform_GetAssetPath(const std::string& relativePath) {
    return "/app0/" + relativePath;
}

bool Platform_IsPS4() {
    return true;
}

#else

const char* TOOLCHAIN_ID = "Desktop-Native";

bool Platform_Init() {
    // No specific initialization required on desktop
    return true;
}

void Platform_Shutdown() {
}

std::string Platform_GetAssetPath(const std::string& relativePath) {
    return "./package/" + relativePath;
}

bool Platform_IsPS4() {
    return false;
}

#endif

#include "platform.h"
#include <iostream>

const char* BUILD_ID = "SkylineSprint-0.1.2-final";

#ifdef PS4
#include <orbis/libkernel.h>
#include <orbis/Sysmodule.h>

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

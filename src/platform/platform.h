#ifndef PLATFORM_H
#define PLATFORM_H

#include <string>

// Build and Toolchain metadata
extern const char* BUILD_ID;
extern const char* TOOLCHAIN_ID;

// Initialize platform systems (Freetype modules, pad, etc.)
bool Platform_Init();

// Clean up platform systems
void Platform_Shutdown();

// Resolve correct runtime path for assets
std::string Platform_GetAssetPath(const std::string& relativePath);

// Retrieve whether the current run is on PS4
bool Platform_IsPS4();

#endif // PLATFORM_H

#pragma once

#if (defined(__APPLE__) || defined(__MACH__))
    #define GS_PLATFORM_MACOS
#elif (defined(__linux__) || defined(linux) || defined(__linux))
    #define GS_PLATFORM_LINUX
#else
    #error "Unknown platform detected"
#endif

#if defined(GS_PLATFORM_MACOS)
    #include <platforms/macos/platform.h>
#elif defined(GS_PLATFORM_LINUX)
    #include <platforms/linux/platform.h>
#else
    #error "Unable to include platform-specific code. Unknown platform"
#endif



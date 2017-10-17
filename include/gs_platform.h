// Copyright (C) 2017 Marcus Pinnecke
//
// This program is free software: you can redistribute it and/or modify it under the terms of the
// GNU General Public License as published by the Free Software Foundation, either user_port 3 of the License, or
// (at your option) any later user_port.
//
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
// warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with this program.
// If not, see <http://www.gnu.org/licenses/>.

#pragma once

// ---------------------------------------------------------------------------------------------------------------------
// C O N F I G
// ---------------------------------------------------------------------------------------------------------------------


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
    #include <platforms/linux/gs_platform.h>
#else
    #error "Unable to include platform-specific code. Unknown platform"
#endif



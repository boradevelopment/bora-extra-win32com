// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

#include <windows.h>
#include <shlwapi.h> // For SHDeleteKey

#pragma comment(lib, "Shlwapi.lib")
const CLSID CLSID_BappIconHandler = { 0x612feee4, 0x21ed, 0x43d9, { 0x8e, 0x1a, 0x97, 0xc7, 0xa6, 0xca, 0x5e, 0x74 } };

// {CEE31DAE-4374-41C9-89D0-5C4454ED4D01}
static const CLSID CLSID_BappPropertyHandler =
{ 0xcee31dae, 0x4374, 0x41c9, { 0x89, 0xd0, 0x5c, 0x44, 0x54, 0xed, 0x4d, 0x1 } };


// add headers that you want to pre-compile here
#include "framework.h"

#endif //PCH_H

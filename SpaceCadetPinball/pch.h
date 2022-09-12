// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file

#pragma once

#define _WIN32_WINNT  0x0501
#define NTDDI_VERSION 0x05010000

#define NOMINMAX

#include <sdkddkver.h>

// TODO: add headers that you want to pre-compile here
#include <windows.h>

#include <commctrl.h>
#include <htmlhelp.h>
#include <windowsx.h>

#include <array>
#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <type_traits> /*For control template*/

/*Sound uses PlaySound*/
#undef PlaySound

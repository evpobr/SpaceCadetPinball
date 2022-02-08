#include "pch.h"
#include "options.h"

#include "fullscrn.h"
#include "memory.h"
#include "midi.h"
#include "resource.h"
#include "Sound.h"
#include "winmain.h"

LPCSTR options::OptionsRegPath;
LPSTR options::OptionsRegPathCur;
HMENU options::MenuHandle;
optionsStruct options::Options;

winhelp_entry options::keymap_help[18]
{
	winhelp_entry{0x1F5, 0x3EA},
	winhelp_entry{0x191, 0x3EC},
	winhelp_entry{0x192, 0x3ED},
	winhelp_entry{0x193, 0x3F1},
	winhelp_entry{0x194, 0x3EE},
	winhelp_entry{0x195, 0x3EF},
	winhelp_entry{0x196, 0x3F0},
	winhelp_entry{0x385, 0x3EC},
	winhelp_entry{0x386, 0x3ED},
	winhelp_entry{0x387, 0x3F1},
	winhelp_entry{0x388, 0x3EE},
	winhelp_entry{0x389, 0x3EF},
	winhelp_entry{0x38A, 0x3F0},
	winhelp_entry{0x38C, -1},
	winhelp_entry{0x38D, -1},
	winhelp_entry{0x321, -1},
	winhelp_entry{0x322, -1},
	winhelp_entry{0, 0},
};

short options::vk_list[28]
{
	-32703,
	0x5A,
	-32720,
	0x39,
	0x402E,
	0x402F,
	0x403B,
	0x4027,
	0x405B,
	0x405D,
	0x20,
	0x0D,
	0x9,
	0x14,
	0x25,
	0x27,
	0x26,
	0x28,
	0x2D,
	0x2E,
	0x24,
	0x23,
	0x21,
	0x22,
	0x90,
	0x91,
	0x13,
	-1
};

void options::init(HMENU menuHandle)
{
	MenuHandle = menuHandle;
	Options.Sounds = 1;
	Options.Music = 0;
	Options.FullScreen = 0;
	Options.Average = 5;
	Options.PriorityAdj = 2;
	Options.LeftFlipperKeyDft = 90;
	Options.RightFlipperKeyDft = 191;
	Options.PlungerKeyDft = 32;
	Options.LeftTableBumpKeyDft = 88;
	Options.RightTableBumpKeyDft = 190;
	Options.BottomTableBumpKeyDft = 38;
	pinball::get_rc_int(159, &Options.LeftFlipperKeyDft);
	pinball::get_rc_int(160, &Options.RightFlipperKeyDft);
	pinball::get_rc_int(161, &Options.PlungerKeyDft);
	pinball::get_rc_int(162, &Options.LeftTableBumpKeyDft);
	pinball::get_rc_int(163, &Options.RightTableBumpKeyDft);
	pinball::get_rc_int(164, &Options.BottomTableBumpKeyDft);
	Options.LeftFlipperKey = Options.LeftFlipperKeyDft;
	Options.RightFlipperKey = Options.RightFlipperKeyDft;
	Options.PlungerKey = Options.PlungerKeyDft;
	Options.LeftTableBumpKey = Options.LeftTableBumpKeyDft;
	Options.RightTableBumpKey = Options.RightTableBumpKeyDft;
	Options.Players = 1;
	Options.BottomTableBumpKey = Options.BottomTableBumpKeyDft;
	Options.Sounds = get_int(nullptr, "Sounds", Options.Sounds);
	Options.Music = get_int(nullptr, "Music", Options.Music);
	Options.Average = get_int(nullptr, "Average", Options.Average);
	Options.FullScreen = get_int(nullptr, "FullScreen", Options.FullScreen);
	Options.PriorityAdj = get_int(nullptr, "Priority_Adjustment", Options.PriorityAdj);
	Options.Players = get_int(nullptr, "Players", Options.Players);
	Options.LeftFlipperKey = get_int(nullptr, "Left Flippper key", Options.LeftFlipperKey);
	Options.RightFlipperKey = get_int(nullptr, "Right Flipper key", Options.RightFlipperKey);
	Options.PlungerKey = get_int(nullptr, "Plunger key", Options.PlungerKey);
	Options.LeftTableBumpKey = get_int(nullptr, "Left Table Bump key", Options.LeftTableBumpKey);
	Options.RightTableBumpKey = get_int(nullptr, "Right Table Bump key", Options.RightTableBumpKey);
	Options.BottomTableBumpKey = get_int(nullptr, "Bottom Table Bump key", Options.BottomTableBumpKey);
	menu_check(Menu1_Sounds, Options.Sounds);
	Sound::Enable(0, 7, Options.Sounds);
	menu_check(Menu1_Music, Options.Music);
	menu_check(Menu1_Full_Screen, Options.FullScreen);
	menu_check(Menu1_1Player, Options.Players == 1);
	menu_check(Menu1_2Players, Options.Players == 2);
	menu_check(Menu1_3Players, Options.Players == 3);
	menu_check(Menu1_4Players, Options.Players == 4);
	auto tmpBuf = memory::allocate(0x1F4u);
	if (tmpBuf)
	{
		get_string(nullptr, "Shell Exe", tmpBuf, pinball::WindowName, 500);
		if (!*tmpBuf)
		{
			if (MenuHandle)
			{
				DeleteMenu(MenuHandle, Menu1_Select_Table, 0);
				DrawMenuBar(winmain::hwnd_frame);
			}
		}
		memory::free(tmpBuf);
	}
}

void options::uninit()
{
	set_int(nullptr, "Sounds", Options.Sounds);
	set_int(nullptr, "Music", Options.Music);
	set_int(nullptr, "FullScreen", Options.FullScreen);
	set_int(nullptr, "Players", Options.Players);
	set_int(nullptr, "Left Flippper key", Options.LeftFlipperKey);
	set_int(nullptr, "Right Flipper key", Options.RightFlipperKey);
	set_int(nullptr, "Plunger key", Options.PlungerKey);
	set_int(nullptr, "Left Table Bump key", Options.LeftTableBumpKey);
	set_int(nullptr, "Right Table Bump key", Options.RightTableBumpKey);
	set_int(nullptr, "Bottom Table Bump key", Options.BottomTableBumpKey);
}

void options::path_init(LPCSTR regPath)
{
	char* buf = memory::allocate(lstrlenA(regPath) + 1);
	OptionsRegPath = buf;
	if (buf)
		lstrcpyA(buf, regPath);
}

void options::path_uninit()
{
	if (OptionsRegPath)
		memory::free((void*)OptionsRegPath);
	OptionsRegPath = nullptr;
}

LPCSTR options::path(LPCSTR regPath)
{
	char* buf = OptionsRegPathCur;
	if (!OptionsRegPathCur)
	{
		buf = memory::allocate(0x7D0u);
		OptionsRegPathCur = buf;
		if (!buf)
			return OptionsRegPath;
	}
	lstrcpyA(buf, OptionsRegPath);
	if (!regPath)
		return OptionsRegPathCur;
	lstrcatA(OptionsRegPathCur, "\\");
	lstrcatA(OptionsRegPathCur, regPath);
	return OptionsRegPathCur;
}

void options::path_free()
{
	if (OptionsRegPathCur)
		memory::free(OptionsRegPathCur);
	OptionsRegPathCur = nullptr;
}


int options::get_int(LPCSTR optPath, LPCSTR lpValueName, int defaultValue)
{
	DWORD dwDisposition;	

	HKEY result = NULL;
	DWORD Data = 0;
	if (!OptionsRegPath)
		return defaultValue;
	LPCSTR regPath = path(optPath);
	if (!RegCreateKeyExA(HKEY_CURRENT_USER, regPath, 0, nullptr, 0, 0xF003Fu, nullptr, &result, &dwDisposition))
	{
		optPath = (LPCSTR)4;
		RegQueryValueExA(result, lpValueName, nullptr, nullptr, (LPBYTE)&Data, (LPDWORD)&optPath);
		RegCloseKey(result);
	}
	path_free();
	return (int)Data;
}

void options::set_int(LPCSTR optPath, LPCSTR lpValueName, int data)
{
	DWORD dwDisposition;

	if (OptionsRegPath)
	{
		const CHAR* regPath = path(optPath);
		if (!RegCreateKeyExA(HKEY_CURRENT_USER, regPath, 0, nullptr, 0, 0xF003Fu, nullptr, (PHKEY)&optPath,
		                     &dwDisposition))
		{
			RegSetValueExA((HKEY)optPath, lpValueName, 0, 4u, (const BYTE*)&data, 4u);
			RegCloseKey((HKEY)optPath);
		}
		path_free();
	}
}

void options::get_string(LPCSTR optPath, LPCSTR lpValueName, LPSTR lpString1, LPCSTR lpString2, int iMaxLength)
{
	lstrcpynA(lpString1, lpString2, iMaxLength);
	if (OptionsRegPath)
	{
		const CHAR* regPath = path(optPath);
		HKEY hkResult;
		if (RegCreateKeyExA(HKEY_CURRENT_USER, regPath, 0, nullptr, 0, 0xF003Fu, nullptr, &hkResult,
		                     (LPDWORD)&optPath))
		{
			RegQueryValueExA(hkResult, lpValueName, nullptr, nullptr, (LPBYTE)lpString1, (LPDWORD)&lpString2);
			RegCloseKey(hkResult);
		}
		path_free();
	}
}

void options::set_string(LPCSTR optPath, LPCSTR lpValueName, LPCSTR value)
{
	DWORD dwDisposition;

	if (OptionsRegPath)
	{
		const CHAR* regPath = path(optPath);
		if (!RegCreateKeyExA(HKEY_CURRENT_USER, regPath, 0, nullptr, 0, 0xF003Fu, nullptr, (PHKEY)&optPath,
		                     &dwDisposition))
		{
			int v4 = lstrlenA(value);
			RegSetValueExA((HKEY)optPath, lpValueName, 0, 1u, (const BYTE*)value, v4 + 1);
			RegCloseKey((HKEY)optPath);
		}
		path_free();
	}
}


void options::menu_check(UINT uIDCheckItem, int check)
{
	if (MenuHandle)
		CheckMenuItem(MenuHandle, uIDCheckItem, check != 0 ? 8 : 0);
}

void options::menu_set(UINT uIDEnableItem, int enable)
{
	if (MenuHandle)
		EnableMenuItem(MenuHandle, uIDEnableItem, enable == 0);
}


void options::toggle(UINT uIDCheckItem)
{
	int newValue;
	switch (uIDCheckItem)
	{
	case Menu1_Sounds:
		newValue = Options.Sounds == 0;
		Options.Sounds = Options.Sounds == 0;
		Sound::Enable(0, 7, newValue);
		menu_check(uIDCheckItem, newValue);
		return;
	case Menu1_Music:
		newValue = Options.Music == 0;
		Options.Music = Options.Music == 0;
		if (!newValue)
			midi::music_stop();
		else
			midi::play_pb_theme(0);
		menu_check(uIDCheckItem, newValue);
		return;
	case Menu1_Full_Screen:
		newValue = Options.FullScreen == 0;
		Options.FullScreen = Options.FullScreen == 0;
		fullscrn::set_screen_mode(newValue);
		menu_check(uIDCheckItem, newValue);
		return;
	}
	if (uIDCheckItem > 407 && uIDCheckItem <= 411)
	{
		Options.Players = uIDCheckItem - 407;
		menu_check(Menu1_1Player, Options.Players == 1);
		menu_check(Menu1_2Players, Options.Players == 2);
		menu_check(Menu1_3Players, Options.Players == 3);
		menu_check(Menu1_4Players, Options.Players == 4);
	}
}

void options::keyboard()
{
	DialogBoxParamA(winmain::hinst, "KEYMAPPER", winmain::hwnd_frame, KeyMapDlgProc, 0);
}

INT_PTR _stdcall options::KeyMapDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	char keyName[20];
	int keyBindings[6];
	char rcString[256];

	switch (msg)
	{
	case WM_HELP:
		WinHelpA(static_cast<HWND>(reinterpret_cast<HELPINFO*>(lParam)->hItemHandle), "pinball.hlp", HELP_WM_HELP,
		         (ULONG_PTR)keymap_help);
		return 1;
	case WM_CONTEXTMENU:
		WinHelpA((HWND)wParam, "pinball.hlp", HELP_CONTEXTMENU, (ULONG_PTR)keymap_help);
		return 1;
	case WM_INITDIALOG:
		for (auto vkPtr = vk_list; *vkPtr != -1; vkPtr++)
		{
			short vk = *vkPtr;
			auto vk2And = vk & 0x4000;
			auto vkChar = static_cast<unsigned __int8>(vk);
			unsigned short maxVk;

			if (vk2And)
			{
				auto index = 128;
				do
				{
					if (vkChar == MapVirtualKeyA(index, MAPVK_VK_TO_CHAR))
						break;
					++index;
				}
				while (index < 256);

				if (index == 256)
				{
					continue;
				}

				keyName[0] = static_cast<char>(vkChar);
				keyName[1] = 0;
				vkChar = index;
				maxVk = index;
			}
			else
			{
				if (vk >= 0)
				{
					maxVk = vkChar;
				}
				else
				{
					++vkPtr;
					maxVk = *vkPtr;
				}
				if (vkChar > maxVk)
				{
					continue;
				}
			}

			for (int curVK = vkChar; curVK <= maxVk; curVK++)
			{
				if (vk2And || get_vk_key_name(curVK, keyName))
				{
					auto ind = SendDlgItemMessageA(hDlg, KEYMAPPER_FlipperL,CB_INSERTSTRING, -1, (LPARAM)keyName);
					SendDlgItemMessageA(hDlg, KEYMAPPER_FlipperL, CB_SETITEMDATA, ind, curVK);
					if (curVK == Options.LeftFlipperKey)
						SendDlgItemMessageA(hDlg, KEYMAPPER_FlipperL, CB_SETCURSEL, ind, 0);
					ind = SendDlgItemMessageA(hDlg, KEYMAPPER_FlipperR, CB_INSERTSTRING, -1, (LPARAM)keyName);
					SendDlgItemMessageA(hDlg, KEYMAPPER_FlipperR, CB_SETITEMDATA, ind, curVK);
					if (curVK == Options.RightFlipperKey)
						SendDlgItemMessageA(hDlg, KEYMAPPER_FlipperR, CB_SETCURSEL, ind, 0);
					ind = SendDlgItemMessageA(hDlg, KEYMAPPER_Plunger, CB_INSERTSTRING, -1, (LPARAM)keyName);
					SendDlgItemMessageA(hDlg, KEYMAPPER_Plunger, CB_SETITEMDATA, ind, curVK);
					if (curVK == Options.PlungerKey)
						SendDlgItemMessageA(hDlg, KEYMAPPER_Plunger, CB_SETCURSEL, ind, 0);
					ind = SendDlgItemMessageA(hDlg, KEYMAPPER_BumpLeft, CB_INSERTSTRING, -1, (LPARAM)keyName);
					SendDlgItemMessageA(hDlg, KEYMAPPER_BumpLeft, CB_SETITEMDATA, ind, curVK);
					if (curVK == Options.LeftTableBumpKey)
						SendDlgItemMessageA(hDlg, KEYMAPPER_BumpLeft, CB_SETCURSEL, ind, 0);
					ind = SendDlgItemMessageA(hDlg, KEYMAPPER_BumpRight, CB_INSERTSTRING, -1, (LPARAM)keyName);
					SendDlgItemMessageA(hDlg, KEYMAPPER_BumpRight, CB_SETITEMDATA, ind, curVK);
					if (curVK == Options.RightTableBumpKey)
						SendDlgItemMessageA(hDlg, KEYMAPPER_BumpRight, CB_SETCURSEL, ind, 0);
					ind = SendDlgItemMessageA(hDlg, KEYMAPPER_BumpBottom, CB_INSERTSTRING, -1, (LPARAM)keyName);
					SendDlgItemMessageA(hDlg, KEYMAPPER_BumpBottom, CB_SETITEMDATA, ind, curVK);
					if (curVK == Options.BottomTableBumpKey)
						SendDlgItemMessageA(hDlg, KEYMAPPER_BumpBottom, CB_SETCURSEL, ind, 0);
				}
			}
		}
		return 1;
	case WM_COMMAND:
		switch (wParam)
		{
		case KEYMAPPER_Ok:
			{
				auto ind = SendDlgItemMessageA(hDlg, KEYMAPPER_FlipperL, CB_GETCURSEL, 0, 0);
				keyBindings[0] = SendDlgItemMessageA(hDlg, KEYMAPPER_FlipperL, CB_GETITEMDATA, ind, 0);
				ind = SendDlgItemMessageA(hDlg, KEYMAPPER_FlipperR, CB_GETCURSEL, 0, 0);
				keyBindings[1] = SendDlgItemMessageA(hDlg, KEYMAPPER_FlipperR, CB_GETITEMDATA, ind, 0);
				ind = SendDlgItemMessageA(hDlg, KEYMAPPER_Plunger, CB_GETCURSEL, 0, 0);
				keyBindings[2] = SendDlgItemMessageA(hDlg, KEYMAPPER_Plunger, CB_GETITEMDATA, ind, 0);
				ind = SendDlgItemMessageA(hDlg, KEYMAPPER_BumpLeft, CB_GETCURSEL, 0, 0);
				keyBindings[3] = SendDlgItemMessageA(hDlg, KEYMAPPER_BumpLeft, CB_GETITEMDATA, ind, 0);
				ind = SendDlgItemMessageA(hDlg, KEYMAPPER_BumpRight, CB_GETCURSEL, 0, 0);
				keyBindings[4] = SendDlgItemMessageA(hDlg, KEYMAPPER_BumpRight, CB_GETITEMDATA, ind, 0);
				ind = SendDlgItemMessageA(hDlg, KEYMAPPER_BumpBottom, CB_GETCURSEL, 0, 0);
				keyBindings[5] = SendDlgItemMessageA(hDlg, KEYMAPPER_BumpBottom, CB_GETITEMDATA, ind, 0);

				auto sameKeyBound = 0;
				auto index = 1;
				auto optPtr = keyBindings;
				while (!sameKeyBound)
				{
					for (auto keyInd = index; keyInd < 6; keyInd++)
					{
						if (sameKeyBound)
							break;
						if (*optPtr == keyBindings[keyInd])
						{
							lstrcpyA(rcString, pinball::get_rc_string(43, 0));
							MessageBoxA(hDlg, pinball::get_rc_string(39, 0), rcString, 0x2000u);
							sameKeyBound = 1;
						}
					}
					++index;
					++optPtr;
					if (index - 1 >= 5)
					{
						if (sameKeyBound)
							return 1;
						Options.LeftFlipperKey = keyBindings[0];
						Options.RightFlipperKey = keyBindings[1];
						Options.PlungerKey = keyBindings[2];
						Options.LeftTableBumpKey = keyBindings[3];
						Options.RightTableBumpKey = keyBindings[4];
						Options.BottomTableBumpKey = keyBindings[5];
						EndDialog(hDlg, wParam);
						return 1;
					}
				}
				return 1;
			}
		case KEYMAPPER_Cancel:
			EndDialog(hDlg, wParam);
			return 1;
		case KEYMAPPER_Default:
			{
				auto name = (LPARAM)get_vk_key_name(Options.LeftFlipperKeyDft, keyName);
				auto ind = SendDlgItemMessageA(hDlg, KEYMAPPER_FlipperL, CB_FINDSTRINGEXACT, 0, name);
				SendDlgItemMessageA(hDlg, KEYMAPPER_FlipperL, CB_SETCURSEL, ind, 0);
				name = (LPARAM)get_vk_key_name(Options.RightFlipperKeyDft, keyName);
				ind = SendDlgItemMessageA(hDlg, KEYMAPPER_FlipperR, CB_FINDSTRINGEXACT, 0, name);
				SendDlgItemMessageA(hDlg, KEYMAPPER_FlipperR, CB_SETCURSEL, ind, 0);
				name = (LPARAM)get_vk_key_name(Options.PlungerKeyDft, keyName);
				ind = SendDlgItemMessageA(hDlg, KEYMAPPER_Plunger, CB_FINDSTRINGEXACT, 0, name);
				SendDlgItemMessageA(hDlg, KEYMAPPER_Plunger, CB_SETCURSEL, ind, 0);
				name = (LPARAM)get_vk_key_name(Options.LeftTableBumpKeyDft, keyName);
				ind = SendDlgItemMessageA(hDlg, KEYMAPPER_BumpLeft, CB_FINDSTRINGEXACT, 0, name);
				SendDlgItemMessageA(hDlg, KEYMAPPER_BumpLeft, CB_SETCURSEL, ind, 0);
				name = (LPARAM)get_vk_key_name(Options.RightTableBumpKeyDft, keyName);
				ind = SendDlgItemMessageA(hDlg, KEYMAPPER_BumpRight, CB_FINDSTRINGEXACT, 0, name);
				SendDlgItemMessageA(hDlg, KEYMAPPER_BumpRight, CB_SETCURSEL, ind, 0);
				name = (LPARAM)get_vk_key_name(Options.BottomTableBumpKeyDft, keyName);
				ind = SendDlgItemMessageA(hDlg, KEYMAPPER_BumpBottom, CB_FINDSTRINGEXACT, 0, name);
				SendDlgItemMessageA(hDlg, KEYMAPPER_BumpBottom, CB_SETCURSEL, ind, 0);
				return 0;
			}
		default:
			return 0;
		}
	default:
		break;
	}

	return 0;
}


LPSTR options::get_vk_key_name(unsigned __int16 vk, LPSTR keyName)
{
	LONG scanCode = MapVirtualKeyA(vk, MAPVK_VK_TO_VSC) << 16;
	if (vk >= 0x21u && vk <= 0x2Eu)
		scanCode |= 0x1000000u;
	return GetKeyNameTextA(scanCode, keyName, 19) != 0 ? keyName : nullptr;
}

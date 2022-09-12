#pragma once
#include "pinball.h"

struct optionsStruct
{
	int Sounds;
	int Music;
	int Average;
	int FullScreen;
	int PriorityAdj;
	int Players;
	int LeftFlipperKey;
	int RightFlipperKey;
	int PlungerKey;
	int LeftTableBumpKey;
	int RightTableBumpKey;
	int BottomTableBumpKey;
	int LeftFlipperKeyDft;
	int RightFlipperKeyDft;
	int PlungerKeyDft;
	int LeftTableBumpKeyDft;
	int RightTableBumpKeyDft;
	int BottomTableBumpKeyDft;
};


class options
{
public:
	static void init(HMENU menuHandle);
	static void uninit();
	static void path_init(LPCSTR regPath);
	static void path_uninit();
	static int get_int(LPCSTR optPath, LPCSTR lpValueName, int defaultValue);
	static void set_int(LPCSTR optPath, LPCSTR lpValueName, int data);
	static void get_string(LPCSTR optPath, LPCSTR lpValueName, LPSTR lpString1, LPCSTR lpString2, int iMaxLength);
	static void set_string(LPCSTR optPath, LPCSTR lpValueName, LPCSTR value);
	static void menu_check(UINT uIDCheckItem, int check);
	static void menu_set(UINT uIDEnableItem, int enable);
	static void toggle(UINT uIDCheckItem);

	static void keyboard();
	static INT_PTR _stdcall KeyMapDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
	static LPSTR get_vk_key_name(uint16_t vk, LPSTR keyName);

	static optionsStruct Options;
private:
	static LPCSTR OptionsRegPath;
	static LPSTR OptionsRegPathCur;
	static LPCSTR path(LPCSTR regPath);
	static void path_free();
	static HMENU MenuHandle;
	static winhelp_entry keymap_help[18];
	static short vk_list[28];
};

#include "ActinidiaGo.h"
#include "bass\\bass.h"
#pragma comment(lib, "bass\\bass.lib")
#include "ActinidiaResLoader.h"

HINSTANCE hInst = NULL;
HWND hwnd;
#define MIN_WIDTH 200
#define MIN_HEIGHT 100
int WIN_WIDTH = 1024;
int WIN_HEIGHT = 768;

// Global functions for lua

// lua: Image CreateImage(width, height), Create Image with Black bg, must DeleteImage() in OnClose()
int CreateImage(lua_State *L)
{
	int n = lua_gettop(L);
	if (n != 2) return 0;
	int width = (int)lua_tointeger(L, 1);
	int height = (int)lua_tointeger(L, 2);
	lua_pop(L, 2);
	auto g = new ImageGraphic(width, height);
	lua_pushinteger(L, (DWORD)g);
	return 1;
}

// lua: Image CreateImageEx(width, height, color), Create Image with specific bg, must DeleteImage() in OnClose()
int CreateImageEx(lua_State *L)
{
	int n = lua_gettop(L);
	if (n != 3) return 0;
	int width = (int)lua_tointeger(L, 1);
	int height = (int)lua_tointeger(L, 2);
	auto c = lua_tointeger(L, 3);
	lua_pop(L, 3);
	auto g = new ImageGraphic(width, height, (color)c);
	lua_pushinteger(L, (DWORD)g);
	return 1;
}

// lua: Image CreateTransImage(width, height), Create Transparent Image, must DeleteImage() in OnClose()
int CreateTransImage(lua_State *L)
{
	int n = lua_gettop(L);
	if (n != 2) return 0;
	int width = (int)lua_tointeger(L, 1);
	int height = (int)lua_tointeger(L, 2);
	lua_pop(L, 2);
	auto g = new ImageGraphic(width, height, true);
	lua_pushinteger(L, (DWORD)g);
	return 1;
}

// lua: void DeleteImage(g)
int DeleteImage(lua_State *L)
{
	int n = lua_gettop(L);
	if (n != 1) return 0;
	auto g = (ImageGraphic*)(DWORD)lua_tointeger(L, 1);
	lua_pop(L, 1);
	delete g;
	return 0;
}

// lua: int GetWidth(g)
int GetWidth(lua_State *L)
{
	int n = lua_gettop(L);
	if (n != 1) return 0;
	auto g = (ImageGraphic*)(DWORD)lua_tointeger(L, 1);
	lua_pop(L, 1);
	lua_pushinteger(L, g->GetWidth());
	return 1;
}

// lua: int GetHeight(g)
int GetHeight(lua_State *L)
{
	int n = lua_gettop(L);
	if (n != 1) return 0;
	auto g = (ImageGraphic*)(DWORD)lua_tointeger(L, 1);
	lua_pop(L, 1);
	lua_pushinteger(L, g->GetHeight());
	return 1;
}

// lua: string GetText(pathname)
int GetText(lua_State *L)
{
	int n = lua_gettop(L);
	if (n != 1) return 0;
	std::string f = lua_tostring(L, 1);
	lua_pop(L, 1);
	LPBYTE pMem;
	long size = GetFile(f.c_str(), &pMem);	// resources will be released in EmptyStack()
	if (size > 0)
	{
		lua_pushlstring(L, (char*)pMem, size);
		return 1;
	}
	lua_pushnil(L);
	return 1;
}

// lua: image GetImage(pathname), must DeleteImage() in OnClose()
int GetImage(lua_State *L)
{
	int n = lua_gettop(L);
	if (n != 1) return 0;
	std::string f = lua_tostring(L, 1);
	lua_pop(L, 1);
	LPBYTE pMem;
	long size = GetFile(f.c_str(), &pMem);	// resources will be released in EmptyStack()
	if (size > 0)
	{
		auto g = new ImageGraphic();
		if (g->LoadImg(pMem, size))
		{
			lua_pushinteger(L, (DWORD)g);
			return 1;
		}
	}
	lua_pushnil(L);
	return 1;
}

// lua: sound GetSound(pathname, b_loop), if b_loop set true, must StopSound() in OnClose()
int GetSound(lua_State *L)
{
	int n = lua_gettop(L);
	if (n != 2) return 0;
	std::string f = lua_tostring(L, 1);
	int b_loop = lua_toboolean(L, 2);
	lua_pop(L, 2);
	LPBYTE pMem;
	long size = GetFile(f.c_str(), &pMem);	// resources will be released in EmptyStack()
	if (size > 0)
	{
		HSTREAM sound = BASS_StreamCreateFile(TRUE, pMem, 0, size,
			b_loop ? BASS_SAMPLE_LOOP : 0);
		if (sound)
		{
			lua_pushinteger(L, sound);
			return 1;
		}
	}
	lua_pushnil(L);
	return 1;
}

// lua: void EmptyStack(), free memory manually, must DeleteImage() and StopSound() in advance.
int EmptyStack(lua_State *L)
{
	for (unsigned long i = 0; i < iter; i++)
		free(ResStack[i]);
	iter = 0;
	return 0;
}

// lua: void PasteToImage(gDest, gSrc, xDest, yDest)
int PasteToImage(lua_State *L)
{
	int n = lua_gettop(L);
	if (n != 4) return 0;
	auto gDest = (ImageGraphic*)(DWORD)lua_tointeger(L, 1);
	auto gSrc = (ImageGraphic*)(DWORD)lua_tointeger(L, 2);
	int x = (int)lua_tointeger(L, 3);
	int y = (int)lua_tointeger(L, 4);
	lua_pop(L, 4);
	gDest->PasteFrom(gSrc, x, y);
	return 0;
}

// lua: void PasteToImageEx(gDest, gSrc, xDest, yDest, DestWidth, DestHeight, xSrc, ySrc, SrcWidth, SrcHeight)
int PasteToImageEx(lua_State *L)
{
	int n = lua_gettop(L);
	if (n != 10) return 0;
	auto gDest = (ImageGraphic*)(DWORD)lua_tointeger(L, 1);
	auto gSrc = (ImageGraphic*)(DWORD)lua_tointeger(L, 2);
	int xDest = (int)lua_tointeger(L, 3);
	int yDest = (int)lua_tointeger(L, 4);
	int DestWidth = (int)lua_tointeger(L, 5);
	int DestHeight = (int)lua_tointeger(L, 6);
	int xSrc = (int)lua_tointeger(L, 7);
	int ySrc = (int)lua_tointeger(L, 8);
	int SrcWidth = (int)lua_tointeger(L, 9);
	int SrcHeight = (int)lua_tointeger(L, 10);
	lua_pop(L, 10);
	gDest->PasteFrom(gSrc, xDest, yDest, DestWidth, DestHeight,
		xSrc, ySrc, SrcWidth, SrcHeight);
	return 0;
}

// lua: void AlphaBlend(gDest, gSrc, xDest, yDest, SrcAlpha)
int AlphaBlend(lua_State *L)
{
	int n = lua_gettop(L);
	if (n != 5) return 0;
	auto gDest = (ImageGraphic*)(DWORD)lua_tointeger(L, 1);
	auto gSrc = (ImageGraphic*)(DWORD)lua_tointeger(L, 2);
	int xDest = (int)lua_tointeger(L, 3);
	int yDest = (int)lua_tointeger(L, 4);
	unsigned char SrcAlpha = (unsigned char)lua_tointeger(L, 5);
	lua_pop(L, 5);
	gDest->BlendFrom(gSrc, xDest, yDest, SrcAlpha);
	return 0;
}

// lua: void AlphaBlendEx(gDest, gSrc, xDest, yDest, DestWidth, DestHeight, xSrc, ySrc, SrcWidth, SrcHeight, SrcAlpha)
int AlphaBlendEx(lua_State *L)
{
	int n = lua_gettop(L);
	if (n != 11) return 0;
	auto gDest = (ImageGraphic*)(DWORD)lua_tointeger(L, 1);
	auto gSrc = (ImageGraphic*)(DWORD)lua_tointeger(L, 2);
	int xDest = (int)lua_tointeger(L, 3);
	int yDest = (int)lua_tointeger(L, 4);
	int DestWidth = (int)lua_tointeger(L, 5);
	int DestHeight = (int)lua_tointeger(L, 6);
	int xSrc = (int)lua_tointeger(L, 7);
	int ySrc = (int)lua_tointeger(L, 8);
	int SrcWidth = (int)lua_tointeger(L, 9);
	int SrcHeight = (int)lua_tointeger(L, 10);
	unsigned char SrcAlpha = (unsigned char)lua_tointeger(L, 11);
	lua_pop(L, 11);
	gDest->BlendFrom(gSrc, xDest, yDest, DestWidth, DestHeight,
		xSrc, ySrc, SrcWidth, SrcHeight, SrcAlpha);
	return 0;
}

// lua: void PasteToWnd(WndGraphic, g)
int PasteToWnd(lua_State *L)
{
	int n = lua_gettop(L);
	if (n != 2) return 0;
	auto w = (DeviceContextGraphic*)(DWORD)lua_tointeger(L, 1);
	auto g = (ImageGraphic*)(DWORD)lua_tointeger(L, 2);
	lua_pop(L, 2);
	w->PasteFrom(g, 0, 0);
	return 0;
}

// lua: void PasteToWndEx(WndGraphic, g, xDest, yDest, DestWidth, DestHeight, xSrc, ySrc, SrcWidth, SrcHeight)
int PasteToWndEx(lua_State *L)
{
	int n = lua_gettop(L);
	if (n != 10) return 0;
	auto w = (DeviceContextGraphic*)(DWORD)lua_tointeger(L, 1);
	auto g = (ImageGraphic*)(DWORD)lua_tointeger(L, 2);
	int xDest = (int)lua_tointeger(L, 3);
	int yDest = (int)lua_tointeger(L, 4);
	int DestWidth = (int)lua_tointeger(L, 5);
	int DestHeight = (int)lua_tointeger(L, 6);
	int xSrc = (int)lua_tointeger(L, 7);
	int ySrc = (int)lua_tointeger(L, 8);
	int SrcWidth = (int)lua_tointeger(L, 9);
	int SrcHeight = (int)lua_tointeger(L, 10);
	lua_pop(L, 10);
	w->PasteFrom(g, xDest, yDest, DestWidth, DestHeight,
		xSrc, ySrc, SrcWidth, SrcHeight);
	return 0;
}

// lua: void StopSound(sound)
int StopSound(lua_State *L)
{
	int n = lua_gettop(L);
	if (n != 1) return 0;
	HSTREAM sound = (HSTREAM)lua_tointeger(L, 1);
	lua_pop(L, 1);
	if (BASS_ACTIVE_PLAYING == BASS_ChannelIsActive(sound))
		BASS_ChannelStop(sound);
	return 0;
}

// lua: void SetVolume(sound,volume), volume: 0-1
int SetVolume(lua_State *L)
{
	int n = lua_gettop(L);
	if (n != 2) return 0;
	HSTREAM sound = (HSTREAM)lua_tointeger(L, 1);
	float volume = (float)lua_tonumber(L, 2);
	lua_pop(L, 2);
	BASS_ChannelSetAttribute(sound, BASS_ATTRIB_VOL, volume);
	return 0;
}

// lua: void PlaySound(sound)
int PlaySound(lua_State *L)
{
	int n = lua_gettop(L);
	if (n != 1) return 0;
	HSTREAM sound = (HSTREAM)lua_tointeger(L, 1);
	lua_pop(L, 1);
	BASS_ChannelPlay(sound, true);
	return 0;
}

// lua: bool Screenshot(), true for success
int Screenshot(lua_State *L)
{
	int r = CreateDirectory(_T("screenshot"), NULL);
	//if (r != 0 && r != ERROR_ALREADY_EXISTS)
	//return false;

	TCHAR szFileName[120];

	SYSTEMTIME SysTime;
	GetSystemTime(&SysTime);
	static UINT n_png = 0;
	
	swprintf_s(szFileName, _T("screenshot\\%d-%d-%d-%d-%d-%d_%d.png"),
		SysTime.wYear, SysTime.wMonth, SysTime.wDay, SysTime.wHour, SysTime.wMinute, SysTime.wSecond, n_png++);

	HANDLE hFile;
	hFile = CreateFile(szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		lua_pushboolean(L, false);
		return 1;
	}

	HDC hdc = GetDC(hwnd);
	HDC tdc = CreateCompatibleDC(hdc);
	HBITMAP hbmp = CreateCompatibleBitmap(hdc, WIN_WIDTH, WIN_HEIGHT);
	HBITMAP hobmp = (HBITMAP)SelectObject(tdc, hbmp);
	BitBlt(tdc, 0, 0, WIN_WIDTH, WIN_HEIGHT, hdc, 0, 0, SRCCOPY);
	ReleaseDC(hwnd, hdc);
	DeleteObject(hobmp);
	DeleteDC(tdc);

	BITMAPFILEHEADER fileHead;
	int fileHeadLen = sizeof(BITMAPFILEHEADER);
	BITMAPINFOHEADER bmpHead;
	int bmpHeadLen = sizeof(BITMAPINFOHEADER);
	BITMAP bmpObj;
	GetObject(hbmp, sizeof(BITMAP), &bmpObj);
	DWORD fileSizeInByte;
	DWORD PixelSizeInBit;
	HDC srcDC = CreateDC(_T("DISPLAY"), NULL, NULL, NULL);
	PixelSizeInBit = GetDeviceCaps(srcDC, BITSPIXEL) * GetDeviceCaps(srcDC, PLANES);
	fileSizeInByte = fileHeadLen + bmpHeadLen + bmpObj.bmWidth*bmpObj.bmHeight*PixelSizeInBit / 8;
	fileHead.bfOffBits = fileHeadLen + bmpHeadLen;
	fileHead.bfReserved1 = 0;
	fileHead.bfReserved2 = 0;
	fileHead.bfSize = fileSizeInByte;
	fileHead.bfType = 0x4D42;
	bmpHead.biBitCount = (WORD)PixelSizeInBit;
	bmpHead.biCompression = BI_RGB;
	bmpHead.biPlanes = 1;
	bmpHead.biHeight = bmpObj.bmHeight;
	bmpHead.biWidth = bmpObj.bmWidth;
	bmpHead.biSize = bmpHeadLen;
	PBYTE pFile = new byte[fileSizeInByte];
	memset(pFile, 0, fileSizeInByte);
	memcpy(pFile, (PBYTE)&fileHead, fileHeadLen);
	memcpy(pFile + fileHeadLen, (PBYTE)&bmpHead, bmpHeadLen);
	GetDIBits(srcDC, hbmp, 0, bmpObj.bmHeight, pFile + fileHeadLen + bmpHeadLen, (LPBITMAPINFO)(pFile + fileHeadLen), DIB_RGB_COLORS);
	DWORD nByteTransfered;
	WriteFile(hFile, pFile, fileSizeInByte, &nByteTransfered, NULL);
	CloseHandle(hFile);
	delete pFile;
	DeleteDC(srcDC);

	lua_pushboolean(L, true);
	return 1;
}

// lua: string GetSetting(string key)
int GetSetting(lua_State *L)
{
	int n = lua_gettop(L);
	if (n != 1) return 0;
	std::string key = lua_tostring(L, 1);
	lua_pop(L, 1);
	try {
		std::string val = prop.at(key);	// at() will throw std::out_of_range if not exists
		lua_pushlstring(L, val.c_str(), val.size());
	}
	catch (std::out_of_range oor) {
		lua_pushnil(L);	
	}
	return 1;
}

// lua: void SaveSetting(string key, string value)
int SaveSetting(lua_State *L)
{
	int n = lua_gettop(L);
	if (n != 2) return 0;
	std::string key = lua_tostring(L, 1);
	std::string val = lua_tostring(L, 2);
	prop[key] = val;
	lua_pop(L, 2);
	return 0;
}
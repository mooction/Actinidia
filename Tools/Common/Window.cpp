#include "pch.h"
#include "Window.h"

#ifdef _WIN32

#include <map>
std::map<HWND, const Window*> callbackSource;

LRESULT CALLBACK dispacher(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_KEYDOWN:
		callbackSource[hWnd]->getKeyDownHandler()((uint32_t)wParam);
		break;
	case WM_KEYUP:
		callbackSource[hWnd]->getKeyUpHandler()((uint32_t)wParam);
		break;
	case WM_LBUTTONDOWN:
		callbackSource[hWnd]->getLButtonDownHandler()((uint32_t)wParam, (short)LOWORD(lParam), (short)HIWORD(lParam));
		break;
	case WM_LBUTTONUP:
		callbackSource[hWnd]->getLButtonUpHandler()((uint32_t)wParam, (short)LOWORD(lParam), (short)HIWORD(lParam));
		break;
	case WM_RBUTTONDOWN:
		callbackSource[hWnd]->getRButtonDownHandler()((uint32_t)wParam, (short)LOWORD(lParam), (short)HIWORD(lParam));
		break;
	case WM_RBUTTONUP:
		callbackSource[hWnd]->getRButtonUpHandler()((uint32_t)wParam, (short)LOWORD(lParam), (short)HIWORD(lParam));
		break;
	case WM_MOUSEMOVE:
		callbackSource[hWnd]->getMouseMoveHandler()((uint32_t)wParam, (short)LOWORD(lParam), (short)HIWORD(lParam));
		break;
	case WM_MOUSEWHEEL:
		callbackSource[hWnd]->getMouseWheelHandler()(
			GET_KEYSTATE_WPARAM(wParam),
			GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA,
			(short)LOWORD(lParam),
			(short)HIWORD(lParam)
		);
		break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		GdiCanvas gdi(hWnd, hdc);
		callbackSource[hWnd]->getPaintHandler()(gdi);
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

void Window::looper()
{
	HINSTANCE hInstance = GetModuleHandle(NULL);
	WNDCLASSEXW wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = dispacher;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = NULL;
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = title.c_str();
	wcex.hIconSm = NULL;
	RegisterClassExW(&wcex);
	this->hWnd = CreateWindowW(title.c_str(), title.c_str(), WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, width, height, nullptr, nullptr, hInstance, nullptr);
	if (!hWnd)
		return;
	callbackSource[hWnd] = this;
	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);

	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

void Window::refresh() const
{
	RECT rc;
	GetClientRect(hWnd, &rc);
	InvalidateRect(hWnd, &rc, FALSE);
}

#if 0
#if defined _M_IX86
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls'"\
" version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls'"\
" version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls'"\
" version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls'"\
" version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	Window w;
	w.create(L"demo");
}
#endif /* 0 */

#endif /* _WIN32 */

#ifdef _GTK

void Window::looper() const
{

}

void Window::refresh() const
{

}

#endif /* _GTK */

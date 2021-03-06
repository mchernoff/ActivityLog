#include "pch.h"
#include <winsock2.h>
#include "windows.h"
#include <ws2tcpip.h>
//#define _CRT_SECURE_NO_WARNINGS 1

#pragma comment (lib, "Ws2_32.lib")
using namespace std;

#define DEFAULT_PORT "11000"

WSADATA wsaData;
struct addrinfo *result = NULL,
	*ptr = NULL,
	hints;

HHOOK kHook = 0;
HHOOK mHook = 0;

SOCKET ConnectSocket = INVALID_SOCKET;
const char *key_code = "kdown";
const char *mouse_code = "mclick";

LRESULT CALLBACK KeyboardHook(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode == HC_ACTION)
	{
		if (wParam == WM_SYSKEYDOWN || wParam == WM_KEYDOWN)
		{
			int iResult = send(ConnectSocket, key_code, (int)strlen(key_code), 0);
		}
	}
	return CallNextHookEx(kHook, nCode, wParam, lParam);
}

LRESULT CALLBACK MouseHook(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode >= 0)
	{
		if (wParam == WM_MBUTTONDOWN || wParam == WM_LBUTTONDOWN || wParam == WM_RBUTTONDOWN)
		{
			int iResult = send(ConnectSocket, mouse_code, (int)strlen(mouse_code), 0);
		}
	}
	return CallNextHookEx(mHook, nCode, wParam, lParam);
}

int APIENTRY WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow)
{
	struct addrinfo *result = NULL,
		*ptr = NULL,
		hints;
	int iResult;
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	char ipaddress[128] = "";
	gethostname(ipaddress, sizeof(ipaddress));

	iResult = getaddrinfo(ipaddress, DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		WSACleanup();
		return 1;
	}

	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

		// Create a SOCKET for connecting to server
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
			ptr->ai_protocol);
		if (ConnectSocket == INVALID_SOCKET) {
			WSACleanup();
			return 1;
		}

		// Connect to server.
		iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			int err = WSAGetLastError();
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			return 1;
		}
		break;
	}

	kHook = SetWindowsHookEx(13, KeyboardHook, hInstance, 0);
	mHook = SetWindowsHookEx(WH_MOUSE_LL, MouseHook, hInstance, 0);
	while (GetMessage(NULL, NULL, 0, 0)); // NOP while not WM_QUIT

	//Flush output
	UnhookWindowsHookEx(mHook);
	return UnhookWindowsHookEx(kHook);
}
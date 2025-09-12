#include <iron_system.h>
#include <iron_net.h>
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <winhttp.h>

static char *returnData = NULL;
static int returnDataSize = 0;

void iron_https_request(const char *url_base, const char *url_path, const char *data, int port, int method,
                        iron_http_callback_t callback, void *callbackdata) {
	// based on https://docs.microsoft.com/en-us/windows/desktop/winhttp/winhttp-sessions-overview

	HINTERNET hSession = WinHttpOpen(L"WinHTTP via Iron/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);

	HINTERNET hConnect = NULL;
	if (hSession) {
		wchar_t wurl[4096];
		MultiByteToWideChar(CP_UTF8, 0, url_base, -1, wurl, 4096);
		hConnect = WinHttpConnect(hSession, wurl, port, 0);
	}

	HINTERNET hRequest = NULL;
	if (hConnect) {
		wchar_t wurl_path[4096];
		MultiByteToWideChar(CP_UTF8, 0, url_path, -1, wurl_path, 4096);
		hRequest = WinHttpOpenRequest(hConnect, method == IRON_HTTP_GET ? L"GET" : L"POST", wurl_path, NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
	}

	BOOL bResults = FALSE;
	if (hRequest) {
		DWORD optionalLength = (data != 0 && strlen(data) > 0) ? (DWORD)strlen(data) : 0;
		bResults = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
		                              data == 0 ? WINHTTP_NO_REQUEST_DATA : (LPVOID)data, optionalLength, optionalLength, 0);
	}

	if (bResults) {
		bResults = WinHttpReceiveResponse(hRequest, NULL);
	}

	int returnDataIndex = 0;
	if (bResults) {
		DWORD dwSize;
		do {
			dwSize = 0;
			if (!WinHttpQueryDataAvailable(hRequest, &dwSize)) {
				iron_error("Error %d in WinHttpQueryDataAvailable.\n", GetLastError());
			}

			if ((int)dwSize + 1 > returnDataSize - returnDataIndex) {
				returnDataSize = (returnDataIndex + dwSize + 1) * 2;
				returnData = realloc(returnData, returnDataSize);
			}

			DWORD dwDownloaded = 0;
			if (!WinHttpReadData(hRequest, (LPVOID)(&returnData[returnDataIndex]), dwSize, &dwDownloaded)) {
				iron_error("Error %d in WinHttpReadData.\n", GetLastError());
			}
			returnDataIndex += dwSize;
		} while (dwSize > 0);
	}
	else {
		callback(NULL, callbackdata);
		return;
	}

	returnData[returnDataIndex] = 0;

	if (!bResults) {
		iron_error("Error %d has occurred.\n", GetLastError());
	}

	if (hRequest) {
		WinHttpCloseHandle(hRequest);
	}
	if (hConnect) {
		WinHttpCloseHandle(hConnect);
	}
	if (hSession) {
		WinHttpCloseHandle(hSession);
	}

	callback(returnData, callbackdata);
}

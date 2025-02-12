#include <kinc/log.h>
#include <kinc/network/http.h>
#include <stdio.h>
#include <stdlib.h>
#include <winhttp.h>

static const wchar_t *convert(int method) {
	switch (method) {
	case KINC_HTTP_GET:
	default:
		return L"GET";
	case KINC_HTTP_POST:
		return L"POST";
	case KINC_HTTP_PUT:
		return L"PUT";
	case KINC_HTTP_DELETE:
		return L"DELETE";
	}
}

static char *returnData = NULL;
static int returnDataSize = 0;

void kinc_http_request(const char *url, const char *path, const char *data, int port, bool secure, int method, const char *header,
                       kinc_http_callback_t callback, void *callbackdata) {
	// based on https://docs.microsoft.com/en-us/windows/desktop/winhttp/winhttp-sessions-overview

	HINTERNET hSession = WinHttpOpen(L"WinHTTP via Kore/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);

	HINTERNET hConnect = NULL;
	if (hSession) {
		wchar_t wurl[4096];
		MultiByteToWideChar(CP_UTF8, 0, url, -1, wurl, 4096);
		hConnect = WinHttpConnect(hSession, wurl, port, 0);
	}

	HINTERNET hRequest = NULL;
	if (hConnect) {
		wchar_t wpath[4096];
		MultiByteToWideChar(CP_UTF8, 0, path, -1, wpath, 4096);
		hRequest =
		    WinHttpOpenRequest(hConnect, convert(method), wpath, NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, secure ? WINHTTP_FLAG_SECURE : 0);
	}

	BOOL bResults = FALSE;

	if (hRequest) {
		wchar_t wheader[4096];
		if (header) {
			MultiByteToWideChar(CP_UTF8, 0, header, -1, wheader, 4096);
		}
		DWORD optionalLength = (data != 0 && strlen(data) > 0) ? (DWORD)strlen(data) : 0;
		bResults = WinHttpSendRequest(hRequest, header == 0 ? WINHTTP_NO_ADDITIONAL_HEADERS : wheader, header == 0 ? 0 : -1L,
		                              data == 0 ? WINHTTP_NO_REQUEST_DATA : (LPVOID)data, optionalLength, optionalLength, 0);
	}

	if (bResults)
		bResults = WinHttpReceiveResponse(hRequest, NULL);

	int returnDataIndex = 0;
	if (bResults) {
		DWORD dwSize;
		do {
			dwSize = 0;
			if (!WinHttpQueryDataAvailable(hRequest, &dwSize)) {
				kinc_log(KINC_LOG_LEVEL_ERROR, "Error %d in WinHttpQueryDataAvailable.\n", GetLastError());
			}

			if ((int)dwSize + 1 > returnDataSize - returnDataIndex) {
				int newReturnDataSize = (returnDataIndex + dwSize + 1) * 2;
				char *newReturnData = (char *)malloc(newReturnDataSize);
				if (newReturnData == 0) {
					kinc_log(KINC_LOG_LEVEL_ERROR, "Out of memory\n");
				}
				memcpy(newReturnData, returnData, returnDataSize);
				returnDataSize = newReturnDataSize;
				returnData = newReturnData;
			}

			DWORD dwDownloaded = 0;
			if (!WinHttpReadData(hRequest, (LPVOID)(&returnData[returnDataIndex]), dwSize, &dwDownloaded)) {
				kinc_log(KINC_LOG_LEVEL_ERROR, "Error %d in WinHttpReadData.\n", GetLastError());
			}
			returnDataIndex += dwSize;
		} while (dwSize > 0);
	}
	else {
		callback(1, 404, NULL, callbackdata);
		return;
	}

	returnData[returnDataIndex] = 0;

	if (!bResults) {
		kinc_log(KINC_LOG_LEVEL_ERROR, "Error %d has occurred.\n", GetLastError());
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

	callback(0, 200, returnData, callbackdata);
}

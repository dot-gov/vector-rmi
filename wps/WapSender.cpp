#include <windows.h>
#include <stdio.h>

#include "WapPush.h"

// -p COM port
// -n Numero (in formato internazionale senza +)
// -l Link
// -t Text
// -c PIN code
// -x XML
void usage(WCHAR *argv) {
#ifdef _DEBUG
	wprintf(L"Usage: %s [options]\n\n", argv);
	wprintf(L"\t-h\t\tHelp: print this help\n\n");
	wprintf(L"\t-p <port>\tPort should be like: COM1, COM2, COM12\n\n");
	wprintf(L"\t-n <number>\tPhone number, in international format without \"+\"\n");
	wprintf(L"\t\t\t(e.g.: \"341234567890\")\n\n");
	wprintf(L"\t-l <link>\tHTTP link to the target\n");
	wprintf(L"\t\t\t(e.g.: http://www.google.com/file.txt)\n\n");
	wprintf(L"\t-t <text>\tText to show in the message\n");
	wprintf(L"\t\t\t(e.g.: \"This is a test\") (optional)\n\n");
	wprintf(L"\t-T <file>\tGet message text from <file>\n\n");
	wprintf(L"\t-s <service>\tService Type: \"sl\" (service loading) or\n");
	wprintf(L"\t\t\t\"si\" (service indication)\n\n");
	wprintf(L"\t\t\t\"sms\" (normal SMS)\n\n");
	wprintf(L"\t-f\t\tFlash sms: use in conjuction with type sms\n\n");
	wprintf(L"\t-r <priority>\tPriority: \"execute-[high, low] | cache\" for \"sl\" and\n");
	wprintf(L"\t\t\t\"signal-[none, low, medium, high] | delete\" for \"si\"\n\n");
	wprintf(L"\t-d <creation>\tCreation Date: needed for \"si\": \"YYYY-MM-DDTHH:MM:SS\"\n");
	wprintf(L"\t\t\t(e.g.: \"2011-01-01T23:59:59\")\n\n");
	wprintf(L"\t-c <PIN>\tPIN code if required (e.g.: \"1234\")\n\n");
	wprintf(L"\t-q\t\tQuery: check modem, use with -p and -c (if needed)\n\n");
	wprintf(L"\t-z\t\tCOM Check: check COM Port status, use with -p\n\n");
	wprintf(L"\t-x <XML>\tA valid XML Message\n");
	wprintf(L"\t\t\tNote: -x can only be used with -c, -p and -n\n\n");
	wprintf(L"\t-y\t\tReturns autodiscovered port\n");
#endif
}

int wmain(int argc, WCHAR* argv[]) {
	INT iNumArgs = 0, i;
	LPWSTR *ppwCommandLine = NULL;
	PWCHAR pwPort, pwNumber, pwLink, pwText, pwPin, pwXml;
	PWCHAR pwService, pwPriority, pwDate;
	BOOL bXml = FALSE, bArg = FALSE, bQuery = FALSE, bCom = FALSE, bFlash = FALSE;
	INT iRet = 4;

	WapPush wapObj;

	pwPort = pwNumber = pwLink = pwText = pwPin = pwXml = NULL;
	pwService = pwPriority = pwDate = NULL;

	// Parsing arguments
	ppwCommandLine = CommandLineToArgvW(GetCommandLineW(), &iNumArgs);

	if (ppwCommandLine == NULL) {
		wprintf(L"[ERROR] Cannot parse command line\n");
		return 4;
	}

	if (iNumArgs < 2) {
		usage(argv[0]);
		return 4;
	}

	for (i = 0; i < iNumArgs; i++) {
		if (ppwCommandLine[i][0] != '-')
			continue;

		if (wcslen(ppwCommandLine[i]) < 2) {
			wprintf(L"[ERROR] Missing parameter on argument %d\n", i + 1);
			LocalFree(ppwCommandLine);
			return 1;
		}

		switch (ppwCommandLine[i][1]) {
			case 'h':
				usage(argv[0]);
				return 4;

			case 'p':
				//wprintf(L"[DEBUG] -p\n");

				if (i + 1 < iNumArgs)
					pwPort = ppwCommandLine[i + 1];

				break;

			case 'n':
				//wprintf(L"[DEBUG] -n\n");

				if (i + 1 < iNumArgs)
					pwNumber = ppwCommandLine[i + 1];

				break;

			case 'l':
				//wprintf(L"[DEBUG] -l\n");
				bArg = TRUE;

				if (i + 1 < iNumArgs)
					pwLink = ppwCommandLine[i + 1];

				break;

			case 't':
				//wprintf(L"[DEBUG] -t\n");
				bArg = TRUE;

				if (i + 1 < iNumArgs)
					pwText = ppwCommandLine[i + 1];

				break;

			case 'T':
				//wprintf(L"[DEBUG] -T\n");
				bArg = TRUE;

				
				if (i + 1 < iNumArgs) {
					DWORD dwWritten = 0;
					HANDLE hTextFile = CreateFile(ppwCommandLine[i + 1], GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

					if (hTextFile == INVALID_HANDLE_VALUE) {
						wprintf(L"Cannot open file containing text message.\n");
						return 1;
					}

					DWORD dwLen = GetFileSize(hTextFile, NULL);

					if (dwLen < 2) {
						wprintf(L"Cannot read file containing text message.\n");
						return 1;
					}

					WCHAR *t = new WCHAR[dwLen / sizeof(WCHAR) + 1];
					ZeroMemory(t, dwLen + sizeof(WCHAR));

					ReadFile(hTextFile, t, dwLen, &dwWritten, NULL); 

					pwText = t;

					CloseHandle(hTextFile);
				}

				break;

			case 's':
				//wprintf(L"[DEBUG] -s\n");
				bArg = TRUE;

				if (i + 1 < iNumArgs)
					pwService = ppwCommandLine[i + 1];

				break;

			case 'f':
				bFlash = TRUE;

				break;

			case 'r':
				//wprintf(L"[DEBUG] -r\n");
				bArg = TRUE;

				if (i + 1 < iNumArgs)
					pwPriority = ppwCommandLine[i + 1];

				break;

			case 'd':
				//wprintf(L"[DEBUG] -d\n");
				bArg = TRUE;

				if (i + 1 < iNumArgs)
					pwDate = ppwCommandLine[i + 1];

				break;

			case 'c':
				//wprintf(L"[DEBUG] -c\n");

				if (i + 1 < iNumArgs)
					pwPin = ppwCommandLine[i + 1];

				break;

			case 'q':
				bQuery = TRUE;

				break;

			case 'z':
				bCom = TRUE;

				break;

			case 'x':
				//wprintf(L"[DEBUG] -x\n");
				bXml = TRUE;

				if (i + 1 < iNumArgs)
					pwXml = ppwCommandLine[i + 1];

				break;

			case 'y': {
				LocalFree(ppwCommandLine);

				INT port = wapObj.GetAutoDiscovered();

				if (port == -1) {
					wprintf(L"No suitable GSM modem has been found on any COM port\n");
				} else {
					wprintf(L"A suitable modem is running on port: COM%d\n", port);
				}

				return 0;
			}
			

			default:
#ifdef _DEBUG
				wprintf(L"[ERROR] Unknown parameter: \"-%c\"\n", ppwCommandLine[i][1]);
#endif
				LocalFree(ppwCommandLine);
				return 1;
		}
	}

	if (bArg && bXml) {
#ifdef _DEBUG
		wprintf(L"[ERROR] Parameter -x can only be used together with -p\n");
#endif
		LocalFree(ppwCommandLine);
		return 4;
	}

	if (bCom) {
		iRet = wapObj.CheckCOM(pwPort);
		LocalFree(ppwCommandLine);

		return iRet;
	}

	if (bQuery) {
		iRet = wapObj.CheckModem(pwPort, pwPin);
		LocalFree(ppwCommandLine);

		return iRet;
	}

	// Start dealing with GSM modem
	iRet = wapObj.SendMessage(pwPort, pwPin, pwNumber, pwText, pwService, pwPriority, pwLink, pwDate, bFlash);
/*
	if (wapObj.CheckModem() == FALSE) {
		wprintf(L"[ERROR] Modem is not able to send WAP Push messages\n");
		LocalFree(ppwCommandLine);
		return 1;
	}
*/
	LocalFree(ppwCommandLine);
	return iRet;
}
#include "framework.h"

#include "../GameCommon/CCommandLine.h"
#include "../GameCommon/CErrorLog.h"
#include "../GameCommon/CLocal.h"
#include "../GameCommon/CString.h"
#include "../GameCommon/CWebBrowser.h"
#include "../GameCommon/CWindow.h"
#include "../GameCommon/CXML.h"

#include "GameManager.h"

CErrorLog* m_errorLog;

char m_model[32] = {};
char m_name[32] = {};

CLocal* m_local;
CWebBrowser* m_webBrowser;
CWindow* m_window;
CXML m_xml;

HRESULT m_hr;

PROCESS_INFORMATION m_startClientProcessInfo;

STARTUPINFO m_startClientStartupInfo;

LRESULT CALLBACK WndProc(HWND, uint32_t, WPARAM, LPARAM);
INT_PTR CALLBACK About(HWND, uint32_t, WPARAM, LPARAM);

/*
*/
int32_t APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int32_t nCmdShow)
{
	m_hr = OleInitialize(0);

	m_local = new CLocal();

	CString* logName = new CString(m_local->m_installPath->m_text);

	logName->Append("GameManagerLog.txt");

	m_errorLog = new CErrorLog(logName->m_text);

	SAFE_DELETE(logName);

	m_window = new CWindow(hInstance, WndProc, "GameManagerClass", IDC_GAMEMANAGER, IDI_GAMEMANAGER, IDI_SMALL, "Game Manager", 1024, 700, 0, 0);

	m_webBrowser = new CWebBrowser(m_window->m_hWnd, m_local);

	m_webBrowser->Bounds();

	m_webBrowser->SetURL("http://127.0.0.1:49151/game/login.html");

	m_webBrowser->Create();

	MSG msg = {};

	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, m_window->m_hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			
			DispatchMessage(&msg);
		}
	}

	SAFE_DELETE(m_local);
	SAFE_DELETE(m_webBrowser);
	SAFE_DELETE(m_errorLog);
	SAFE_DELETE(m_window);

	OleUninitialize();

	return (int32_t)msg.wParam;
}

/*
*/
LRESULT CALLBACK WndProc(HWND hWnd, uint32_t message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_COMMAND:
	{
		int32_t wmId = LOWORD(wParam);

		switch (wmId)
		{
		case IDM_ABOUT:
		{
			DialogBox(m_window->m_hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);

			break;
		}
		case IDM_EXIT:
		{
			DestroyWindow(hWnd);
			
			break;
		}
		case CWebBrowser::E_IDM_BROWSER_FROM_DOCUMENT:
		{
			m_xml.InitBuffer((char*)lParam);

			BYTE type = {};
			char valid = {};

			while (!m_xml.CheckEndOfBuffer())
			{
				if (strncmp(m_xml.m_buffer, "<login>", 7) == 0)
				{
					type = 0;

					m_xml.Move(7);
				}
				else if (strncmp(m_xml.m_buffer, "<modelSelect>", 13) == 0)
				{
					type = 1;

					m_xml.Move(13);
				}
				else if (strncmp(m_xml.m_buffer, "<valid>", 7) == 0)
				{
					m_xml.Move(7);

					char* v = m_xml.GetValue();

					valid = *v;

					m_xml.MoveToEnd();
				}
				else if (strncmp(m_xml.m_buffer, "<name>", 6) == 0)
				{
					m_xml.Move(6);

					char* v = m_xml.GetValue();

					memset(m_name, 0x00, 32);
					memcpy_s(m_name, 32, v, strlen(v));

					m_xml.MoveToEnd();
				}
				else if (strncmp(m_xml.m_buffer, "<model>", 7) == 0)
				{
					m_xml.Move(7);

					char* v = m_xml.GetValue();

					memset(m_model, 0x00, 32);
					memcpy_s(m_model, 32, v, strlen(v));

					m_xml.MoveToEnd();
				}
				else
				{
					m_xml.Move(1);
				}
			}

			switch (type)
			{
			case 0:
			{
				if (valid == 'Y')
				{
					m_webBrowser->SendMessageToDocument(L"Logged In");

					m_webBrowser->m_webview->Navigate(L"http://127.0.0.1:49151/game/lobby.html");

					return 0;
				}

				m_webBrowser->SendMessageToDocument(L"Login Invalid");

				break;
			}
			case 1:
			{
				m_startClientStartupInfo = {};
				m_startClientStartupInfo.cb = sizeof(STARTUPINFO);

				m_startClientProcessInfo = {};

				CString* commandLine = new CString("GameClient.exe 127.0.0.1 49152 49153 ");

				commandLine->Append(m_name);

				commandLine->Append(" ");

				commandLine->Append(m_model);

				CreateProcess(NULL, // No module name (use command line)
					commandLine->GetWide(),           // Command line
					NULL,           // Process handle not inheritable
					NULL,           // Thread handle not inheritable
					FALSE,          // Set handle inheritance to FALSE
					0,              // No creation flags
					NULL,           // Use parent's environment block
					NULL,           // Use parent's starting directory
					&m_startClientStartupInfo,
					&m_startClientProcessInfo);

				SAFE_DELETE(commandLine);

				break;
			}
			}

			return 0;
		}
		default:
		{
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		}

		break;
	}
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		
		BeginPaint(hWnd, &ps);
		
		EndPaint(hWnd, &ps);

		break;
	}
	case WM_DESTROY:
	{
		PostThreadMessage(m_startClientProcessInfo.dwThreadId, WM_QUIT, 0, 0);

		PostQuitMessage(0);
		
		break;
	}
	default:
	{
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	}

	return 0;
}

/*
*/
INT_PTR CALLBACK About(HWND hDlg, uint32_t message, WPARAM wParam, LPARAM lParam)
{
	lParam;

	switch (message)
	{
	case WM_INITDIALOG:
	{
		return (INT_PTR)TRUE;
	}

	case WM_COMMAND:
	{
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
		
			return (INT_PTR)TRUE;
		}
	
		break;
	}
	}

	return (INT_PTR)FALSE;
}
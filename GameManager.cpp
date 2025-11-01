#include "framework.h"

#include "../GameCommon/CCommandLine.h"
#include "../GameCommon/CErrorLog.h"
#include "../GameCommon/CLocal.h"
#include "../GameCommon/CString.h"
#include "../GameCommon/CWebBrowser.h"
#include "../GameCommon/CXML.h"

#include "GameManager.h"

CErrorLog* m_errorLog;

char m_model[32] = {};
char m_name[32] = {};

CLocal* m_local;

constexpr auto MAX_LOADSTRING = 100;

CWebBrowser* m_webBrowser;
CXML m_xml;

HINSTANCE m_hInst;

HRESULT m_hr;

HWND m_hWnd;

PROCESS_INFORMATION m_startClientProcessInfo;

STARTUPINFO m_startClientStartupInfo;

WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];

ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

/*
*/
int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	hPrevInstance;
	lpCmdLine;

	m_hr = OleInitialize(0);

	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_GAMEMANAGER, szWindowClass, MAX_LOADSTRING);
	
	MyRegisterClass(hInstance);

	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_GAMEMANAGER));

	MSG msg = {};

	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			
			DispatchMessage(&msg);
		}
	}

	delete m_local;
	delete m_webBrowser;
	delete m_errorLog;

	OleUninitialize();

	return (int)msg.wParam;
}

/*
*/
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex = {};

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GAMEMANAGER));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_GAMEMANAGER);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

/*
*/
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	m_hInst = hInstance;

	int x = (GetSystemMetrics(SM_CXSCREEN) / 2) - 1024 / 2;
	int y = (GetSystemMetrics(SM_CYSCREEN) / 2) - 700 / 2;

	m_hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		x, y,
		1024, 700,
		nullptr, nullptr, hInstance, nullptr);

	if (!m_hWnd)
	{
		return FALSE;
	}

	m_local = new CLocal();

	CString* logName = new CString(m_local->m_installPath->m_text);

	logName->Append("GameManagerLog.txt");

	m_errorLog = new CErrorLog(logName->m_text);

	delete logName;

	m_webBrowser = new CWebBrowser(m_hWnd, m_local);

	m_webBrowser->Bounds();

	m_webBrowser->SetURL("http://127.0.0.1:26105/game/login.php/");

	m_webBrowser->Create();

	ShowWindow(m_hWnd, nCmdShow);
	UpdateWindow(m_hWnd);

	return TRUE;
}

/*
*/
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		switch (wmId)
		{
		case IDM_ABOUT:
		{
			DialogBox(m_hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);

			break;
		}
		case IDM_EXIT:
		{
			DestroyWindow(hWnd);
			
			break;
		}
		case WEBVIEW_COMMAND:
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

					m_webBrowser->m_webview->Navigate(L"http://127.0.0.1:26105/game/lobby.php/");

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

				delete commandLine;

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
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
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
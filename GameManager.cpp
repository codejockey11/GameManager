// GameManager.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "GameManager.h"

constexpr auto MAX_LOADSTRING = 100;

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name


// NuGet project items
#include <stdlib.h>
#include <string>

// Pointer to WebViewController
static wil::com_ptr<ICoreWebView2Controller> webviewController;

// Pointer to WebView window
static wil::com_ptr<ICoreWebView2> webview;


enum Controls
{
	BTN_STARTCLIENT = 200
};

HWND m_startClient;

STARTUPINFO m_startClientStartupInfo;
PROCESS_INFORMATION m_startClientProcessInfo;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: Place code here.
	HRESULT hr = OleInitialize(0);




	// Initialize global strings
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_GAMEMANAGER, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_GAMEMANAGER));

	MSG msg;

	// Main message loop:
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}



	OleUninitialize();


	return (int)msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
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

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // Store instance handle in our global variable

	int x = (GetSystemMetrics(SM_CXSCREEN) / 2) - 1024 / 2;
	int y = (GetSystemMetrics(SM_CYSCREEN) / 2) - 700 / 2;

	HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		x, y,
		1024, 700, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		return FALSE;
	}


	std::wstring m_userDataFolder = L"C:\\Users\\junk_\\source\\repos\\udf";

	CreateCoreWebView2EnvironmentWithOptions(nullptr, m_userDataFolder.c_str(), nullptr,
		Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
			[hWnd](HRESULT result, ICoreWebView2Environment* env) -> HRESULT {

				// Create a CoreWebView2Controller and get the associated CoreWebView2 whose parent is the main window hWnd
				env->CreateCoreWebView2Controller(hWnd, Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
					[hWnd](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT {
						if (controller != nullptr) {
							webviewController = controller;
							webviewController->get_CoreWebView2(&webview);
						}

	// Add a few settings for the webview
	// The demo step is redundant since the values are the default settings
	wil::com_ptr<ICoreWebView2Settings> settings;
	webview->get_Settings(&settings);
	settings->put_IsScriptEnabled(TRUE);
	settings->put_AreDefaultScriptDialogsEnabled(TRUE);
	settings->put_IsWebMessageEnabled(TRUE);

	// Resize WebView to fit the bounds of the parent window
	RECT bounds;
	GetClientRect(hWnd, &bounds);
	bounds.left += 10;
	bounds.top += 10;
	bounds.right -= 10;
	bounds.bottom -= 50;

	webviewController->put_Bounds(bounds);

	// Schedule an async task to navigate to Bing
	webview->Navigate(L"https://www.google.com/");

	// <NavigationEvents>
	// Step 4 - Navigation events
	// register an ICoreWebView2NavigationStartingEventHandler to cancel any non-https navigation
	EventRegistrationToken token;
	webview->add_NavigationStarting(Callback<ICoreWebView2NavigationStartingEventHandler>(
		[](ICoreWebView2* webview, ICoreWebView2NavigationStartingEventArgs* args) -> HRESULT {
			wil::unique_cotaskmem_string uri;
	args->get_Uri(&uri);
	std::wstring source(uri.get());
	if (source.substr(0, 5) != L"https") {
		//args->put_Cancel(true);
	}
	return S_OK;
		}).Get(), &token);
	// </NavigationEvents>

	// <Scripting>
	// Step 5 - Scripting
	// Schedule an async task to add initialization script that freezes the Object object
	webview->AddScriptToExecuteOnDocumentCreated(L"Object.freeze(Object);", nullptr);
	// Schedule an async task to get the document URL
	webview->ExecuteScript(L"window.document.URL;", Callback<ICoreWebView2ExecuteScriptCompletedHandler>(
		[](HRESULT errorCode, LPCWSTR resultObjectAsJson) -> HRESULT {
			LPCWSTR URL = resultObjectAsJson;
	//doSomethingWithURL(URL);
	return S_OK;
		}).Get());
	// </Scripting>

	// <CommunicationHostWeb>
	// Step 6 - Communication between host and web content
	// Set an event handler for the host to return received message back to the web content
	webview->add_WebMessageReceived(Callback<ICoreWebView2WebMessageReceivedEventHandler>(
		[](ICoreWebView2* webview, ICoreWebView2WebMessageReceivedEventArgs* args) -> HRESULT {
			wil::unique_cotaskmem_string message;
	args->TryGetWebMessageAsString(&message);
	// processMessage(&message);
	webview->PostWebMessageAsString(message.get());
	return S_OK;
		}).Get(), &token);

	// <WindowCloseRequested>
	// Register a handler for the WindowCloseRequested event.
	// This handler will close the app window if it is not the main window.
	webview->add_WindowCloseRequested(
		Callback<ICoreWebView2WindowCloseRequestedEventHandler>(
			[hWnd](ICoreWebView2* sender, IUnknown* args)
			{
				//m_browserOpen = false;

				webviewController->Close();

	return S_OK;
			})
		.Get(),
				nullptr);
	// <WindowCloseRequested>

	// Schedule an async task to add initialization script that
	// 1) Add an listener to print message from the host
	// 2) Post document URL to the host
	/*
	webview->AddScriptToExecuteOnDocumentCreated(
		L"window.chrome.webview.addEventListener(\'message\', event => alert(event.data));" \
		L"window.chrome.webview.postMessage(window.document.URL);",
		nullptr);
		*/
		// </CommunicationHostWeb>

	return S_OK;
					}).Get());
	return S_OK;
			}).Get());



	// Resize WebView to fit the bounds of the parent window
	RECT bounds;
	GetClientRect(hWnd, &bounds);
	bounds.left += 10;
	bounds.top += 10;
	bounds.right -= 10;
	bounds.bottom -= 35;



	m_startClient = CreateWindow(WC_BUTTON, L"Client", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
		/* position */ bounds.right - 64, bounds.bottom,
		/* size     */ 64, 22,
		hWnd,
		(HMENU)Controls::BTN_STARTCLIENT,
		(HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);

			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		case Controls::BTN_STARTCLIENT:
		{
			m_startClientStartupInfo = {};
			m_startClientStartupInfo.cb = sizeof(STARTUPINFO);

			m_startClientProcessInfo = {};

			WCHAR accountInfo[] = L"GameClient.exe Steve 127.0.0.1 26105";

#ifdef _DEBUG
			bool r = CreateProcess(L"C:/Users/junk_/source/repos/GameClient/x64/Debug/GameClient.exe", // No module name (use command line)
				accountInfo,           // Command line
				NULL,           // Process handle not inheritable
				NULL,           // Thread handle not inheritable
				FALSE,          // Set handle inheritance to FALSE
				0,              // No creation flags
				NULL,           // Use parent's environment block
				NULL,           // Use parent's starting directory 
				&m_startClientStartupInfo,            // Pointer to STARTUPINFO structure
				&m_startClientProcessInfo);           // Pointer to PROCESS_INFORMATION structure
#else
			bool r = CreateProcess(L"GameClient.exe", // No module name (use command line)
				accountInfo,           // Command line
				NULL,           // Process handle not inheritable
				NULL,           // Thread handle not inheritable
				FALSE,          // Set handle inheritance to FALSE
				0,              // No creation flags
				NULL,           // Use parent's environment block
				NULL,           // Use parent's starting directory 
				&m_startClientStartupInfo,            // Pointer to STARTUPINFO structure
				&m_startClientProcessInfo);           // Pointer to PROCESS_INFORMATION structure
#endif
			return 0;

		}
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code that uses hdc here...
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_DESTROY:
	{
		PostThreadMessage(m_startClientProcessInfo.dwThreadId, WM_QUIT, 0, 0);

		PostQuitMessage(0);
		break;
	}
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
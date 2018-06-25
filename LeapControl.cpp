// LeapControl.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "ShellAPI.h"
#include "LeapControl.h"
#include "Leap.h"
#include <math.h>

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

//�궨��
#define APP_NAME L"LeapMotion PPT���Ƴ���"		//����� 2014.4.6

//��������
Leap::Controller controller;	//leapmotion controller
NOTIFYICONDATA nid;				//��������  
HMENU hMenu;					//���̲˵�
const UINT NID = 1;
const UINT WM_TrayNotify = WM_USER+123;
const UINT ID_TRAYEXIT = 1;
const unsigned long TIME_ELAPSE = 500;	//�ж�������Чswipe����֮�����С���
HWND ghWnd = 0;

//��������
void isLeapmotionConnected();

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_LEAPCONTROL, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}
	
	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_LEAPCONTROL));

	//��ʼ��LeapMotion
	int64_t lastFrameID = -1;
	int lastSwipeID = -1;
	DWORD timeElapse = 0;
	bool leapmotion_connected = false;

	// Main message loop:
	while (TRUE) 
	{ 
		if (PeekMessage (&msg, NULL, 0, 0, PM_REMOVE)) 
		{ 
			if (msg.message == WM_QUIT) 
				break ; 
			TranslateMessage (&msg); 
			DispatchMessage (&msg); 
		} 
		else 
		{
			//�ж�leapmotion�Ƿ�����
			if(!controller.isConnected())
			{
				leapmotion_connected = false;
				Sleep(500);
				continue;
			}

			//�״������������ƺͺ�̨����
			if(!leapmotion_connected)
			{
				PostMessage(ghWnd, WM_TrayNotify, NULL, WM_LBUTTONDOWN);
				lastFrameID = -1;
				lastSwipeID = -1;
				leapmotion_connected = true;
				controller.enableGesture(Leap::Gesture::TYPE_SWIPE);
				controller.setPolicyFlags(Leap::Controller::PolicyFlag::POLICY_BACKGROUND_FRAMES);
			}

			//��ȡframe
			Leap::Frame frame = controller.frame();

			//�ж��Ƿ����µ�frame
			if (lastFrameID >= frame.id())
			{
				Sleep(100);
				continue;
			}

			lastFrameID = frame.id();

			Leap::GestureList gl = frame.gestures();
			for(int i=0; i<gl.count(); i++)
			{
				if(gl[i].type() == Leap::Gesture::TYPE_SWIPE /*&& gl[i].state() == Leap::Gesture::STATE_UPDATE*/)
				{
					Leap::SwipeGesture swipe = gl[i];
					
					//ͨ��ID�ж��Ƿ��Ǿ�����
					if(lastSwipeID >= swipe.id())
					{
						break;
					}

					//�ж�����swipe��ʱ����
					DWORD t = GetTickCount();
					if(abs(long(t - timeElapse)) < TIME_ELAPSE)
					{
						break;
					}

					timeElapse = t;
					lastSwipeID = swipe.id();

					Leap::Vector diff = 0.004f * (swipe.position() - swipe.startPosition());

					//��������㹻�����������ҷ���
					if(abs(diff.x) > 0.1 && abs(diff.x) > abs(diff.y*0.58) && abs(diff.x) > abs(diff.z*0.58))
					{
						//�ҵ�ppt���Ŵ���
						HWND hPPTWnd = FindWindow(L"screenClass", NULL);

						if(NULL != hPPTWnd)
						{
							//���ҷ�
							if(diff.x > 0)
							{
								PostMessage(hPPTWnd, WM_COMMAND, MAKELONG(393,1), 0);
							}
							else	//����
							{
								PostMessage(hPPTWnd, WM_COMMAND, MAKELONG(394,1), 0);
							}
						}
					}
				}
			}
		} 
	}

	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_LEAPCONTROL));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_LEAPCONTROL);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_LEAPCONTROL));

	return RegisterClassEx(&wcex);
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
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   //hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
   //   CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);
   hWnd = CreateWindowEx(WS_EX_APPWINDOW, szWindowClass, szTitle, WS_POPUP,
	   CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
	   return FALSE;
   }

   ghWnd = hWnd;

   //ShowWindow(hWnd, nCmdShow);
   //UpdateWindow(hWnd);
   ShowWindow(hWnd, SW_HIDE);

   //��������ͼ��
   nid.cbSize = sizeof(NOTIFYICONDATA);
   nid.hWnd = hWnd;
   nid.uID = NID;
   nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP | NIF_INFO;
   nid.uCallbackMessage = WM_TrayNotify;
   nid.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_LEAPCONTROL));
   lstrcpy(nid.szTip, APP_NAME);

   hMenu = CreatePopupMenu();//�������̲˵�  
   AppendMenu(hMenu, MF_STRING, ID_TRAYEXIT, TEXT("�˳�"));

   Shell_NotifyIcon(NIM_ADD, &nid);  

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		Shell_NotifyIcon(NIM_DELETE, &nid);	//ɾ������ͼ��
		PostQuitMessage(0);
		break;
	case WM_TrayNotify:
		switch(lParam)
		{
		//�Ҽ���ʾ�˵�
		case WM_RBUTTONDOWN:
			{
				//��ȡ�������
				POINT pt;
				GetCursorPos(&pt);
				SetForegroundWindow(hWnd);
				
				//��ʾ����ȡѡ�еĲ˵�
				int cmd = TrackPopupMenu(hMenu, TPM_RETURNCMD, pt.x, pt.y, NULL, hWnd, NULL);
				if(cmd == ID_TRAYEXIT)
				{
					PostMessage(hWnd, WM_DESTROY, NULL, NULL);
				}
			}
			break;
		//�����ʾLeapMotion������Ϣ
		case WM_LBUTTONDOWN:
			isLeapmotionConnected();
			break;
		}
		break;

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

//�ж�Leapmotion�Ƿ����ӣ�����������ʾ
void isLeapmotionConnected()
{
	if(controller.isConnected())
	{
		lstrcpy(nid.szInfoTitle, APP_NAME);
		lstrcpy(nid.szInfo, TEXT("LeapMotion�����ӣ�"));
		nid.uTimeout = 200;
		Shell_NotifyIcon(NIM_MODIFY, &nid);
	}
	else
	{
		lstrcpy(nid.szInfoTitle, APP_NAME);
		lstrcpy(nid.szInfo, TEXT("LeapMotionδ���ӣ�"));
		nid.uTimeout = 200;
		Shell_NotifyIcon(NIM_MODIFY, &nid);
	}
}
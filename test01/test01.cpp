// test01.cpp : Defines the entry point for the application.
//
#include "stdafx.h"
#include "test01.h"

#include <windows.h> 
#include <GL/glu.h> 
#include <GL/gl.h> 
#include <GL/glut.h> 
#include <GL/glaux.h> 

HGLRC hRC = NULL; // Rendering Context
HDC hDC   = NULL; // Device Context
HWND hWnd = NULL; // window handle
HINSTANCE hInstance; // Use to save instance of window

bool keys[256]; //save keyword array
bool active = TRUE; //window active flag
bool fullscreen = TRUE; //flag to full screen, default to full screen

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM); //WndProc's definition

// reset OpenGl scene's size
GLvoid ReSizeGLScene(GLsizei width, GLsizei height){

	if(height == 0){
		// avoid remainder is 0
		height = 1;
	}

	glViewport(0, 0, width, height);// reset current viewport

	glMatrixMode(GL_PROJECTION);// selection projection matrix
	glLoadIdentity(); //reset projection matrix

	gluPerspective(45.0f, (GLfloat)width/(GLfloat)height, 0.1f, 100.0f); //reset viewport's size
	glMatrixMode(GL_MODELVIEW); //selection mode observation matrix
	glLoadIdentity(); //reset mode observation matrix
}

int InitGL(GLvoid){

	//open smooth shading
	glShadeModel(GL_SMOOTH);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f); //clear screen's color, set black background
	glClearDepth(1.0f); //set depth cache
	glEnable(GL_DEPTH_TEST); //open depth test
	glDepthFunc(GL_LEQUAL); //depth test's type

	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);//tell system to modify perspective
	
	return (TRUE);
}

// include all draw code
int DrawGLScene(GLvoid){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //clear screen and depth buffer
	glLoadIdentity(); //reset current's modern observation matrix

	return (TRUE);
}

//be called before entire program exit
GLvoid KillGLWindow(GLvoid) {
	if(fullscreen) {
		// is fullscreen mode
		ChangeDisplaySettings(NULL, 0); // yes, change to desktop
		ShowCursor(TRUE); //show mouse pointer
	}

	if (hRC) {
		// if we have OpenGL render description ??
		if (!wglMakeCurrent(NULL, NULL)) {
			// if we can release DC and RC description table
			MessageBox(NULL, _T("release DC or RC failure!"), _T("close error"), MB_OK | MB_ICONINFORMATION);
		}

		// delete color description table
		if (!wglDeleteContext(hRC)) {
			MessageBox(NULL, _T("release RC failure!"), _T("close error"), MB_OK | MB_ICONINFORMATION);
		}
	}

	if( hDC && !ReleaseDC(hWnd, hDC)) {
		MessageBox(NULL, _T("release DC failure!"), _T("close error"), MB_OK | MB_ICONINFORMATION);
		hDC = NULL;
	}
	
	if (hWnd && !DestroyWindow(hWnd)) {
		// if we can destory window
		MessageBox(NULL, _T("release window handler failure!"), _T("close error"), MB_OK | MB_ICONINFORMATION);
		hWnd = NULL;
	}

	// At last, we register our window class, 
	// or we open new window, we will receive message "Windows Class already registered"
	if (!UnregisterClass(_T("OpenGL"), hInstance)) {
		MessageBox(NULL, _T("cannot register window class!"), _T("close error"), MB_OK | MB_ICONINFORMATION);
		hInstance = NULL;
	}
}

BOOL CreateGLWindow(char *title, int width, int height, int bits, bool fullscreenflag) {

	GLuint pixelFormat;
	
	WNDCLASS wc; // window class structure

	DWORD dwExStyle;
	DWORD dwStyle;

	RECT windowRect;
	windowRect.left   = (long)0;
	windowRect.right  = (long)width;
	windowRect.top    = (long)0;
	windowRect.bottom = (long)height;

	fullscreen = fullscreenflag;

	hInstance = GetModuleHandle(NULL); //get our window instance
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = (WNDPROC)WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance  = hInstance;
	wc.hIcon      = LoadIcon(NULL, IDI_WINLOGO);
	wc.hCursor    = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = NULL;
	wc.lpszClassName = _T("OpenGL");
	wc.lpszMenuName  = NULL;

	if (!RegisterClass(&wc)) {
		MessageBox(NULL, _T("register window class failure!"), _T("error"), MB_OK | MB_ICONINFORMATION);
		return (FALSE);
	}

	if(fullscreen) {
		DEVMODE dmScreenSettings;

		memset(&dmScreenSettings, 0, sizeof(DEVMODE));
		dmScreenSettings.dmSize = sizeof(DEVMODE);
		dmScreenSettings.dmPelsWidth  = width;
		dmScreenSettings.dmPelsHeight = height;
		dmScreenSettings.dmBitsPerPel = bits;
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL) {
			if (MessageBox(NULL, _T("full screen mode setting failure!, \nusing window mode?"), _T("NeHe G"), MB_YESNO | MB_ICONEXCLAMATION) == IDYES) {
				fullscreen = FALSE;
			} else {
				MessageBox(NULL, _T("program close failure!"), _T("error"), MB_OK | MB_ICONSTOP);
				return (FALSE);
			}
		}
	}

	if (fullscreen) {
		dwExStyle = WS_EX_APPWINDOW;
		dwStyle   = WS_POPUP;
		ShowCursor(FALSE);
	} else {
		dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
		dwStyle   = WS_OVERLAPPEDWINDOW;
	}

	AdjustWindowRectEx(&windowRect, dwStyle, FALSE, dwExStyle); //adjust window size to destination size

	CString cstrTitle(title);
	if (!(hWnd = CreateWindowEx( dwExStyle,
		_T("OpenGl"),
		cstrTitle,
		WS_CLIPSIBLINGS | WS_CLIPCHILDREN | dwStyle,
		0, 
		0,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		NULL,
		NULL,
		hInstance,
		NULL
		))) {
			KillGLWindow();
			MessageBox(NULL,  _T("cannot create window device description table!"), _T("error"), MB_OK | MB_ICONEXCLAMATION);
			return (FALSE);

	}

	static	PIXELFORMATDESCRIPTOR pfd =	{	// /pfd ���ߴ���������ϣ���Ķ�����������ʹ�õ����ظ�ʽ
		sizeof(PIXELFORMATDESCRIPTOR),					// ������ʽ�������Ĵ�С
		1,								// �汾��
		PFD_DRAW_TO_WINDOW |						// ��ʽ֧�ִ���
		PFD_SUPPORT_OPENGL |						// ��ʽ����֧��OpenGL
		PFD_DOUBLEBUFFER,						// ����֧��˫����
		PFD_TYPE_RGBA,							// ���� RGBA ��ʽ
		bits,								// ѡ��ɫ�����
		0, 0, 0, 0, 0, 0,						// ���Ե�ɫ��λ
		0,								// ��Alpha����
		0,								// ����Shift Bit
		0,								// ���ۼӻ���
		0, 0, 0, 0,							// ���Ծۼ�λ
		16,								// 16λ Z-���� (��Ȼ���)
		0,								// ���ɰ建��
		0,								// �޸�������
		PFD_MAIN_PLANE,							// ����ͼ��
		0,								// Reserved
		0, 0, 0								// ���Բ�����
	};

	if (!(hDC=GetDC(hWnd)))	{					// ȡ���豸��������ô?
		KillGLWindow();							// ������ʾ��
		MessageBox(NULL,_T("���ܴ���һ����ƥ������ظ�ʽ"), _T("����"), MB_OK|MB_ICONEXCLAMATION);
		return FALSE;							// ���� FALSE
	}

	if (!(pixelFormat = ChoosePixelFormat(hDC, &pfd))) {				// Windows �ҵ���Ӧ�����ظ�ʽ����?
		KillGLWindow();							// ������ʾ��
		MessageBox(NULL,_T("�����������ظ�ʽ"), _T("����"), MB_OK|MB_ICONEXCLAMATION);
		return FALSE;							// ���� FALSE
	}

	if(!SetPixelFormat(hDC, pixelFormat,&pfd)) {			// �ܹ��������ظ�ʽô?
		KillGLWindow();							// ������ʾ��
		MessageBox(NULL,_T("�����������ظ�ʽ"), _T("����"), MB_OK|MB_ICONEXCLAMATION);
		return FALSE;							// ���� FALSE
	}

	if (!(hRC=wglCreateContext(hDC))) {					// �ܷ�ȡ����ɫ������?
		KillGLWindow();							// ������ʾ��
		MessageBox(NULL, _T("���ܴ���OpenGL��Ⱦ������"), _T("����"), MB_OK|MB_ICONEXCLAMATION);
		return FALSE;							// ���� FALSE
	}

	if(!wglMakeCurrent(hDC,hRC)) {						// ���Լ�����ɫ������
		KillGLWindow();							// ������ʾ��
		MessageBox(NULL, _T("���ܼ��ǰ��OpenGL��Ȼ������"), _T("����"), MB_OK|MB_ICONEXCLAMATION);
		return FALSE;							// ���� FALSE
	}

	ShowWindow(hWnd,SW_SHOW);						// ��ʾ����
	SetForegroundWindow(hWnd);						// ����������ȼ�
	SetFocus(hWnd);								// ���ü��̵Ľ������˴���
	ReSizeGLScene(width, height);						// ����͸�� GL ��Ļ

	if (!InitGL()) {							// ��ʼ���½���GL����
		KillGLWindow();							// ������ʾ��
		MessageBox(NULL,_T("Initialization Failed."), _T("ERROR"), MB_OK|MB_ICONEXCLAMATION);
		return FALSE;							// ���� FALSE
	}

	return TRUE;
}


LRESULT CALLBACK WndProc(	HWND	hWnd,					// ���ڵľ��
						 UINT	uMsg,					// ���ڵ���Ϣ
						 WPARAM	wParam,					// ���ӵ���Ϣ����
						 LPARAM	lParam)					// ���ӵ���Ϣ����
{
	switch (uMsg)								// ���Windows��Ϣ
	{
	case WM_ACTIVATE:						// ���Ӵ��ڼ�����Ϣ
		{
			if (!HIWORD(wParam))					// �����С��״̬
			{
				active=TRUE;					// �����ڼ���״̬
			}
			else
			{
				active=FALSE;					// �����ټ���
			}

			return 0;						// ������Ϣѭ��
		}
	case WM_SYSCOMMAND:						// ϵͳ�ж�����
		{
			switch (wParam)						// ���ϵͳ����
			{
			case SC_SCREENSAVE:				// ����Ҫ����?
			case SC_MONITORPOWER:				// ��ʾ��Ҫ����ڵ�ģʽ?
				return 0;					// ��ֹ����
			}

			break;							// �˳�
		}
	case WM_CLOSE:							// �յ�Close��Ϣ?
		{
			PostQuitMessage(0);					// �����˳���Ϣ
			return 0;						// ����
		}
	case WM_KEYDOWN:						// �м�����ô?
		{
			keys[wParam] = TRUE;					// ����ǣ���ΪTRUE
			return 0;						// ����
		}
	case WM_KEYUP:							// �м��ſ�ô?
		{
			keys[wParam] = FALSE;					// ����ǣ���ΪFALSE
			return 0;						// ����
		}
	case WM_SIZE:							// ����OpenGL���ڴ�С
		{
			ReSizeGLScene(LOWORD(lParam),HIWORD(lParam));		// LoWord=Width,HiWord=Height
			return 0;						// ����
		}
	}
	// �� DefWindowProc��������δ�������Ϣ��
	return DefWindowProc(hWnd,uMsg,wParam,lParam);
}

int WINAPI WinMain(	HINSTANCE	hInstance,				// ��ǰ����ʵ��
				   HINSTANCE	hPrevInstance,				// ǰһ������ʵ��
				   LPSTR		lpCmdLine,				// �����в���
				   int		nCmdShow)				// ������ʾ״̬
{
	MSG	msg;								// Windowsx��Ϣ�ṹ
	BOOL	done=FALSE;							// �����˳�ѭ����Bool ����
	// ��ʾ�û�ѡ������ģʽ
	if (MessageBox(NULL, _T("������ȫ��ģʽ������ô��"), _T("����ȫ��ģʽ"), MB_YESNO|MB_ICONQUESTION)==IDNO) {
		fullscreen=FALSE;						// FALSEΪ����ģʽ
	}

	// ����OpenGL����
	if (!CreateGLWindow("NeHe's OpenGL������",640,480,16,fullscreen)) {
		return 0;							// ʧ���˳�
	}

	while(!done) {								// ����ѭ��ֱ�� done=TRUE
		if (PeekMessage(&msg,NULL,0,0,PM_REMOVE)) {			// ����Ϣ�ڵȴ���?
			if (msg.message==WM_QUIT) {				// �յ��˳���Ϣ?
				done=TRUE;					// �ǣ���done=TRUE
			} else {							// ���ǣ���������Ϣ
				TranslateMessage(&msg);				// ������Ϣ
				DispatchMessage(&msg);				// ������Ϣ
			}

		}  else {								// ���û����Ϣ
			// ���Ƴ���������ESC��������DrawGLScene()���˳���Ϣ

			if (active)	{					// ���򼤻��ô?
				if (keys[VK_ESCAPE]) {				// ESC ������ô?
					done=TRUE;				// ESC �����˳��ź�
				} else {						// �����˳���ʱ��ˢ����Ļ
					DrawGLScene();				// ���Ƴ���
					SwapBuffers(hDC);			// �������� (˫����)
				}
			}

			if (keys[VK_F1])					// F1��������ô?
			{
				keys[VK_F1]=FALSE;				// ���ǣ�ʹ��Ӧ��Key�����е�ֵΪ FALSE
				KillGLWindow();					// ���ٵ�ǰ�Ĵ���
				fullscreen=!fullscreen;				// �л� ȫ�� / ���� ģʽ
				// �ؽ� OpenGL ����
				if (!CreateGLWindow("NeHe's OpenGL ������",640,480,16,fullscreen))
				{
					return 0;				// �������δ�ܴ����������˳�
				}
			}
		}
	}
	// �رճ���
	KillGLWindow();	

	// ���ٴ���
	return (msg.wParam);							// �˳�����
}


//void background(void) 
//{ 
//	//���ñ�����ɫΪ��ɫ 
//	glClearColor(0.0,0.0,0.0,0.0); 
//} 
//
//void myDisplay(void) 
//{ 
//	//buffer����Ϊ��ɫ��д 
//	glClear(GL_COLOR_BUFFER_BIT); 
//
//	//��ʼ�������� 
//	glBegin(GL_TRIANGLES); 
//
//	//����Ϊ�⻬����ģʽ 
//	glShadeModel(GL_SMOOTH); 
//
//	//���õ�һ������Ϊ��ɫ 
//	glColor3f(1.0,0.0,0.0); 
//
//	//���õ�һ�����������Ϊ��-1.0��-1.0�� 
//	glVertex2f(-1.0,-1.0); 
//
//	//���õڶ�������Ϊ��ɫ 
//	glColor3f(0.0,1.0,0.0); 
//
//	//���õڶ������������Ϊ��0.0��-1.0�� 
//	glVertex2f(0.0,-1.0); 
//
//	//���õ���������Ϊ��ɫ 
//	glColor3f(0.0,0.0,1.0); 
//
//	//���õ��������������Ϊ��-0.5��1.0�� 
//	glVertex2f(-0.5,1.0); 
//
//	//�����ν��� 
//	glEnd(); 
//
//	//ǿ��OpenGL����������ʱ�������� 
//	glFlush(); 
//} 
//
//  
//
//void myReshape(GLsizei w,GLsizei h) 
//{ 
//	glViewport(0,0,w,h); 
//
//	//�����ӿ� 
//	glMatrixMode(GL_PROJECTION); 
//	//ָ����ǰ����ΪGL_PROJECTION 
//	glLoadIdentity(); 
//
//	//����ǰ�����û�Ϊ��λ�� 
//
//	if(w <= h) 
//		gluOrtho2D(-1.0,1.5,-1.5,1.5*(GLfloat)h/(GLfloat)w); 
//	//�����ά����ͶӰ���� 
//	else 
//		gluOrtho2D(-1.0,1.5*(GLfloat)w/(GLfloat)h,-1.5,1.5); 
//	glMatrixMode(GL_MODELVIEW); 
//
//	//ָ����ǰ����ΪGL_MODELVIEW 
//
//} 
//
//int main(int argc, char* argv[]) 
//{ 
//	// ��ʼ�� 
//	glutInit(&argc,argv); 
//	glutInitDisplayMode(GLUT_SINGLE|GLUT_RGB); 
//	glutInitWindowSize(400,400); 
//	glutInitWindowPosition(200,200); 
//
//	//�������� 
//	glutCreateWindow("Triangle"); 
//
//	//��������ʾ 
//	background(); 
//	glutReshapeFunc(myReshape); 
//	glutDisplayFunc(myDisplay); 
//	glutMainLoop(); 
//
//	return(0); 
//}
//
//
//
////
////
////#define MAX_LOADSTRING 100
////
////// Global Variables:
////HINSTANCE hInst;								// current instance
////TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
////TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
////
////// Forward declarations of functions included in this code module:
////ATOM				MyRegisterClass(HINSTANCE hInstance);
////BOOL				InitInstance(HINSTANCE, int);
////LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
////INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
////
////int APIENTRY _tWinMain(HINSTANCE hInstance,
////                     HINSTANCE hPrevInstance,
////                     LPTSTR    lpCmdLine,
////                     int       nCmdShow)
////{
////	UNREFERENCED_PARAMETER(hPrevInstance);
////	UNREFERENCED_PARAMETER(lpCmdLine);
////
//// 	// TODO: Place code here.
////	MSG msg;
////	HACCEL hAccelTable;
////
//	// Initialize global strings
//	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
//	LoadString(hInstance, IDC_TEST01, szWindowClass, MAX_LOADSTRING);
//	MyRegisterClass(hInstance);
//
//	// Perform application initialization:
//	if (!InitInstance (hInstance, nCmdShow))
//	{
//		return FALSE;
//	}
//
//	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_TEST01));
//
//	// Main message loop:
//	while (GetMessage(&msg, NULL, 0, 0))
//	{
//		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
//		{
//			TranslateMessage(&msg);
//			DispatchMessage(&msg);
//		}
//	}
//
//	return (int) msg.wParam;
//}
//
//
//
////
////  FUNCTION: MyRegisterClass()
////
////  PURPOSE: Registers the window class.
////
////  COMMENTS:
////
////    This function and its usage are only necessary if you want this code
////    to be compatible with Win32 systems prior to the 'RegisterClassEx'
////    function that was added to Windows 95. It is important to call this function
////    so that the application will get 'well formed' small icons associated
////    with it.
////
//ATOM MyRegisterClass(HINSTANCE hInstance)
//{
//	WNDCLASSEX wcex;
//
//	wcex.cbSize = sizeof(WNDCLASSEX);
//
//	wcex.style			= CS_HREDRAW | CS_VREDRAW;
//	wcex.lpfnWndProc	= WndProc;
//	wcex.cbClsExtra		= 0;
//	wcex.cbWndExtra		= 0;
//	wcex.hInstance		= hInstance;
//	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TEST01));
//	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
//	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
//	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_TEST01);
//	wcex.lpszClassName	= szWindowClass;
//	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
//
//	return RegisterClassEx(&wcex);
//}
//
////
////   FUNCTION: InitInstance(HINSTANCE, int)
////
////   PURPOSE: Saves instance handle and creates main window
////
////   COMMENTS:
////
////        In this function, we save the instance handle in a global variable and
////        create and display the main program window.
////
//BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
//{
//   HWND hWnd;
//
//   hInst = hInstance; // Store instance handle in our global variable
//
//   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
//      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);
//
//   if (!hWnd)
//   {
//      return FALSE;
//   }
//
//   ShowWindow(hWnd, nCmdShow);
//   UpdateWindow(hWnd);
//
//   return TRUE;
//}
//
////
////  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
////
////  PURPOSE:  Processes messages for the main window.
////
////  WM_COMMAND	- process the application menu
////  WM_PAINT	- Paint the main window
////  WM_DESTROY	- post a quit message and return
////
////
//LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
//{
//	int wmId, wmEvent;
//	PAINTSTRUCT ps;
//	HDC hdc;
//
//	switch (message)
//	{
//	case WM_COMMAND:
//		wmId    = LOWORD(wParam);
//		wmEvent = HIWORD(wParam);
//		// Parse the menu selections:
//		switch (wmId)
//		{
//		case IDM_ABOUT:
//			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
//			break;
//		case IDM_EXIT:
//			DestroyWindow(hWnd);
//			break;
//		default:
//			return DefWindowProc(hWnd, message, wParam, lParam);
//		}
//		break;
//	case WM_PAINT:
//		hdc = BeginPaint(hWnd, &ps);
//		// TODO: Add any drawing code here...
//		EndPaint(hWnd, &ps);
//		break;
//	case WM_DESTROY:
//		PostQuitMessage(0);
//		break;
//	default:
//		return DefWindowProc(hWnd, message, wParam, lParam);
//	}
//	return 0;
//}
//
//// Message handler for about box.
//INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
//{
//	UNREFERENCED_PARAMETER(lParam);
//	switch (message)
//	{
//	case WM_INITDIALOG:
//		return (INT_PTR)TRUE;
//
//	case WM_COMMAND:
//		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
//		{
//			EndDialog(hDlg, LOWORD(wParam));
//			return (INT_PTR)TRUE;
//		}
//		break;
//	}
//	return (INT_PTR)FALSE;
//}

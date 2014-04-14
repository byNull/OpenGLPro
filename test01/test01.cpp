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

	static	PIXELFORMATDESCRIPTOR pfd =	{	// /pfd 告诉窗口我们所希望的东东，即窗口使用的像素格式
		sizeof(PIXELFORMATDESCRIPTOR),					// 上述格式描述符的大小
		1,								// 版本号
		PFD_DRAW_TO_WINDOW |						// 格式支持窗口
		PFD_SUPPORT_OPENGL |						// 格式必须支持OpenGL
		PFD_DOUBLEBUFFER,						// 必须支持双缓冲
		PFD_TYPE_RGBA,							// 申请 RGBA 格式
		bits,								// 选定色彩深度
		0, 0, 0, 0, 0, 0,						// 忽略的色彩位
		0,								// 无Alpha缓存
		0,								// 忽略Shift Bit
		0,								// 无累加缓存
		0, 0, 0, 0,							// 忽略聚集位
		16,								// 16位 Z-缓存 (深度缓存)
		0,								// 无蒙板缓存
		0,								// 无辅助缓存
		PFD_MAIN_PLANE,							// 主绘图层
		0,								// Reserved
		0, 0, 0								// 忽略层遮罩
	};

	if (!(hDC=GetDC(hWnd)))	{					// 取得设备描述表了么?
		KillGLWindow();							// 重置显示区
		MessageBox(NULL,_T("不能创建一种相匹配的像素格式"), _T("错误"), MB_OK|MB_ICONEXCLAMATION);
		return FALSE;							// 返回 FALSE
	}

	if (!(pixelFormat = ChoosePixelFormat(hDC, &pfd))) {				// Windows 找到相应的象素格式了吗?
		KillGLWindow();							// 重置显示区
		MessageBox(NULL,_T("不能设置像素格式"), _T("错误"), MB_OK|MB_ICONEXCLAMATION);
		return FALSE;							// 返回 FALSE
	}

	if(!SetPixelFormat(hDC, pixelFormat,&pfd)) {			// 能够设置象素格式么?
		KillGLWindow();							// 重置显示区
		MessageBox(NULL,_T("不能设置像素格式"), _T("错误"), MB_OK|MB_ICONEXCLAMATION);
		return FALSE;							// 返回 FALSE
	}

	if (!(hRC=wglCreateContext(hDC))) {					// 能否取得着色描述表?
		KillGLWindow();							// 重置显示区
		MessageBox(NULL, _T("不能创建OpenGL渲染描述表"), _T("错误"), MB_OK|MB_ICONEXCLAMATION);
		return FALSE;							// 返回 FALSE
	}

	if(!wglMakeCurrent(hDC,hRC)) {						// 尝试激活着色描述表
		KillGLWindow();							// 重置显示区
		MessageBox(NULL, _T("不能激活当前的OpenGL渲然描述表"), _T("错误"), MB_OK|MB_ICONEXCLAMATION);
		return FALSE;							// 返回 FALSE
	}

	ShowWindow(hWnd,SW_SHOW);						// 显示窗口
	SetForegroundWindow(hWnd);						// 略略提高优先级
	SetFocus(hWnd);								// 设置键盘的焦点至此窗口
	ReSizeGLScene(width, height);						// 设置透视 GL 屏幕

	if (!InitGL()) {							// 初始化新建的GL窗口
		KillGLWindow();							// 重置显示区
		MessageBox(NULL,_T("Initialization Failed."), _T("ERROR"), MB_OK|MB_ICONEXCLAMATION);
		return FALSE;							// 返回 FALSE
	}

	return TRUE;
}


LRESULT CALLBACK WndProc(	HWND	hWnd,					// 窗口的句柄
						 UINT	uMsg,					// 窗口的消息
						 WPARAM	wParam,					// 附加的消息内容
						 LPARAM	lParam)					// 附加的消息内容
{
	switch (uMsg)								// 检查Windows消息
	{
	case WM_ACTIVATE:						// 监视窗口激活消息
		{
			if (!HIWORD(wParam))					// 检查最小化状态
			{
				active=TRUE;					// 程序处于激活状态
			}
			else
			{
				active=FALSE;					// 程序不再激活
			}

			return 0;						// 返回消息循环
		}
	case WM_SYSCOMMAND:						// 系统中断命令
		{
			switch (wParam)						// 检查系统调用
			{
			case SC_SCREENSAVE:				// 屏保要运行?
			case SC_MONITORPOWER:				// 显示器要进入节电模式?
				return 0;					// 阻止发生
			}

			break;							// 退出
		}
	case WM_CLOSE:							// 收到Close消息?
		{
			PostQuitMessage(0);					// 发出退出消息
			return 0;						// 返回
		}
	case WM_KEYDOWN:						// 有键按下么?
		{
			keys[wParam] = TRUE;					// 如果是，设为TRUE
			return 0;						// 返回
		}
	case WM_KEYUP:							// 有键放开么?
		{
			keys[wParam] = FALSE;					// 如果是，设为FALSE
			return 0;						// 返回
		}
	case WM_SIZE:							// 调整OpenGL窗口大小
		{
			ReSizeGLScene(LOWORD(lParam),HIWORD(lParam));		// LoWord=Width,HiWord=Height
			return 0;						// 返回
		}
	}
	// 向 DefWindowProc传递所有未处理的消息。
	return DefWindowProc(hWnd,uMsg,wParam,lParam);
}

int WINAPI WinMain(	HINSTANCE	hInstance,				// 当前窗口实例
				   HINSTANCE	hPrevInstance,				// 前一个窗口实例
				   LPSTR		lpCmdLine,				// 命令行参数
				   int		nCmdShow)				// 窗口显示状态
{
	MSG	msg;								// Windowsx消息结构
	BOOL	done=FALSE;							// 用来退出循环的Bool 变量
	// 提示用户选择运行模式
	if (MessageBox(NULL, _T("你想在全屏模式下运行么？"), _T("设置全屏模式"), MB_YESNO|MB_ICONQUESTION)==IDNO) {
		fullscreen=FALSE;						// FALSE为窗口模式
	}

	// 创建OpenGL窗口
	if (!CreateGLWindow("NeHe's OpenGL程序框架",640,480,16,fullscreen)) {
		return 0;							// 失败退出
	}

	while(!done) {								// 保持循环直到 done=TRUE
		if (PeekMessage(&msg,NULL,0,0,PM_REMOVE)) {			// 有消息在等待吗?
			if (msg.message==WM_QUIT) {				// 收到退出消息?
				done=TRUE;					// 是，则done=TRUE
			} else {							// 不是，处理窗口消息
				TranslateMessage(&msg);				// 翻译消息
				DispatchMessage(&msg);				// 发送消息
			}

		}  else {								// 如果没有消息
			// 绘制场景。监视ESC键和来自DrawGLScene()的退出消息

			if (active)	{					// 程序激活的么?
				if (keys[VK_ESCAPE]) {				// ESC 按下了么?
					done=TRUE;				// ESC 发出退出信号
				} else {						// 不是退出的时候，刷新屏幕
					DrawGLScene();				// 绘制场景
					SwapBuffers(hDC);			// 交换缓存 (双缓存)
				}
			}

			if (keys[VK_F1])					// F1键按下了么?
			{
				keys[VK_F1]=FALSE;				// 若是，使对应的Key数组中的值为 FALSE
				KillGLWindow();					// 销毁当前的窗口
				fullscreen=!fullscreen;				// 切换 全屏 / 窗口 模式
				// 重建 OpenGL 窗口
				if (!CreateGLWindow("NeHe's OpenGL 程序框架",640,480,16,fullscreen))
				{
					return 0;				// 如果窗口未能创建，程序退出
				}
			}
		}
	}
	// 关闭程序
	KillGLWindow();	

	// 销毁窗口
	return (msg.wParam);							// 退出程序
}


//void background(void) 
//{ 
//	//设置背景颜色为黑色 
//	glClearColor(0.0,0.0,0.0,0.0); 
//} 
//
//void myDisplay(void) 
//{ 
//	//buffer设置为颜色可写 
//	glClear(GL_COLOR_BUFFER_BIT); 
//
//	//开始画三角形 
//	glBegin(GL_TRIANGLES); 
//
//	//设置为光滑明暗模式 
//	glShadeModel(GL_SMOOTH); 
//
//	//设置第一个顶点为红色 
//	glColor3f(1.0,0.0,0.0); 
//
//	//设置第一个顶点的坐标为（-1.0，-1.0） 
//	glVertex2f(-1.0,-1.0); 
//
//	//设置第二个顶点为绿色 
//	glColor3f(0.0,1.0,0.0); 
//
//	//设置第二个顶点的坐标为（0.0，-1.0） 
//	glVertex2f(0.0,-1.0); 
//
//	//设置第三个顶点为蓝色 
//	glColor3f(0.0,0.0,1.0); 
//
//	//设置第三个顶点的坐标为（-0.5，1.0） 
//	glVertex2f(-0.5,1.0); 
//
//	//三角形结束 
//	glEnd(); 
//
//	//强制OpenGL函数在有限时间内运行 
//	glFlush(); 
//} 
//
//  
//
//void myReshape(GLsizei w,GLsizei h) 
//{ 
//	glViewport(0,0,w,h); 
//
//	//设置视口 
//	glMatrixMode(GL_PROJECTION); 
//	//指明当前矩阵为GL_PROJECTION 
//	glLoadIdentity(); 
//
//	//将当前矩阵置换为单位阵 
//
//	if(w <= h) 
//		gluOrtho2D(-1.0,1.5,-1.5,1.5*(GLfloat)h/(GLfloat)w); 
//	//定义二维正视投影矩阵 
//	else 
//		gluOrtho2D(-1.0,1.5*(GLfloat)w/(GLfloat)h,-1.5,1.5); 
//	glMatrixMode(GL_MODELVIEW); 
//
//	//指明当前矩阵为GL_MODELVIEW 
//
//} 
//
//int main(int argc, char* argv[]) 
//{ 
//	// 初始化 
//	glutInit(&argc,argv); 
//	glutInitDisplayMode(GLUT_SINGLE|GLUT_RGB); 
//	glutInitWindowSize(400,400); 
//	glutInitWindowPosition(200,200); 
//
//	//创建窗口 
//	glutCreateWindow("Triangle"); 
//
//	//绘制与显示 
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

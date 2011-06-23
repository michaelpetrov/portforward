// portfoward.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "portfoward.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
HWND topWindow, status, redirs;

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	Add(HWND, UINT, WPARAM, LPARAM);

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

	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
 
	wVersionRequested = MAKEWORD( 2, 2 );
 
	err = WSAStartup( wVersionRequested, &wsaData );
	if ( err != 0 ) {
		/* Tell the user that we could not find a usable */
		/* WinSock DLL.                                  */
		return 1;
	}

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_PORTFOWARD, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

    INITCOMMONCONTROLSEX init;
    init.dwSize = sizeof(init);
    init.dwICC = ICC_BAR_CLASSES;
    InitCommonControlsEx(&init);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_PORTFOWARD));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
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
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PORTFOWARD));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_PORTFOWARD);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

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

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, 400, 400, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   topWindow = hWnd;

   status = CreateWindowEx(0, STATUSCLASSNAME, L"Stopped.", WS_CHILD, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, hWnd, NULL, hInstance, NULL);
   ShowWindow(status, nCmdShow);

   RECT rs;
   GetWindowRect(status, &rs);

   RECT r;
   GetClientRect(hWnd, &r);
   redirs = CreateWindowEx(0, WC_LISTVIEW, L"Port Redirections", WS_CHILD | LVS_REPORT, 0, 0, r.right - r.left, r.bottom - (rs.bottom - rs.top), hWnd, NULL, hInstance, NULL);
   ListView_SetExtendedListViewStyle(redirs, LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES);
   LVCOLUMN col;
   col.mask = LVCF_TEXT | LVCF_WIDTH;
   col.pszText = L"Local Port";
   col.cx = 100;
   ListView_InsertColumn(redirs, 0, &col);
   col.pszText = L"Destination";
   col.cx = r.right - r.left - 300;
   ListView_InsertColumn(redirs, 1, &col);
   col.mask |= LVCF_FMT;
   col.fmt = LVCFMT_CENTER;
   col.pszText = L"Destination Port";
   col.cx = 100;
   ListView_InsertColumn(redirs, 2, &col);
   col.pszText = L"# Connections";
   col.cx = 100;
   ListView_InsertColumn(redirs, 3, &col);
   ShowWindow(redirs, nCmdShow);

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

int edit = -1;
bool delete_redir(int n);
void close_connections_for_redir(int idx);
int get_total_connections();
void load_redirs();
void save_redirs();

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
	case WM_CLOSE:
		wParam = IDM_EXIT;
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case ID_FILE_LOAD:
			load_redirs();
			break;
		case ID_FILE_SAVE:
			save_redirs();
			break;
		case ID_REDIR_ADD:
			edit = -1;
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ADDBOX), hWnd, Add);
			break;
		case ID_REDIR_CLOSECONNECTIONS:
			{
				for (int i = 0; i < ListView_GetItemCount(redirs); i++)
					if (ListView_GetItemState(redirs, i, LVIS_SELECTED))
						close_connections_for_redir(i);
			}
			break;
		case ID_REDIR_DELETE:
			{
				for (int i = 0; i < ListView_GetItemCount(redirs); )
					if (ListView_GetItemState(redirs, i, LVIS_SELECTED))
						if (!delete_redir(i))
							i++;
					else
						i++;
			}
			break;
		case ID_REDIR_EDIT:
			edit = -1;
			for (int i = 0; i < ListView_GetItemCount(redirs); i++)
				if (ListView_GetItemState(redirs, i, LVIS_SELECTED)) {
					edit = i;
					break;
				}
			if (edit != -1)
				DialogBox(hInst, MAKEINTRESOURCE(IDD_ADDBOX), hWnd, Add);
			break;
		case IDM_EXIT:
			{
				int nconnections = get_total_connections();
				if (nconnections > 0) {
					WCHAR bufW[1024];
					swprintf(bufW, 1024, L"There are %i active connections being redirected, they will all be disconnected.  Do you really want to quit?", nconnections);
					if (MessageBox(hWnd, bufW, L"Warning", MB_ICONHAND|MB_YESNO) == IDNO)
						break;
				}
				DestroyWindow(hWnd);
			}
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_NOTIFY:
		{
			LPNMLISTVIEW lpnmlv = (LPNMLISTVIEW)lParam;
			if (lpnmlv->hdr.code == LVN_ITEMACTIVATE) {
				edit = lpnmlv->iItem;
				DialogBox(hInst, MAKEINTRESOURCE(IDD_ADDBOX), hWnd, Add);
			} else if (lpnmlv->hdr.code == LVN_ITEMCHANGED) {
				EnableMenuItem(GetMenu(hWnd), ID_REDIR_DELETE, ListView_GetSelectedCount(redirs) != 0 ? MF_ENABLED : MF_GRAYED);
				EnableMenuItem(GetMenu(hWnd), ID_REDIR_CLOSECONNECTIONS, ListView_GetSelectedCount(redirs) != 0 ? MF_ENABLED : MF_GRAYED);
				EnableMenuItem(GetMenu(hWnd), ID_REDIR_EDIT, ListView_GetSelectedCount(redirs) == 1 ? MF_ENABLED : MF_GRAYED);
			}
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
		EndPaint(hWnd, &ps);
		break;
	case WM_SIZE:
		{
			RECT r;
			GetClientRect(status, &r);
			SetWindowPos(status, NULL, 0, HIWORD(lParam)-(r.bottom - r.top), LOWORD(lParam), r.bottom - r.top, SWP_NOZORDER);
			SetWindowPos(redirs, NULL, 0, 0, LOWORD(lParam), HIWORD(lParam)-1 - (r.bottom - r.top), SWP_NOMOVE|SWP_NOZORDER);
			ListView_SetColumnWidth(redirs, 1, r.right - r.left - 300);
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

bool setup_redir(int idx, int lport, const char *dest, int dport);

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

bool update_redir(int idx, const char *dest, int dport);

// Message handler for add box.
INT_PTR CALLBACK Add(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		if (edit != -1) {
			SetWindowText(hDlg, L"Edit Port Redirection");
			WCHAR bufW[1024];
			ListView_GetItemText(redirs, edit, 0, bufW, 1024);
			SetDlgItemText(hDlg, IDC_LPORT, bufW);
			EnableWindow(GetDlgItem(hDlg, IDC_LPORT), FALSE);
			ListView_GetItemText(redirs, edit, 1, bufW, 1024);
			SetDlgItemText(hDlg, IDC_DEST, bufW);
			ListView_GetItemText(redirs, edit, 2, bufW, 1024);
			SetDlgItemText(hDlg, IDC_DPORT, bufW);
		}
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK) 
		{
			WCHAR bufW[1024];
			int lport, dport;
			char dest[1024];

			GetDlgItemText(hDlg, IDC_LPORT, bufW, 1024);
			lport = _wtoi(bufW);

			int n;
			if (edit == -1) {
				LVITEM item;
				item.iItem = ListView_GetItemCount(redirs) + 10;
				item.iSubItem = 0;
				item.mask = LVIF_TEXT;
				item.pszText = bufW;
				n = ListView_InsertItem(redirs, &item);
			} else {
				n = edit;
				GetDlgItemTextA(hDlg, IDC_DEST, dest, 1024);
				GetDlgItemText(hDlg, IDC_DPORT, bufW, 1024);
				dport = _wtoi(bufW);
				if (!update_redir(n, dest, dport))
					return (INT_PTR)TRUE;
			}

			GetDlgItemText(hDlg, IDC_DEST, bufW, 1024);
			GetDlgItemTextA(hDlg, IDC_DEST, dest, 1024);
			ListView_SetItemText(redirs, n, 1, bufW);

			GetDlgItemText(hDlg, IDC_DPORT, bufW, 1024);
			ListView_SetItemText(redirs, n, 2, bufW);
			dport = _wtoi(bufW);

			if (edit != -1) {
				EndDialog(hDlg, LOWORD(wParam));
				return (INT_PTR)TRUE;
			}

			ListView_SetItemText(redirs, n, 3, L"0");

			if (!setup_redir(n, lport, dest, dport)) {
				ListView_DeleteItem(redirs, n);
				return (INT_PTR)TRUE;
			}

			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		else if (LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

struct redir_info_t;

typedef struct {
	redir_info_t *info;
	const char *dest_host;
	int dport;
	SOCKET n, d;
	bool stop, stopped;
	bool writer_exited;
} conn_info;

typedef struct redir_info_t {
	int idx;
	int lport;
	const char *dest_host;
	unsigned int dest;  // yeah, ipv4 only, sue me
	int dport;
	SOCKET s;
	std::set<conn_info*> connections;
} redir_info;
std::map<int, redir_info*> redir_infos;

DWORD WINAPI accepter(LPVOID lpThreadParameter);
unsigned int dnslookup(const char *dest);

bool bindToPort(SOCKET s, int port)
{
    sockaddr_in sin;
    sin.sin_addr.S_un.S_addr = INADDR_ANY;
    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);
    if (bind(s, (sockaddr*)&sin, sizeof(sin)) != 0) {
        MessageBox(NULL, L"Cannot bind to the specified local port.  Do you already have a redirection on this port?  If not, another process is already bound to this port.", L"Error", MB_OK);
        return false;
    }
	return true;
}

bool setup_redir(int idx, int lport, const char *dest, int dport)
{
	unsigned int dest_ip = dnslookup(dest);
	if (dest_ip == 0)
		return false;

	redir_info *info = new redir_info;
	info->idx = idx;
	info->lport = lport;
	info->dest_host = strdup(dest);
	info->dest = dest_ip;
	info->dport = dport;

	info->s = socket(AF_INET, SOCK_STREAM, 0);

	if (!bindToPort(info->s, info->lport)) {
		delete info;
		return false;
	}

    listen(info->s, 5);
	
	redir_infos[idx] = info;

	DWORD id;
	CreateThread(NULL, 0, accepter, (LPVOID)idx, 0, &id);
	return true;		
}

void updateNumConnections(int idx);

int get_total_connections()
{
	int nconnections = 0;
	for (std::map<int, redir_info*>::iterator it = redir_infos.begin(); it != redir_infos.end(); it++)
		nconnections += it->second->connections.size();
	return nconnections;
}

void load_redirs()
{
	char filename[MAX_PATH];
	OPENFILENAMEA of;
	of.lStructSize = sizeof(of);
	of.hwndOwner = topWindow;
	of.hInstance = hInst;
	of.lpstrFilter = "*.txt";
	of.lpstrCustomFilter = NULL;
	of.nMaxCustFilter = 0;
	of.nFilterIndex = 1;
	filename[0] = 0;
	of.lpstrFile = filename;
	of.nMaxFile = sizeof(filename);
	of.lpstrFileTitle = NULL;
	of.nMaxFileTitle = 0;
	of.lpstrInitialDir = NULL;
	of.lpstrTitle = "Load redirections from..";
	of.Flags = 0;
	of.lpstrDefExt = "txt";
	of.pvReserved = 0;
	of.dwReserved = 0;
	of.FlagsEx = 0;
	if (!GetOpenFileNameA(&of))
		return;

	FILE *f = fopen(filename, "r");
	if (f == NULL) {
		MessageBox(topWindow, L"Cannot open specified file", L"Error", MB_OK);
		return;
	}

	char buf[1024];
	while (fgets(buf, 1024, f)) {
		int lport, dport;
		char dest[1024];
		if (sscanf(buf, "%i %s %i", &lport, dest, &dport) == 3) {
			WCHAR bufW[1024];

			swprintf(bufW, 1024, L"%i", lport);
			LVITEM item;
			item.iItem = ListView_GetItemCount(redirs) + 10;
			item.iSubItem = 0;
			item.mask = LVIF_TEXT;
			item.pszText = bufW;
			int n = ListView_InsertItem(redirs, &item);

			MultiByteToWideChar(CP_ACP, 0, dest, -1, bufW, 1024);
			ListView_SetItemText(redirs, n, 1, bufW);

			swprintf(bufW, 1024, L"%i", dport);
			ListView_SetItemText(redirs, n, 2, bufW);
			ListView_SetItemText(redirs, n, 3, L"0");

			if (!setup_redir(n, lport, dest, dport)) {
				ListView_DeleteItem(redirs, n);
			}
		}
	}

	fclose(f);
}

void save_redirs()
{
	char filename[MAX_PATH];
	OPENFILENAMEA of;
	of.lStructSize = sizeof(of);
	of.hwndOwner = topWindow;
	of.hInstance = hInst;
	of.lpstrFilter = "*.txt";
	of.lpstrCustomFilter = NULL;
	of.nMaxCustFilter = 0;
	of.nFilterIndex = 1;
	filename[0] = 0;
	of.lpstrFile = filename;
	of.nMaxFile = sizeof(filename);
	of.lpstrFileTitle = NULL;
	of.nMaxFileTitle = 0;
	of.lpstrInitialDir = NULL;
	of.lpstrTitle = "Save redirections to..";
	of.Flags = 0;
	of.lpstrDefExt = "txt";
	of.pvReserved = 0;
	of.dwReserved = 0;
	of.FlagsEx = 0;
	if (!GetSaveFileNameA(&of))
		return;

	FILE *f = fopen(filename, "w");
	if (f == NULL) {
		MessageBox(topWindow, L"Cannot create save file", L"Error", MB_OK);
		return;
	}
	for (std::map<int, redir_info*>::iterator it = redir_infos.begin(); it != redir_infos.end(); it++) {
		redir_info *info = it->second;
		
		char buf[1024];
		sprintf(buf, "%i %s %i", info->lport, info->dest_host, info->dport);
		fputs(buf, f);
	}
	fclose(f);
}

void close_connections_for_redir(int idx)
{
	redir_info *info = redir_infos[idx];

	// close the ones that are open
	for (std::set<conn_info*>::iterator it = info->connections.begin(); it != info->connections.end(); it++) {
		conn_info *ci = *it;
		ci->stop = true;
		while (!ci->stopped) {
			Sleep(10);
			// pay attention!
			closesocket(ci->d);
			closesocket(ci->n);
		}
		delete ci;
	}

	info->connections.clear();
	updateNumConnections(idx);
}

bool delete_redir(int n)
{
	redir_info *info = redir_infos[n];
	if (info->connections.size()) {
		char buf[1024];
		sprintf(buf, "There are %d connections currently being redirected from %d to %s:%d.\nThese connections will NOT be disconnected (at least not now).\nAre you sure you want to delete this redirection?", 
			info->connections.size(), info->lport, info->dest_host, info->dport);
		if (MessageBoxA(topWindow, buf, "Warning", MB_YESNO) == IDNO)
			return false;

		// dangle these connections
		for (std::set<conn_info*>::iterator it = info->connections.begin(); it != info->connections.end(); it++)
			(*it)->info = NULL;
	}

	// don't accept any more connections
	closesocket(info->s);

	redir_infos.erase(n);

	delete info;

	int count = ListView_GetItemCount(redirs);
	ListView_DeleteItem(redirs, n);
	if (count - 1 > n) {
		for (int i = n + 1; i < count; i++) {
			redir_infos[i - 1] = redir_infos[i];
			redir_infos[i - 1]->idx = i - 1;
		}
		redir_infos.erase(count - 1);
	}

	return true;
}

bool update_redir(int idx, const char *dest, int dport)
{
	redir_info *info = redir_infos[idx];
	info->dport = dport;
	if (strcmp(info->dest_host, dest)) {
		unsigned int dest_ip = dnslookup(dest);
		if (dest_ip == 0)
			return false;
		info->dest = dest_ip;
		info->dest_host = strdup(dest);
	}
	return true;
}

unsigned int dnslookup(const char *dest)
{
	unsigned int a, b, c, d;
	if (sscanf(dest, "%d.%d.%d.%d", &a, &b, &c, &d) == 4)
		return (a << 24) | (b << 16) | (c << 8) | d;   // network order

	hostent *he = gethostbyname(dest);
	if (he == NULL)
		return 0;
	return *(unsigned int *)he->h_addr_list[0];
}

DWORD WINAPI reader(LPVOID lpThreadParameter);
DWORD WINAPI writer(LPVOID lpThreadParameter);

void updateNumConnections(int idx)
{
	WCHAR bufW[100];
	swprintf(bufW, 100, L"%d", redir_infos[idx]->connections.size());
	ListView_SetItemText(redirs, idx, 3, bufW);
}

DWORD WINAPI accepter(LPVOID lpThreadParameter)
{
	redir_info *info = redir_infos[(int)lpThreadParameter];

	sockaddr_in sin;
	int ss = sizeof(sin);
	SOCKET n;
    while ((n = accept(info->s, (sockaddr*)&sin, &ss)) != -1) {
		char buf[1024];
        sprintf(buf, "received connection from %i.%i.%i.%i on port %i\n", sin.sin_addr.S_un.S_un_b.s_b1, sin.sin_addr.S_un.S_un_b.s_b2, sin.sin_addr.S_un.S_un_b.s_b3, sin.sin_addr.S_un.S_un_b.s_b4, info->lport);
		SetWindowTextA(status, buf);
        SOCKET d = socket(AF_INET, SOCK_STREAM, 0);
        sin.sin_family = AF_INET;
        sin.sin_addr.S_un.S_addr = info->dest;
        sin.sin_port = htons(info->dport);
        if (connect(d, (sockaddr*)&sin, sizeof(sin)) != 0) {
            sprintf(buf, "received a connection but can't connect to %s:%i\n", info->dest_host, info->dport);
			SetWindowTextA(status, buf);
            closesocket(n);
        } else {
            sprintf(buf, "connection to %s:%i established\n", info->dest_host, info->dport);
			SetWindowTextA(status, buf);
			conn_info *cinfo = new conn_info;
			cinfo->info = info;
			cinfo->dest_host = info->dest_host;
			cinfo->dport = info->dport;
			cinfo->d = d;
			cinfo->n = n;
			cinfo->stop = false;
			cinfo->stopped = false;
			cinfo->writer_exited = false;
			info->connections.insert(cinfo);
            DWORD id;
            CreateThread(NULL, 0, reader, cinfo, 0, &id);
            CreateThread(NULL, 0, writer, cinfo, 0, &id);

			updateNumConnections(info->idx);
        }
    }
    closesocket(info->s);

	return 0;
}

DWORD WINAPI reader(LPVOID lpThreadParameter)
{
	conn_info *ci = (conn_info*)lpThreadParameter;

    char buf[65536];
    int n;
    while ((n = recv(ci->n, buf, sizeof(buf), 0)) > 0 && !ci->stop) {
        send(ci->d, buf, n, 0);
    }

    closesocket(ci->n);
    closesocket(ci->d);

	while (!ci->writer_exited)
		Sleep(10);

	ci->stopped = true;

	// told to stop
	if (ci->stop)
		return 0;

	// otherwise, cleanup
	if (ci->info) {
		ci->info->connections.erase(ci);
		updateNumConnections(ci->info->idx);
	}

	sprintf(buf, "connection to %s:%i closed\n", ci->dest_host, ci->dport);
	SetWindowTextA(status, buf);

	delete ci;

	return 0;
}

DWORD WINAPI writer(LPVOID lpThreadParameter)
{
	conn_info *ci = (conn_info*)lpThreadParameter;

	char buf[65536];
    int n;
    while ((n = recv(ci->d, buf, sizeof(buf), 0)) > 0 && !ci->stop) {
        send(ci->n, buf, n, 0);
    }

    closesocket(ci->n);
    closesocket(ci->d);

	ci->writer_exited = true;

	return 0;
}

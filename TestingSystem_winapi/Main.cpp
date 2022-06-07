#define _UNICODE
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <windowsX.h>
#include <commctrl.h> 
#include <tchar.h>
#include <ctime>
#include "resource.h"
#include <iostream>
#include <fstream> 
#include <stdio.h>
#pragma comment(lib,"comctl32")

using namespace std;


BOOL CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);

CRITICAL_SECTION cs;
HWND hShowSeconds[30], hProgress1, hedit;
HANDLE Th1, hMutex;  
TCHAR szClassWindow[] = TEXT("Каркасное приложение");
TCHAR buffer[260];
SYSTEMTIME st;
TCHAR buff[260];
TCHAR format[] = { L"Количество правильных ответов %d, оценка %d"}; 
int grate = 0;
int arr[10];
int AllOtvet[30] = {	6,8,4,
					6,3,9,
					45,8,64,
					5,0,3,
					10,8,9,
					99,100,101,
					25,30,35,
					0,1,-1,
					48,58,38,
					6,7,0		};
int Otvet[10] = { 0,0,0,0,
				  0,0,0,0,
				  0,0 };

DWORD WINAPI Thread(LPVOID lp)
{
	HWND hProgress = (HWND)lp;
	while (TRUE)
	{ 
		GetLocalTime(&st);
		wsprintf(buff, _T("%d-%02d-%02d"), st.wYear, st.wMonth, st.wDay);
		SetWindowText(hedit, buff);
	}
	return 0;
}


void MessageAboutError(DWORD dwError)
{
	LPVOID lpMsgBuf = NULL;
	TCHAR szBuf[300];

	BOOL fOK = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
		NULL, dwError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);
	if (lpMsgBuf != NULL)
	{
		wsprintf(szBuf, TEXT("Ошибка %d: %s"), dwError, lpMsgBuf);
		MessageBox(0, szBuf, TEXT("Сообщение об ошибке"), MB_OK | MB_ICONSTOP);
		LocalFree(lpMsgBuf);
	}
}

DWORD WINAPI Write(LPVOID lp)
{
	srand(time(0));
	EnterCriticalSection(&cs);
	ofstream out(TEXT("txt.txt"));
	if (!out.is_open())
	{
		MessageAboutError(GetLastError());
		return 1;
	} 
	for (int i = 0; i < 10; i++)
	{ 
		out << Otvet[i] << ' ';
	}
	out.close();
	LeaveCriticalSection(&cs);
	MessageBox(0, TEXT("Поток записал информацию в файл"), TEXT("Критическая секция"), MB_OK);
	return 0;
}

DWORD WINAPI Read(LPVOID lp)
{
	EnterCriticalSection(&cs);
	ifstream in(TEXT("txt.txt"));
	if (!in.is_open())
	{
		MessageAboutError(GetLastError());
		return 1;
	}
	int B[100];
	int sum = 0;
	for (int i = 0; i < 100; i++)
	{
		in >> B[i];
		sum += B[i];
	}
	in.close();
	LeaveCriticalSection(&cs);
	MessageBox(0, TEXT("Поток прочитал информацию из файла"), TEXT("Критическая секция"), MB_OK);
	TCHAR str[30];
	wsprintf(str, TEXT("Сумма чисел равна %d"), sum);
	MessageBox(0, str, TEXT("Критическая секция"), MB_OK);
	return 0;
}


int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInst, LPTSTR lpszCmdLine, int nCmdShow)
{
	// создаём главное окно приложения на основе модального диалога
	return  DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, (DLGPROC)DlgProc);
}

BOOL CALLBACK DlgProc(HWND hWnd, UINT message, WPARAM wp, LPARAM lp)
{
	switch (message)
	{
	case WM_CLOSE:
		EndDialog(hWnd, 0); // закрываем модальный диалог
		return TRUE;
	case WM_INITDIALOG:
	{
		TCHAR GUID[] = TEXT("text");
		hMutex = CreateMutex(NULL, FALSE, GUID);
		DWORD dwAnswer = WaitForSingleObject(hMutex, 0);
		if (dwAnswer == WAIT_TIMEOUT)
		{
			MessageBox(hWnd, TEXT("Нельзя запускать более одной копии приложения!!!"), TEXT("Мьютекс"), MB_OK | MB_ICONINFORMATION);
			EndDialog(hWnd, 0);
		}


		for (int i = 0; i < 30; i++)
		{
			hShowSeconds[i] = GetDlgItem(hWnd, IDC_CHECK1 + i);
		} 

		InitializeCriticalSection(&cs);

		hedit = GetDlgItem(hWnd, IDC_EDIT1);

		hProgress1 = GetDlgItem(hWnd, IDC_PROGRESS1);
		SendMessage(hProgress1, PBM_SETRANGE, 0, MAKELPARAM(0, 60)); // Установка интервала для индикатора 
		SendMessage(hProgress1, PBM_SETSTEP, 6, 0); // Установка шага прикращения  индикатора 
		SendMessage(hProgress1, PBM_SETPOS, 0, 0); // Установка текущей позиции индикатора
		SendMessage(hProgress1, PBM_SETBKCOLOR, 0, LPARAM(RGB(255, 255, 255))); // Установка цвета фона индикатора
		SendMessage(hProgress1, PBM_SETBARCOLOR, 0, LPARAM(RGB(0, 255, 0))); // Установка цвета заполняемых прямоугольников

		Th1 = CreateThread(0, 0, Thread, hProgress1, 0, 0);

		return TRUE;
	}
	case WM_COMMAND:
		ResumeThread(Th1);
		if (LOWORD(wp) == IDOK)
		{ 
			LRESULT lResult1[10]; 
			lResult1[0] = SendMessage(hShowSeconds[0], BM_GETCHECK, 0, 0);
			lResult1[1] = SendMessage(hShowSeconds[5], BM_GETCHECK, 0, 0);
			lResult1[2] = SendMessage(hShowSeconds[7], BM_GETCHECK, 0, 0);
			lResult1[3] = SendMessage(hShowSeconds[10], BM_GETCHECK, 0, 0);
			lResult1[4] = SendMessage(hShowSeconds[14], BM_GETCHECK, 0, 0);
			lResult1[5] = SendMessage(hShowSeconds[16], BM_GETCHECK, 0, 0);
			lResult1[6] = SendMessage(hShowSeconds[18], BM_GETCHECK, 0, 0);
			lResult1[7] = SendMessage(hShowSeconds[23], BM_GETCHECK, 0, 0);
			lResult1[8] = SendMessage(hShowSeconds[24], BM_GETCHECK, 0, 0);
			lResult1[9] = SendMessage(hShowSeconds[29], BM_GETCHECK, 0, 0);

			int x = 0;
			for (size_t i = 0; i < 30; i++)
			{
				LRESULT lResult = SendMessage(hShowSeconds[i], BM_GETCHECK, 0, 0);
				if (lResult == BST_CHECKED) 
				{
					for (size_t i1 = 0; i1 < 10; i1++)
					{
						if (lResult1[i1] == lResult)
						{
							Otvet[x] = AllOtvet[i];
							x++; grate++;
							SendMessage(hProgress1, PBM_STEPIT, 0, 0); // Изменение текущей позиции индикатора путём прибавления шага 
							break;
						} 
					}
					if (Otvet[x] == 0)
					{
						Otvet[x] = AllOtvet[i];
						x++;
					} 
				}
			}  
			HANDLE hThread = CreateThread(NULL, 0, Write, 0, 0, NULL);
			CloseHandle(hThread);
			hThread = CreateThread(NULL, 0, Read, 0, 0, NULL);
			CloseHandle(hThread); 
		} 
		return TRUE;
	}
	return FALSE; 
}
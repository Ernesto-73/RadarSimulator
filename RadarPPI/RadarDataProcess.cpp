#include "RadarDataProcess.h"
#include "stdafx.h"

DWORD WINAPI RadarDataAccess(LPVOID lpParameter)
{
	ThreadData *thrData = (ThreadData *)lpParameter;
	HWND hwnd = thrData->hwnd;
	SOCKET sock = thrData->sock;

	listen(sock, 5);
	SOCKADDR_IN addrClient;
	int len = sizeof(SOCKADDR);
	while(1)
	{
		SOCKET sockConn = accept(sock, (SOCKADDR *)&addrClient, &len);
	/*
		char sendBuf[100];
		sprintf_s(sendBuf, "Hello, %s. Welcome to javier.net.", inet_ntoa(addrClient.sin_addr));
		send(sockConn, sendBuf, strlen(sendBuf) + 1, 0);
	*/
		char recvBuf[200];

		while(recv(sockConn, recvBuf, 200, 0) > 0)
		{
			CString str(recvBuf);
		//	str.Format("[%s]: %s\n", inet_ntoa(addrClient.sin_addr), recvBuf);
		//	::MessageBox(hwnd, str, NULL, NULL);
			Pos pos;

			CString cx = str.Left(str.Find(","));
			pos.x = _ttoi(cx); 
			str = str.Right(str.Delete(0, str.Find(",") + 1));       
			CString cy = str.Left(str.Find(","));
			pos.y = _ttoi(cy); 
			str = str.Right(str.Delete(0, str.Find(",") + 1));
			CString cz = str;
			pos.z = _ttoi(cz); 

			str = str.Right(str.Delete(0, str.Find(","))); 
			
		//	::MessageBox(hwnd, cx, NULL, NULL);
		//	::SendMessage((HWND)lpParameter, WM_TARGET_UPDATE, NULL, (LPARAM)&pos);
			::SendMessage(hwnd, WM_TARGET_UPDATE, NULL, (LPARAM)&pos);
		}
		closesocket(sockConn);
	}
	return 0;
}
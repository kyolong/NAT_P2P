// NATClient.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include <Winsock2.h>
#include <stdio.h>

int main()
{
	/////////////��ʼ���׽���///////////////////////////////////////////////////
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	wVersionRequested = MAKEWORD(1, 1);//�汾��1.1
									   //1.�����׽��ֿ�	
	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0)
	{
		return -1;
	}///...if
	 //�ж��Ƿ����������winsocket�汾���������
	 //�����WSACleanup��ֹwinsocket��ʹ�ò�����
	if (LOBYTE(wsaData.wVersion) != 1 || HIBYTE(wsaData.wVersion) != 1)
	{
		WSACleanup();
		return -1;
	}///...if
	 ////////////////////////////////////////////////////////////
    return 0;
}


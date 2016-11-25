// NATClient.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <Winsock2.h>
#include <stdio.h>

int main()
{
	/////////////初始化套接字///////////////////////////////////////////////////
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	wVersionRequested = MAKEWORD(1, 1);//版本号1.1
									   //1.加载套接字库	
	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0)
	{
		return -1;
	}///...if
	 //判断是否我们请求的winsocket版本，如果不是
	 //则调用WSACleanup终止winsocket的使用并返回
	if (LOBYTE(wsaData.wVersion) != 1 || HIBYTE(wsaData.wVersion) != 1)
	{
		WSACleanup();
		return -1;
	}///...if
	 ////////////////////////////////////////////////////////////
    return 0;
}


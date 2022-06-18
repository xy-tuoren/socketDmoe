#ifndef _EasyTcpClient_hpp_
#define _EasyTcpClient_hpp_
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
//缓冲区大小
#define RECV_BUFF_SZIE 1024
#include<windows.h>
#include<WinSock2.h>
#pragma comment(lib,"ws2_32.lib")
#else // _WIN32
#include<unistd.h>
#include<arpa/inet.h>
#include <string.h>
#define SOCKET int
#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR           (-1)
#endif
#include <iostream>
#include <thread>
#include <vector>
#include "MessageHeader.hpp"
using namespace std;
class EasyTcpClient
{
public:
	EasyTcpClient()
	{
		_sock = INVALID_SOCKET;
	};
	//虚析构函数
	virtual ~EasyTcpClient()
	{
		Close();
	};
	//初始化socket
	void InitSocket()
	{
#ifdef _WIN32
		//启动windows socket 2.x环境
		WORD ver = MAKEWORD(2, 2);
		WSADATA dat;
		WSAStartup(ver, &dat);
#endif
		if (INVALID_SOCKET != _sock) {
			cout << "socket:" << _sock << "关闭旧连接!" << endl;
			Close();
		}
		//建立一个流socket
		_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == _sock) {
			cout << "socket=" << _sock << "错误" << "建立socket失败!" << endl;
		}
		else
		{
			//cout << "socket=" << _sock << "建立socket成功!" << endl;
		}
	}
	//连接服务器
	int Connect(const char* ip, unsigned short port)
	{
		if (INVALID_SOCKET == _sock) {
			InitSocket();
		}
		//连接服务器 connet
		sockaddr_in sin = {};
		sin.sin_family = AF_INET;
		sin.sin_port = htons(port);
#ifdef _WIN32	
		sin.sin_addr.S_un.S_addr = inet_addr(ip);
#else
		sin.sin_addr.s_addr = inet_addr(ip);
#endif    
		int ret = connect(_sock, (sockaddr*)&sin, sizeof(sin));
		if (SOCKET_ERROR == ret) {
			std::cout << "socket=" << _sock << "连接服务器:" << ip << ":" << port << "失败" << std::endl;
		}
		else
		{
			//std::cout << "socket=" << _sock << "连接服务器:" << ip << ":" << port << "成功" << std::endl;
		}
		return ret;
	}
	//关闭socket
	void Close()
	{
		if (_sock != INVALID_SOCKET)
		{
#ifdef _WIN32   
			//关闭套接字
			closesocket(_sock);
			//清除Windows socket环境
			//WSACleanup();
			//system("pause");
#else
			close(_sock);
#endif  
			cout << "已退出!" << endl;
			_sock = INVALID_SOCKET;
		}
	}

	//处理网络消息
	bool OnRun()
	{
		if (isRun())
		{
			fd_set fdReads;
			FD_ZERO(&fdReads);
			FD_SET(_sock, &fdReads);
			timeval t = { 0,0 };
			int ret = select(_sock + 1, &fdReads, 0, 0, &t);
			if (ret < 0) {
				std::cout << "socket=" << _sock << "select任务结束1" << std::endl;
				Close();
				return false;
			}
			if (FD_ISSET(_sock, &fdReads)) {
				FD_CLR(_sock, &fdReads);
				if (RecvData() < 0) {
					std::cout << "socket=" << _sock << "select任务结束2" << std::endl;
					Close();
					return false;
				};
			}
			return true;
		}
		return false;
	}
	//是否工作中
	bool isRun()
	{
		return _sock != INVALID_SOCKET;
	}
	//接收数据处理粘包拆包
	int RecvData() {
		int nLen = (int)recv(_sock, szRecv, RECV_BUFF_SZIE, 0);
		if (nLen <= 0) {
			std::cout << "socket=" << _sock << "与服务器断开连接,任务结束。" << std::endl;
			return -1;
		}
		//将收到得数据拷贝到消息缓冲区
		memcpy(szMsgBuf + lastPos, szRecv, nLen);
		//消息缓冲区得数据尾部位置后移
		lastPos += nLen;
		//判断消息缓冲区的数据长度大于消息头DateHeader长度
		while(lastPos >= sizeof(DataHeader))
		{
			//这时就可以知道当前消息体的长度
			DataHeader* header = (DataHeader*)szMsgBuf;
			//判断消息缓冲区的数据长度大于消息长度
			if (lastPos >= header->dataLength)
			{
				//消息缓冲区剩余未处理数据的长度
				int nSize = lastPos - header->dataLength;
				//处理网络消息
				onNetMsg(header->cmd);
				//将消息缓冲区剩余未处理数据前移
				memcpy(szMsgBuf, szMsgBuf + header->dataLength, nSize);
				//消息缓冲区的数据尾部位置前移
				lastPos = nSize;
			}
			else
			{
				//消息缓冲区剩余数据不够一条完整消息
				break;
			}
		} 
		return 0;
	};
	//响应
	virtual void onNetMsg(int cmd)
	{
		//处理服务端发送的命令
		switch (cmd)
		{
		case CMD_LOGIN_RESULT: {
			//LoginResult* loginRet = (LoginResult*)szRecv;
			//std::cout << "socket=" << _sock << "收到服务器消息:" << "数据长度:" << loginRet->dataLength << "收到命令:CMD_LOGIN_RESULT" << "内容" << loginRet->result << std::endl;
		}break;
		case CMD_LOGINOUT_RESULT: {
			//LoginOutResult* loginOutRet = (LoginOutResult*)szRecv;
			//std::cout << "socket=" << _sock << "收到服务器消息" << "数据长度:" << loginOutRet->dataLength << "收到命令:CMD_LOGINOUT_RESULT" << "内容" << loginOutRet->result << std::endl;
		}break;
		case CMD_NEW_USER_JOIN: {
			//NewUserJoin* newUserJoin = (NewUserJoin*)szRecv;
			//std::cout << "socket=" << _sock << "收到服务器消息" << "数据长度:" << newUserJoin->dataLength << "收到命令:CMD_NEW_USER_JOIN" << "内容" << newUserJoin->sock << std::endl;
		}break;
		default: {
			std::cout << "socket=" << _sock << "收到服务端错误消息!" << "长度为:" << ((DataHeader*)szRecv)->dataLength << std::endl;
		}break;
		}
	}

	//发送数据
	int SendData(DataHeader* header)
	{
		if (isRun() && header)
		{
			return send(_sock, (const char*)header, header->dataLength, 0);
		}
		return SOCKET_ERROR;
	}

	SOCKET getSocket() 
	{
		return _sock;
	}
private:
	SOCKET _sock;
	//接收缓冲区
	char szRecv[RECV_BUFF_SZIE] = {};
	//第二缓冲区 消息缓冲区
	char szMsgBuf[RECV_BUFF_SZIE * 10] = {};
	int lastPos=0;
};
#endif
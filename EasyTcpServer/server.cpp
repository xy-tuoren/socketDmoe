#include "EasyTcpServer.hpp"

void cmdThread(EasyTcpServer* server) {
    while (true) {
        char cmdBuf[256] = {};
        std::cin >> cmdBuf;
        if (0 == strcmp(cmdBuf, "exit")) {
            std::cout << "服务器socket:" << server->getSocket() << "退出cmdThread线程成功!" << std::endl;
            server->Close();
            break;
        }
        else
        {
            std::cout << "不存在的命令" << std::endl;
        }
    }
}
int main()
{
	EasyTcpServer server;
	server.InitSocket();
	server.Bind(nullptr, 4567);  
	server.Listen(5);
    std::thread t1(cmdThread,&server);
    if (t1.joinable()) 
    {
        t1.detach();
    }
	while (server.isRun())
	{
		server.OnRun();
	}
	server.Close();
	return 0;
}


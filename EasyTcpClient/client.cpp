#include "EasyTcpClient.hpp"

int flag = true;
void cmdThread() {
    while (true) {
        char cmdBuf[256] = {};
        std::cin >> cmdBuf;
        if (0 == strcmp(cmdBuf, "exit")) {
            std::cout << "退出cmdThread线程!" << std::endl;
            flag = false;
            break;
        }
    }
}

int cCount = 1000;
int tCount = 4;
vector<EasyTcpClient*> client(cCount);

void sendThread(int id) 
{
    int begin = cCount / 4 * id;
    int end;
    if (id == (tCount - 1)) {
        end = cCount;
    }
    else
    {
        end = cCount / 4 * (id + 1);
    }
    for (int n = begin; n < end; n++) 
    {
        if (!flag) return;
        client[n] = new EasyTcpClient();
    }
    for (int n = begin; n < end; n++)
    {
        if (!flag) return;
        client[n]->Connect("127.0.0.1", 4567);
        cout << "Connect=" << n+1 << endl;
    }
    Login login;
    strcpy(login.userName, "xy");
    strcpy(login.passWord, "123");
    while (flag) {
        for (auto client : client) {
            client->SendData(&login);
            //client->OnRun();
        }
    };
    for (auto client : client) {
        client->Close();
        delete client;
    }
} 

int main()
{
    //启动发送线程
    for (int n = 0; n < tCount; n++)
    {
        thread t(sendThread,n);
        t.detach();
    }
    //启动Ui线程
    thread t1(cmdThread);
    t1.join();
#ifdef  _WIN32
    WSACleanup();
#endif //  _WIN32
}


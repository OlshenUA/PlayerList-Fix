#include "MyUtil.h"
#include <cwchar>
#define _USE_MATH_DEFINES
#include <cmath>
#include "KMatrix2.h"
#include <complex>
#include <Windows.h>
#include <WinUser.h>
#include <strsafe.h>
#include "KInput.h"
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include "KPacket.h"
#include <map>

#pragma comment(lib, "ws2_32.lib")

double g_drawScale = 1.0;
#define DEFAULT_PORT        5150
#define DEFAULT_BUFFER      2048

int g_iPort = DEFAULT_PORT;
char g_szServer[] = "127.0.0.1";
unsigned int g_myUserKey;
KVector2 g_myUserPos = KVector2::zero;
unsigned int g_userDataStampOld = 0;
unsigned int g_userDataStampNew = 0;

std::map<unsigned int, KPacketUserInfo> g_users;

bool DeleteUser(const unsigned int key)
{
    auto mit = g_users.find(key);
    if (mit == g_users.end())
        return false;
    g_users.erase(key);
    return true;
}

void InsertUser(unsigned int key, KPacketUserInfo packetUserInfo)
{
    g_users.insert(std::make_pair(key, packetUserInfo));
}

void UpdatetUser(unsigned int key, KPacketUserInfo packetUserInfo)
{
    g_users[key] = packetUserInfo;
}


void DrawLine(double x, double y, double x2, double y2, char ch)
{
    KVector2 center{ g_width / 2.0, g_height / 2.0 };
    ScanLine(int(x * g_drawScale + center.x), int(-y * g_drawScale + center.y)
        , int(x2 * g_drawScale + center.x), int(-y2 * g_drawScale + center.y), ch);
}

void DrawLine(KVector2 v0, KVector2 v1, char ch)
{
    KVector2 center{ g_width / 2.0, g_height / 2.0 };
    ScanLine(int(v0.x * g_drawScale + center.x), int(-v0.y * g_drawScale + center.y)
        , int(v1.x * g_drawScale + center.x), int(-v1.y * g_drawScale + center.y), ch);
}

void Update(double elapsedTime)
{
    g_drawScale = 1.0;
    DrawLine(-g_width / 2, 0, g_width / 2, 0, '.');
    DrawLine(0, -g_height / 2, 0, g_height / 2, '.');

    PutTextf(0, 0, "%g", elapsedTime);
    //
    // game object update logic here
    //
    auto it = g_users.begin();
    while (it != g_users.end()) {
        if (it->first != g_myUserKey) {
            PutTextf((int)it->second._xPos, (int)it->second._yPos, "P");
        }
        ++it;
    }
    PutTextf((int)g_myUserPos.x, (int)g_myUserPos.y, "M");
}

void DrawGameWorld() {
    //
    // game object drawing routine here
    //
    DrawBuffer();
}

SOCKET        sClient;

void ReceiverThread()
{
    WSADATA       wsd;
    char          szBuffer[DEFAULT_BUFFER];
    int           ret;
    struct sockaddr_in server;
    struct hostent* host = NULL;

    if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0)
    {
        printf("Failed to load Winsock library! Error %d\n", WSAGetLastError());
        return;
    }
    else
        printf("Winsock library loaded successfully!\n");

    // Create the socket, and attempt to connect to the server
    sClient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sClient == INVALID_SOCKET)
    {
        printf("socket() failed with error code %d\n", WSAGetLastError());
        return;
    }
    else
        printf("socket() looks fine!\n");
    server.sin_family = AF_INET;
    server.sin_port = htons(g_iPort);
    server.sin_addr.s_addr = inet_addr(g_szServer);

    // If the supplied server address wasn't in the form
    // "aaa.bbb.ccc.ddd" it's a hostname, so try to resolve it
    if (server.sin_addr.s_addr == INADDR_NONE)
    {
        host = gethostbyname(g_szServer);
        if (host == NULL)
        {
            printf("Unable to resolve server %s\n", g_szServer);
            return;
        }
        else
            printf("The hostname resolved successfully!\n");

        CopyMemory(&server.sin_addr, host->h_addr_list[0], host->h_length);
    }

    if (connect(sClient, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
    {
        printf("connect() failed with error code %d\n", WSAGetLastError());
        return;
    }
    else
        printf("connect() is pretty damn fine!\n");

    while (true) {
        ret = recv(sClient, szBuffer, DEFAULT_BUFFER, 0);
        if (ret == 0)        // Graceful close
        {
            printf("It is a graceful close!\n");
            break;
        }
        else if (ret == SOCKET_ERROR)
        {
            printf("recv() failed with error code %d\n", WSAGetLastError());
            break;
        }
        //szBuffer[ret] = '\0';

        KPacket packet;
        std::vector<char> buffer;
        buffer.reserve(ret);
        buffer.assign(&szBuffer[0], &szBuffer[ret]);
        BufferToPacket(buffer, packet);
        if (packet.m_usPacketId == EPACKET_ACCEPT_CONNECTION) {
            KPacketAcceptConnection connection;
            BufferToPacket(packet.m_buffer, connection);
            printf("Connection key=%u\n", connection._dwKey);
            g_myUserKey = connection._dwKey;
        }
        else if (packet.m_usPacketId == EPACKET_CREATE_USER) {
            KPacketCreateUser createUser;
            BufferToPacket(packet.m_buffer, createUser);
            printf("Create user: %u\n", createUser._dwKey);
            {
                KPacketUserInfo userInfo;
                userInfo._dwUserKey = createUser._dwKey;
                userInfo._xPos = 0;
                userInfo._yPos = 0;
                InsertUser(createUser._dwKey, userInfo);
            }
        }
        else if (packet.m_usPacketId == EPACKET_DESTROY_USER) {
            KPacketDestroyUser destroyUser;
            BufferToPacket(packet.m_buffer, destroyUser);
            printf("Destroy user: %u\n", destroyUser._dwKey);
            DeleteUser(destroyUser._dwKey);
        }
        else if (packet.m_usPacketId == EPACKET_CHAT) {
            KPacketChat chat;
            BufferToPacket(packet.m_buffer, chat);
            printf("Chat: %s\n", chat._text.c_str());
        }
        else if (packet.m_usPacketId == EPACKET_USER_INFO) {
            KPacketUserInfo userInfo;
            BufferToPacket(packet.m_buffer, userInfo);
            printf("UserInfo: %u: (%i,%i)\n", userInfo._dwUserKey, userInfo._xPos, userInfo._yPos);
            UpdatetUser(userInfo._dwUserKey, userInfo);
        }
        else if (packet.m_usPacketId == EPACKET_USER_LIST) {
            KPacketUserList userList;
            BufferToPacket(packet.m_buffer, userList);

            for (auto& user : userList._vecUsers) {
                printf("UserList: %u: (%i,%i)\n", user._dwUserKey, user._xPos, user._yPos);
                UpdatetUser(user._dwUserKey, user);
            }
        }
        //printf("recv() is OK. Received %d bytes: %s\n", ret, szBuffer);
    }

    if (closesocket(sClient) == 0)
        printf("closesocket() is OK!\n");
    else
        printf("closesocket() failed with error code %d\n", WSAGetLastError());

    if (WSACleanup() == 0)
        printf("WSACleanup() is fine!\n");
    else
        printf("WSACleanup() failed with error code %d\n", WSAGetLastError());
}

void usage()
{
    printf("ClientName: client [-p:x] [-s:IP] [-n:x] [-o]\n\n");
    printf("       -p:x      Remote port to send to\n");
    printf("       -s:IP     Server's IP address or hostname\n");
    printf("\n");
}

void ValidateArgs(int argc, char** argv)
{
    int    i;

    for (i = 1; i < argc; i++) {
        if ((argv[i][0] == '-') || (argv[i][0] == '/')) {
            switch (tolower(argv[i][1])) {
            case 'p':        // Remote port
                if (strlen(argv[i]) > 3)
                    g_iPort = atoi(&argv[i][3]);
                break;
            case 's':       // Server
                if (strlen(argv[i]) > 3)
                    strcpy_s(g_szServer, sizeof(g_szServer), &argv[i][3]);
                break;
            default:
                usage();
                break;
            }
        }
    }
}

int main(int argc, char** argv)
{
    //if (argc < 2) {
    //    usage();
    //    return 0;
    //}

    // Parse the command line and load Winsock
    ValidateArgs(argc, argv);

    std::thread     receiver(ReceiverThread);

    g_hwndConsole = GetConsoleWindow();
    g_hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    ShowCursor(false);

    bool isGameLoop = true;
    clock_t prevClock = clock();
    clock_t currClock = prevClock;
    int i = 1;
    char ch = 0;

    while (isGameLoop == true) {
        if (_kbhit()) {
            ch = _getch();
        }
        //if (Input.GetKeyState(27) == KInput::InputState::PRESSED)
        if (ch == 27)
            isGameLoop = false;

        if (ch == 'a') {
            g_myUserPos.x -= 1;
            g_userDataStampNew += 1;
        }
        if (ch == 'd') {
            g_myUserPos.x += 1;
            g_userDataStampNew += 1;
        }
        if (ch == 'w') {
            g_myUserPos.y -= 1;
            g_userDataStampNew += 1;
        }
        if (ch == 's') {
            g_myUserPos.y += 1;
            g_userDataStampNew += 1;
        }
    
        if (g_userDataStampNew != g_userDataStampOld) {
            g_userDataStampOld = g_userDataStampNew;
            KPacket packet;
            std::vector<char> buffer;
            {
                KPacketUserInfo userInfo;
                userInfo._dwUserKey = g_myUserKey;
                userInfo._xPos = (int)g_myUserPos.x;
                userInfo._yPos = (int)g_myUserPos.y;
                packet.SetData(0, EPACKET_USER_INFO, userInfo);
                PacketToBuffer(packet, buffer);
            }
            int ret = send(sClient, &buffer[0], buffer.size(), 0);

            if (ret == 0) {
                isGameLoop = false;
            }
            else if (ret == SOCKET_ERROR) {
                printf("Send error\r\n");
            }
        }

        ch = 0;
        prevClock = currClock;
        currClock = clock();
        const double elapsedTime = ((double)currClock - (double)prevClock) / CLOCKS_PER_SEC;
        ClearBuffer();
        Input.Update(elapsedTime);
        Update(elapsedTime);
        Sleep(10);
        DrawGameWorld();
    }

    if (closesocket(sClient) == 0)
        printf("Client01: closesocket() is OK!\n");
    else
        printf("Client01: closesocket() failed with error code %d\n", WSAGetLastError());
    receiver.join();
}

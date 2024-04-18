#if defined(WIN32)
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "net.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include "utils-win32.h"

std::string XNet::GetOSUserName()
{
    DWORD dwSize = MAX_PATH;
    WCHAR pszName[MAX_PATH] = { 0 };
    if (!GetUserNameW(pszName, &dwSize))
    {
        GetUserNameW(pszName, &dwSize);
    }
    std::string str = StringWideCharToUtf8(pszName);
    return str;
}

bool XNet::GetLocalIps(std::vector<std::string>& vectIps)
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) 
    {
        XERROR("Failed to initialize winsock");
        return false;
    }
    char hostname[256] = {0};
    if (gethostname(hostname, sizeof(hostname)) != 0) 
    {
        XERROR("Failed to get hostname");
        WSACleanup();
        return false;
    }
    struct hostent* host = gethostbyname(hostname);
    if (host == NULL) 
    {
        XERROR("Failed to get host information");
        WSACleanup();
        return false;
    }
    //XINFO("h_name:%s", host->h_name);
    //for (int i = 0; host->h_aliases[i] != NULL; i++)
    //{
    //    XINFO("h_aliases[i%d]:%s", i, host->h_aliases[i]);
    //}
    //XINFO("h_length : %h", host->h_length);
    struct in_addr** addr_list = (struct in_addr**)host->h_addr_list;
    char tmp[32] = { 0 };
    for (int i = 0; addr_list[i] != NULL; i++) 
    {
        //vectIps.push_back(inet_ntoa(*addr_list[i]));
        memset(tmp, 0, sizeof tmp);
        InetNtop(host->h_addrtype, addr_list[i], tmp, sizeof tmp);
        vectIps.push_back(tmp);
    }
    WSACleanup();
    return true;
}


#endif
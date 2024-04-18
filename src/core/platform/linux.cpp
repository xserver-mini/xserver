/***************************************************************************
 * Copyright (C) 2023-, openlinyou, <linyouhappy@foxmail.com>
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 ***************************************************************************/

#include "net.h"
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <assert.h>
#include <netdb.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>


bool XNet::GetLocalIps(std::vector<std::string>& vectIps)
{
	int sfd = socket(AF_INET, SOCK_DGRAM, 0); 
    if (sfd < 0)
    {
        return false;
    }
    struct ifreq buf[16];
    struct ifconf ifc = {0};
    ifc.ifc_len = sizeof(buf);
    ifc.ifc_buf = (caddr_t)buf;
    if (ioctl(sfd, SIOCGIFCONF, (char *)&ifc))
    {
        return false;
    }
    int intr = ifc.ifc_len / sizeof(struct ifreq);
    const char* ptr;
    while (intr-- > 0)
    {
        ioctl(sfd, SIOCGIFADDR, (char *)&buf[intr]);
        struct sockaddr_in* addr = (struct sockaddr_in*)(&buf[intr].ifr_addr);
        ptr = inet_ntoa(addr-> sin_addr);
		vectIps.push_back(ptr);
    }
    close(sfd);
	return true;
}

#ifndef __NETFUNC_HH__
#define __NETFUNC_HH__
#include <sys/types.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <errno.h>
#include <stdio.h>
#include <string>
using namespace std;

//Ref:http://publib.boulder.ibm.com/infocenter/iseries/v7r1m0/index.jsp?topic=%2Fapis%2Fgetifaddrs.htm
uint32_t getInterfaceAddressV4(char* interfaceName) {
	struct ifaddrs *interfaceArray = NULL, *iter = NULL;
	void *addrPtr = NULL;
	int rc = 0;
  	uint32_t ret = 0;

  	rc = getifaddrs(&interfaceArray);  /* retrieve the current interfaces */
	if (0 == rc) {
    	for(iter = interfaceArray; iter != NULL; iter = iter->ifa_next) {
      		if(iter->ifa_addr->sa_family == AF_INET) {
				addrPtr = &((struct sockaddr_in *)iter->ifa_addr)->sin_addr;
				if (!strcmp(interfaceName, iter->ifa_name)) {
					ret = ((struct in_addr*)addrPtr)->s_addr;
				}
			}
		}
		freeifaddrs(interfaceArray);             /* free the dynamic memory */
		interfaceArray = NULL;                   /* prevent use after free  */
	} else {
		printf("getifaddrs() failed with errno =  %d %s \n",errno, strerror(errno));
	}
	return ret;
}

void printIp(uint32_t ip) {
	printf("%u.%u.%u.%u\n",ip&0xff,(ip>>8)&0xff,(ip>>16)&0xff,(ip>>24)&0xff);
}

string Ipv4Int2Str(uint32_t ip) {
	char buf[INET_ADDRSTRLEN];
	struct in_addr addr;
	addr.s_addr = ip;
	inet_ntop(AF_INET, &addr.s_addr, buf, sizeof(buf));
	return string(buf);
}

#endif

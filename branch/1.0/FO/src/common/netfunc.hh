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

uint32_t getInterfaceAddressV4(char* interfaceName);

void printIp(uint32_t ip);

string Ipv4Int2Str(uint32_t ip);

#endif

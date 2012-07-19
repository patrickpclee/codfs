// Definition of the Socket class

#ifndef __SOCKET_HH__
#define __SOCKET_HH__


#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <string>
#include <arpa/inet.h>


const int MAXHOSTNAME = 200;
const int MAXCONNECTIONS = 5;
const int MAXRECV = 500;

class Socket
{
 public:
  Socket();
  virtual ~Socket();

  // Server initialization
  bool create();
  bool bind ( const int port );
  bool listen() const;
  bool accept ( Socket* ) const;

  // Client initialization
  bool connect ( const std::string host, const int port );

  // Data Transimission
  bool sendString ( const std::string ) const;
  int recvString ( std::string& ) const;
  uint32_t sendn (const char* buf, uint32_t buf_len);
  uint32_t recvn (char* buf, uint32_t buf_len);

  void set_non_blocking ( const bool );

  bool is_valid() const { return m_sock != -1; }

  int getSockfd();

 private:

  int m_sock;
  sockaddr_in m_addr;

};

#endif

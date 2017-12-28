/*
   Copyright (C) Zhang GuoQi <guoqi.zhang@gmail.com>

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

   see https://github.com/chenshuo/muduo/
   see http://software.schmorp.de/pkg/libev.html

   libev was written and designed by Marc Lehmann and Emanuele Giaquinta.
   muduo was written by chenshuo.
*/


#ifndef FLAGWLD_NET_SOCKETSOPS_H
#define FLAGWLD_NET_SOCKETSOPS_H

#include <arpa/inet.h>
#include <sys/un.h>

#include <netinet/tcp.h>

namespace flagwld
{
namespace net
{
namespace sockets
{

#define ECONNSELF    255     /* Connection self*/

//#define FLAGWLD_LISTEN_BACKLOG  511
#define FLAGWLD_LISTEN_BACKLOG  10240

const struct sockaddr* sockaddr_cast(const struct sockaddr_in* addr);
struct sockaddr* sockaddr_cast(struct sockaddr_in* addr);
const struct sockaddr_in* sockaddr_in_cast(const struct sockaddr* addr);
struct sockaddr_in* sockaddr_in_cast(struct sockaddr* addr);

const struct sockaddr* sockaddr_cast(const struct sockaddr_in6* addr);
struct sockaddr* sockaddr_cast(struct sockaddr_in6* addr);
const struct sockaddr_in6* sockaddr_in6_cast(const struct sockaddr* addr);
struct sockaddr_in6* sockaddr_in6_cast(struct sockaddr* addr);

const struct sockaddr* sockaddr_cast(const struct sockaddr_un* addr);
struct sockaddr* sockaddr_cast(struct sockaddr_un* addr);
const struct sockaddr_un* sockaddr_un_cast(const struct sockaddr* addr);
struct sockaddr_un* sockaddr_un_cast(struct sockaddr* addr);

///
/// Creates a non-blocking socket file descriptor,
/// abort if any error.
int createStreamNonblockingOrDie(sa_family_t);

void bindOrDie(int sockfd, const struct sockaddr* addr, socklen_t len);
void listenOrDie(int sockfd);
int  accept(int sockfd, struct sockaddr_un* addr);
int  connect(int sockfd, const struct sockaddr* addr, socklen_t len);

ssize_t read(int sockfd, void *buf, size_t count);
ssize_t readv(int sockfd, const struct iovec *iov, int iovcnt);
ssize_t write(int sockfd, const void *buf, size_t count);
void close(int sockfd);
void shutdownWrite(int sockfd);

void toIp(char* buf, size_t size,
          const struct sockaddr_in& addr);
void toIpPort(char* buf, size_t size,
              const struct sockaddr_in& addr);
void fromIpPort(const char* ip, uint16_t port,
                  struct sockaddr_in* addr);

void toIp(char* buf, size_t size,
          const struct sockaddr_in6& addr);
void toIpPort(char* buf, size_t size,
              const struct sockaddr_in6& addr);
void fromIp6Port(const char* ip, uint16_t port,
                  struct sockaddr_in6* addr);

bool validIp(const char* ip);
bool validIp6(const char* ip);

int getSocketError(int sockfd);

bool getTcpInfo(int sockfd, struct tcp_info* tcpi);
  /// Enable/disable TCP_NODELAY (disable/enable Nagle's algorithm).
void setTcpNoDelay(int sockfd, bool on);
  ///Enable/disable SO_REUSEADDR
void setReuseAddr(int sockfd, bool on);
  ///Enable/disable SO_REUSEPORT
void setReusePort(int sockfd, bool on);
  /// Enable/disable SO_KEEPALIVE
void setKeepAlive(int sockfd, bool on);
  // Set SND buffer
void setSndBuffer(int sockfd, int buf_len);
  // Set RCV buffer
void setRcvBuffer(int sockfd, int buf_len);

struct sockaddr_un getLocalAddr(int sockfd);
struct sockaddr_un getPeerAddr(int sockfd);

bool isSelfConnect(int sockfd);
}
}
}

#endif  // FLAGWLD_NET_SOCKETSOPS_H

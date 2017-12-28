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


#include <flagwld/net/SocketsOps.h>

#include <flagwld/base/Logging.h>
#include <flagwld/base/Types.h>
#include <flagwld/net/Endian.h>

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>  // snprintf
#include <strings.h>  // bzero
#include <sys/socket.h>
#include <sys/uio.h>  // readv
#include <unistd.h>

#include <netinet/in.h>
#include <netinet/tcp.h>

using namespace flagwld;
using namespace flagwld::net;

namespace
{

typedef struct sockaddr SA;

void setNonBlockAndCloseOnExec(int sockfd)
{
  // non-block
  int flags = ::fcntl(sockfd, F_GETFL, 0);
  flags |= O_NONBLOCK;
  int ret = ::fcntl(sockfd, F_SETFL, flags);
  // FIXME check

  // close-on-exec
  flags = ::fcntl(sockfd, F_GETFD, 0);
  flags |= FD_CLOEXEC;
  ret = ::fcntl(sockfd, F_SETFD, flags);
  // FIXME check

  (void)ret;
}

}

const struct sockaddr* sockets::sockaddr_cast(const struct sockaddr_in* addr)
{
  return static_cast<const struct sockaddr*>(implicit_cast<const void*>(addr));
}

struct sockaddr* sockets::sockaddr_cast(struct sockaddr_in* addr)
{
  return static_cast<struct sockaddr*>(implicit_cast<void*>(addr));
}

const struct sockaddr_in* sockets::sockaddr_in_cast(const struct sockaddr* addr)
{
  return static_cast<const struct sockaddr_in*>(implicit_cast<const void*>(addr));
}

struct sockaddr_in* sockets::sockaddr_in_cast(struct sockaddr* addr)
{
  return static_cast<struct sockaddr_in*>(implicit_cast<void*>(addr));
}

const struct sockaddr* sockets::sockaddr_cast(const struct sockaddr_in6* addr)
{
  return static_cast<const struct sockaddr*>(implicit_cast<const void*>(addr));
}

struct sockaddr* sockets::sockaddr_cast(struct sockaddr_in6* addr)
{
  return static_cast<struct sockaddr*>(implicit_cast<void*>(addr));
}

const struct sockaddr_in6* sockets::sockaddr_in6_cast(const struct sockaddr* addr)
{
  return static_cast<const struct sockaddr_in6*>(implicit_cast<const void*>(addr));
}

struct sockaddr_in6* sockets::sockaddr_in6_cast(struct sockaddr* addr)
{
  return static_cast<struct sockaddr_in6*>(implicit_cast<void*>(addr));
}

const struct sockaddr* sockets::sockaddr_cast(const struct sockaddr_un* addr)
{
  return static_cast<const struct sockaddr*>(implicit_cast<const void*>(addr));
}

struct sockaddr* sockets::sockaddr_cast(struct sockaddr_un* addr)
{
  return static_cast<struct sockaddr*>(implicit_cast<void*>(addr));
}

const struct sockaddr_un* sockets::sockaddr_un_cast(const struct sockaddr* addr)
{
  return static_cast<const struct sockaddr_un*>(implicit_cast<const void*>(addr));
}

struct sockaddr_un* sockets::sockaddr_un_cast(struct sockaddr* addr)
{
  return static_cast<struct sockaddr_un*>(implicit_cast<void*>(addr));
}

int sockets::createStreamNonblockingOrDie(sa_family_t family)
{
  // socket
  int sockfd=-1;

  if (family != AF_UNIX)
  {
    sockfd = ::socket(family, SOCK_STREAM, IPPROTO_TCP);
  }
  else
  {
    sockfd = ::socket(AF_UNIX, SOCK_STREAM, 0);
  }

  if (sockfd < 0)
  {
    LOG_SYSFATAL << "sockets::createTcp6NonblockingOrDie";
  }

  setNonBlockAndCloseOnExec(sockfd);
  
  return sockfd;
}

void sockets::bindOrDie(int sockfd, const struct sockaddr* addr, socklen_t len)
{
  int ret = ::bind(sockfd, addr, len);
  if (ret < 0)
  {
    char buf[256] = {0};
    switch (addr->sa_family)
    {
      case AF_INET:
        toIpPort(buf, sizeof(buf), *sockaddr_in_cast(addr));
        break;
      case AF_INET6:
        toIpPort(buf, sizeof(buf), *sockaddr_in6_cast(addr));
        break;
      case AF_UNIX:
        strncpy(buf, sockaddr_un_cast(addr)->sun_path, sizeof(buf));
        break;
      default:
        assert(0);
    }
    LOG_SYSFATAL << "sockets::bindOrDie " << buf;
  }
}

void sockets::listenOrDie(int sockfd)
{
  //int ret = ::listen(sockfd, SOMAXCONN);
  int ret = ::listen(sockfd, FLAGWLD_LISTEN_BACKLOG);
  if (ret < 0)
  {
    LOG_SYSFATAL << "sockets::listenOrDie " << sockfd;
  }
}

int sockets::accept(int sockfd, struct sockaddr_un* addr)
{
  socklen_t addrlen = static_cast<socklen_t>(sizeof *addr);
  int connfd = ::accept(sockfd, sockaddr_cast(addr), &addrlen);
  if (connfd < 0)
  {
    int savedErrno = errno;
    switch (savedErrno)
    {
      case EAGAIN /*EWOULDBLOCK*/:
      case ECONNABORTED:
      case EINTR:
      case EPROTO: // ???
      case EPERM:
      case EMFILE: // per-process lmit of open file desctiptor ???
        // expected errors
        LOG_TRACE << "Socket::accept: " << savedErrno;
        errno = savedErrno;
        break;
      case EBADF:
      case EFAULT:
      case EINVAL:
      case ENFILE:
      case ENOBUFS:
      case ENOMEM:
      case ENOTSOCK:
      case EOPNOTSUPP:
        // unexpected errors
        LOG_FATAL << "unexpected error of ::accept " << savedErrno;
        break;
      default:
        LOG_FATAL << "unknown error of ::accept " << savedErrno;
        break;
    }
  }
  else
  {
    setNonBlockAndCloseOnExec(connfd);
  }
  return connfd;
}

int sockets::connect(int sockfd, const struct sockaddr* addr, socklen_t len)
{
  return ::connect(sockfd, addr, len);
}

ssize_t sockets::read(int sockfd, void *buf, size_t count)
{
  return ::read(sockfd, buf, count);
}

ssize_t sockets::readv(int sockfd, const struct iovec *iov, int iovcnt)
{
  return ::readv(sockfd, iov, iovcnt);
}

ssize_t sockets::write(int sockfd, const void *buf, size_t count)
{
  return ::write(sockfd, buf, count);
}

void sockets::close(int sockfd)
{
  if (::close(sockfd) < 0)
  {
    LOG_SYSERR << "sockets::close fd=" <<  sockfd;
  }
}

void sockets::shutdownWrite(int sockfd)
{
  if (::shutdown(sockfd, SHUT_WR) < 0)
  {
    LOG_SYSERR << "sockets::shutdownWrite " << sockfd;
  }
}

void sockets::toIp(char* buf, size_t size,
                   const struct sockaddr_in& addr)
{
  assert(size >= INET_ADDRSTRLEN);
  ::inet_ntop(AF_INET, &addr.sin_addr, buf, static_cast<socklen_t>(size));
}

void sockets::toIpPort(char* buf, size_t size,
                       const struct sockaddr_in& addr)
{
  assert(size >= INET_ADDRSTRLEN);
  ::inet_ntop(AF_INET, &addr.sin_addr, buf, static_cast<socklen_t>(size));
  size_t end = ::strlen(buf);
  uint16_t port = sockets::networkToHost16(addr.sin_port);
  assert(size > end);
  snprintf(buf+end, size-end, ":%u", port);
}

void sockets::fromIpPort(const char* ip, uint16_t port,
                           struct sockaddr_in* addr)
{
  addr->sin_family = AF_INET;
  addr->sin_port = hostToNetwork16(port);
  if (::inet_pton(AF_INET, ip, &addr->sin_addr) <= 0)
  {
    LOG_SYSERR << "sockets::fromIpPort " << ip << ":" << port;
  }
}

void sockets::toIp(char* buf, size_t size,
          const struct sockaddr_in6& addr)
{
  assert(size >= INET6_ADDRSTRLEN);
  ::inet_ntop(AF_INET6, &addr.sin6_addr, buf, static_cast<socklen_t>(size));
}

void sockets::toIpPort(char* buf, size_t size,
              const struct sockaddr_in6& addr)
{
  toIp(buf, size, addr);
  size_t end = ::strlen(buf);
  uint16_t port = sockets::networkToHost16(addr.sin6_port);
  assert(size > end);
  snprintf(buf+end, size-end, ":%u", port);
}

void sockets::fromIp6Port(const char* ip, uint16_t port,
                  struct sockaddr_in6* addr)
{
  addr->sin6_family = AF_INET6;
  addr->sin6_port = hostToNetwork16(port);
  if (::inet_pton(AF_INET6, ip, &addr->sin6_addr) <= 0)
  {
    LOG_SYSERR << "sockets::fromIpPort";
  }
}

bool sockets::validIp(const char* ip)
{
  struct in_addr inp;
  return (::inet_aton(ip, &inp)>0);
}

bool sockets::validIp6(const char* ip)
{
  struct in6_addr in6p;
  return (::inet_pton(AF_INET6, ip, &in6p)>0);
}

int sockets::getSocketError(int sockfd)
{
  int optval;
  socklen_t optlen = static_cast<socklen_t>(sizeof optval);

  if (::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0)
  {
    return errno;
  }
  else
  {
    return optval;
  }
}

bool sockets::getTcpInfo(int sockfd, struct tcp_info* tcpi)
{
  socklen_t len = sizeof(*tcpi);
  bzero(tcpi, len);
  return ::getsockopt(sockfd, SOL_TCP, TCP_INFO, tcpi, &len) == 0;
}

void sockets::setTcpNoDelay(int sockfd, bool on)
{
  int optval = on ? 1 : 0;
  int ret = ::setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY,
               &optval, static_cast<socklen_t>(sizeof optval));
  if (ret < 0)
  {
    LOG_SYSERR << "TCP_NODELAY failed.";
  }
}

void sockets::setReuseAddr(int sockfd, bool on)
{
  int optval = on ? 1 : 0;
  int ret = ::setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,
               &optval, static_cast<socklen_t>(sizeof optval));
  if (ret < 0)
  {
    LOG_SYSERR << "SO_REUSEADDR failed.";
  }
}

void sockets::setReusePort(int sockfd, bool on)
{
#ifdef SO_REUSEPORT 
  int optval = on ? 1 : 0;
  int ret = ::setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT,
                         &optval, static_cast<socklen_t>(sizeof optval));
  if (ret < 0 && on)
  { 
    LOG_SYSERR << "SO_REUSEPORT failed.";
  }
#else
  if (on)
  { 
    LOG_ERROR << "SO_REUSEPORT is not supported.";
  }
#endif
}

void sockets::setKeepAlive(int sockfd, bool on)
{
  int optval = on ? 1 : 0;
  ::setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE,
               &optval, static_cast<socklen_t>(sizeof optval));
  
  /*optval = 30;
  ::setsockopt(sockfd, SOL_SOCKET, TCP_KEEPIDLE,
               &optval, static_cast<socklen_t>(sizeof optval));
  
  optval = 15;
  ::setsockopt(sockfd, SOL_SOCKET, TCP_KEEPINTVL,
               &optval, static_cast<socklen_t>(sizeof optval));
  optval = 3;
  ::setsockopt(sockfd, SOL_SOCKET, TCP_KEEPCNT,
               &optval, static_cast<socklen_t>(sizeof optval));
  */ 
  // FIXME CHECK
}

void sockets::setSndBuffer(int sockfd, int buf_len)
{
  ::setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF,
             &buf_len, static_cast<socklen_t>(sizeof buf_len));
}

void sockets::setRcvBuffer(int sockfd, int buf_len)
{
  ::setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF,
             &buf_len, static_cast<socklen_t>(sizeof buf_len));
}

struct sockaddr_un sockets::getLocalAddr(int sockfd)
{
  struct sockaddr_un localaddr;
  bzero(&localaddr, sizeof localaddr);
  socklen_t addrlen = static_cast<socklen_t>(sizeof localaddr);
  if (::getsockname(sockfd, sockaddr_cast(&localaddr), &addrlen) < 0)
  {
    LOG_SYSERR << "sockets::getLocalAddr " << sockfd;
  }
  return localaddr;
}

struct sockaddr_un sockets::getPeerAddr(int sockfd)
{
  struct sockaddr_un peeraddr;
  bzero(&peeraddr, sizeof peeraddr);
  socklen_t addrlen = static_cast<socklen_t>(sizeof peeraddr);
  if (::getpeername(sockfd, sockaddr_cast(&peeraddr), &addrlen) < 0)
  {
    LOG_SYSERR << "sockets::getPeerAddr " << sockfd;
  }
  return peeraddr;
}

bool sockets::isSelfConnect(int sockfd)
{
  struct sockaddr_un localaddr = getLocalAddr(sockfd);
  struct sockaddr_un peeraddr = getPeerAddr(sockfd);

  if (localaddr.sun_family == AF_INET)
  { 
    const struct sockaddr_in* laddr4 = reinterpret_cast<struct sockaddr_in*>(&localaddr);
    const struct sockaddr_in* raddr4 = reinterpret_cast<struct sockaddr_in*>(&peeraddr);
    return laddr4->sin_port == raddr4->sin_port
        && laddr4->sin_addr.s_addr == raddr4->sin_addr.s_addr;
  }
  else if (localaddr.sun_family == AF_INET6)
  { 
    const struct sockaddr_in6* laddr6 = reinterpret_cast<struct sockaddr_in6*>(&localaddr);
    const struct sockaddr_in6* raddr6 = reinterpret_cast<struct sockaddr_in6*>(&peeraddr);

    return laddr6->sin6_port == raddr6->sin6_port
        && memcmp(&laddr6->sin6_addr, &raddr6->sin6_addr, sizeof laddr6->sin6_addr) == 0;
  }
  else
  { 
    return false;
  }
}

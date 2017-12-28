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


#include <flagwld/net/detail/Socket.h>

#include <flagwld/base/Logging.h>
#include <flagwld/net/SocketsOps.h>
#include <flagwld/net/SockAddress.h>

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <strings.h>  // bzero
#include <stdio.h>  // snprintf

using namespace flagwld;
using namespace flagwld::net;

Socket::~Socket()
{
  sockets::close(sockfd_);
}

void Socket::bindAddress(const SockAddress& addr)
{
  sockets::bindOrDie(sockfd_, addr.getSockAddr(), addr.getSockLen());
}

void Socket::listen()
{
  sockets::listenOrDie(sockfd_);
}

int Socket::accept(SockAddress* peeraddr)
{
  struct sockaddr_un addr;
  bzero(&addr, sizeof addr);
  int connfd = sockets::accept(sockfd_, &addr);
  if (connfd >= 0)
  {
    peeraddr->setSockAddr( sockets::sockaddr_cast(&addr) );
  }
  return connfd;
}

void Socket::shutdownWrite()
{
  sockets::shutdownWrite(sockfd_);
}


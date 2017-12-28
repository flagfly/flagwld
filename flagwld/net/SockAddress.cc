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


#include <flagwld/net/SockAddress.h>

#include <flagwld/base/Logging.h>
#include <flagwld/net/Endian.h>
#include <flagwld/net/SocketsOps.h>

#include <strings.h>  // bzero
#include <string.h>  // strncpy
#include <netinet/in.h>
#include <netdb.h>

#include <sys/un.h>
#include <sys/socket.h>

#include <boost/static_assert.hpp>

// INADDR_ANY use (type)value casting.
#pragma GCC diagnostic ignored "-Wold-style-cast"
static const in_addr_t kInaddrAny = INADDR_ANY;
static const in_addr_t kInaddrLoopback = INADDR_LOOPBACK;
#pragma GCC diagnostic error "-Wold-style-cast"


//      /* Structure describing a generic socket address.  */
//      struct sockaddr
//        {
//          __SOCKADDR_COMMON (sa_);    /* Common data: address family and length.  */
//          char sa_data[14];           /* Address data.  */
//        };

//     /* Structure describing an Internet socket address.  */
//     struct sockaddr_in {
//         sa_family_t    sin_family; /* address family: AF_INET */
//         uint16_t       sin_port;   /* port in network byte order */
//         struct in_addr sin_addr;   /* internet address */
//     };

//     /* Internet address. */
//     typedef uint32_t in_addr_t;
//     struct in_addr {
//         in_addr_t       s_addr;     /* address in network byte order */
//     };

//     struct sockaddr_in6 {
//         sa_family_t     sin6_family;   /* address family: AF_INET6 */
//         uint16_t        sin6_port;     /* port in network byte order */
//         uint32_t        sin6_flowinfo; /* IPv6 flow information */
//         struct in6_addr sin6_addr;     /* IPv6 address */
//         uint32_t        sin6_scope_id; /* IPv6 scope-id */
//     };

//     /* IPv6 address */
//     struct in6_addr
//     {
//         union
//         {
//             uint8_t u6_addr8[16];
//             uint16_t u6_addr16[8];
//             uint32_t u6_addr32[4];
//         } in6_u;
//     #define s6_addr                 in6_u.u6_addr8
//     #define s6_addr16               in6_u.u6_addr16
//     #define s6_addr32               in6_u.u6_addr32
//     };

//     /* Structure describing the address of an AF_LOCAL (aka AF_UNIX) socket.  */
//     struct sockaddr_un
//       {
//         __SOCKADDR_COMMON (sun_);
//         char sun_path[108];         /* Path name.  */
//       };

using namespace flagwld;
using namespace flagwld::net;

BOOST_STATIC_ASSERT(sizeof(SockAddress) == (sizeof(struct sockaddr_un)+2));

BOOST_STATIC_ASSERT(offsetof(sockaddr_in, sin_family) == 0);
BOOST_STATIC_ASSERT(offsetof(sockaddr_in, sin_port) == 2);
BOOST_STATIC_ASSERT(offsetof(sockaddr_in6, sin6_family) == 0);
BOOST_STATIC_ASSERT(offsetof(sockaddr_in6, sin6_port) == 2);
BOOST_STATIC_ASSERT(offsetof(sockaddr_un, sun_family) == 0);
BOOST_STATIC_ASSERT(offsetof(sockaddr_un, sun_path) == 2);

SockAddress::SockAddress(uint16_t port, bool loopbackOnly, bool ipv6)
{
  if (ipv6)
  {
    bzero(&addr6_, sizeof addr6_);
    addr6_.sin6_family = AF_INET6;
    in6_addr ip = loopbackOnly ? in6addr_loopback : in6addr_any;
    addr6_.sin6_addr = ip;
    addr6_.sin6_port = sockets::hostToNetwork16(port);
  }
  else
  {
    bzero(&addr_, sizeof addr_);
    addr_.sin_family = AF_INET;
    in_addr_t ip = loopbackOnly ? kInaddrLoopback : kInaddrAny;
    addr_.sin_addr.s_addr = sockets::hostToNetwork32(ip); 
    addr_.sin_port = sockets::hostToNetwork16(port);
  }
}

SockAddress::SockAddress(const StringArg& ip, uint16_t port, bool ipv6)
{
  if (ipv6)
  {
    bzero(&addr6_, sizeof addr6_);
    sockets::fromIp6Port(ip.c_str(), port, &addr6_);
  }
  else
  {
    bzero(&addr_, sizeof addr_);
    sockets::fromIpPort(ip.c_str(), port, &addr_);
  }
}

SockAddress::SockAddress(const char* un_domain)
{
  bzero(&addr_, sizeof addr_);
  addrun_.sun_family = AF_UNIX;
  strncpy(addrun_.sun_path, un_domain, sizeof(addrun_.sun_path)-1);
}

SockAddress::SockAddress(const StringArg& un_domain)
{
  bzero(&addr_, sizeof addr_);
  addrun_.sun_family = AF_UNIX;
  strncpy(addrun_.sun_path, un_domain.c_str(), sizeof(addrun_.sun_path)-1);
}

SockAddress::SockAddress(const struct sockaddr* addr)
{
  setSockAddr(addr);
}

void SockAddress::setSockAddr(const struct sockaddr* addr)
{
  switch (addr->sa_family)
  {
    case AF_INET:
    {
      bzero(&addr_, sizeof addr_);
      addr_ = *(sockets::sockaddr_in_cast(addr));
      break;
    }
    case AF_INET6:
    {
      bzero(&addr6_, sizeof addr6_);
      addr6_ = *(sockets::sockaddr_in6_cast(addr));

      break;
    }
    case AF_UNIX:
    {
      bzero(&addrun_, sizeof addrun_);
      addrun_ = *(sockets::sockaddr_un_cast(addr));

      break;
    }
    default:
      assert (0);
  };
}

socklen_t SockAddress::getSockLen() const
{
  socklen_t len = -1;

  switch (family())
  { 
    case AF_INET:
    { 
      len = static_cast<socklen_t>(sizeof addr_);
      break;
    }
    case AF_INET6:
    { 
      len = static_cast<socklen_t>(sizeof addr6_);
      break;
    }
    case AF_UNIX:
    { 
      len = static_cast<socklen_t>(sizeof addrun_);
      break;
    }
    default: 
      assert (0);
  };

  return len;
}

string SockAddress::toString() const
{
  char buf[64] = "";
  const char *buf2 = buf;

  switch (family())
  {
    case AF_INET:
    {
      sockets::toIpPort(buf, sizeof buf, addr_);
      break;
    }
    case AF_INET6:
    {
      sockets::toIpPort(buf, sizeof buf, addr6_);
      break;
    }
    case AF_UNIX:
    {
      buf2 = addrun_.sun_path;
      break;
    }
    default:
      assert (0);
  };

  return buf2;
}

string SockAddress::toShortString() const
{
  char buf[64] = "";
  const char *buf2 = buf;

  switch (family())
  {
    case AF_INET:
    {
      sockets::toIp(buf, sizeof buf, addr_);
      break;
    }
    case AF_INET6:
    {
      sockets::toIp(buf, sizeof buf, addr6_);
      break;
    }
    case AF_UNIX:
    {
      buf2 = addrun_.sun_path;
      break;
    }
    default:
      assert (0);
  };

  return buf2;
}

uint16_t SockAddress::toPort() const
{
  return sockets::networkToHost16(portNetEndian());
}

uint16_t SockAddress::portNetEndian() const
{
  assert(family() != AF_UNIX);
  return addr_.sin_port;
}

uint32_t SockAddress::ipNetEndian() const
{
  assert(family() == AF_INET);
  return addr_.sin_addr.s_addr;
}

static __thread char t_resolveBuffer[64 * 1024];

bool SockAddress::resolve(StringArg hostname, SockAddress* out)
{
  assert(out != NULL);
  struct hostent hent;
  struct hostent* he = NULL;
  int herrno = 0;
  bzero(&hent, sizeof(hent));

  int ret = gethostbyname_r(hostname.c_str(), &hent, t_resolveBuffer, sizeof t_resolveBuffer, &he, &herrno);
  if (ret == 0 && he != NULL)
  {
    if ( he->h_addrtype == AF_INET )
    {
      assert(he->h_length == (sizeof(uint32_t)));
      out->addr_.sin_addr = *reinterpret_cast<struct in_addr*>(he->h_addr);

      return true;
    }
    else if ( he->h_addrtype == AF_INET6 )
    {
      assert(he->h_length == (4*sizeof(uint32_t)));
      out->addr6_.sin6_addr = *reinterpret_cast<struct in6_addr*>(he->h_addr);

      return true;
    }
    else
    {
      assert (0);
    }
  }
  else
  {
    if (ret)
    {
      LOG_SYSERR << "SockAddress::resolve";
    }
  }

  return false;
}

